#include "stereokit.hlsli"

//--color:color = 1,1,1,1
//--tex_trans   = 0,0,1,1
//--diffuse     = white

float4       color;
float4       tex_trans;
Texture2D    diffuse   : register(t0);
SamplerState diffuse_s : register(s0);

cbuffer shadow_buffer : register(b12) {
	float4x4 shadowmap_transform;
	float shadowmap_size;
	float shadowmap_bias;
	float depth_distance;
	float dummy1;
	float3 light_direction;
	float dummy2;
};
Texture2D    shadow_map   : register(t12);
//SamplerState shadow_map_s : register(s12);
SamplerComparisonState shadow_map_s : register(s12);

///////////////////////////////////////////

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;
	float4 col  : COLOR0;
};
struct psIn : sk_ps_input_t {
	float4 pos   : SV_Position;
	float2 uv    : TEXCOORD0;
	float3 shadow_uv : TEXCOORD1;
	float3 world : TEXCOORD2;
	float3 normal : TEXCOORD3;
	float4 color : COLOR0;
	float3 light : COLOR1;
};

///////////////////////////////////////////

psIn vs(vsIn input, sk_vs_input_t sk_in) {
	psIn o;
	uint view_id = sk_view_init(sk_in, o);
	uint id      = sk_inst_id  (sk_in);

	float4 world = mul(input.pos, sk_inst[id].world);
	o.pos        = mul(world,     sk_viewproj[view_id]);

	float4 shadow_pos = mul(float4(world.xyz, 1), shadowmap_transform);
	o.shadow_uv = float3(shadow_pos.xy, shadow_pos.z / shadow_pos.w);
	// In GL, shadow_pos.z/shadow_pos.w seems to be in the range [-1, 1], so we
	// need to adjust it to [0, 1] to match D3D. There's also a flip in the Y
	// direction, but that's more expected.
#ifdef SK_OPENGL
	o.shadow_uv    = o.shadow_uv    * 0.5                 + 0.5;
#else
	o.shadow_uv.xy = o.shadow_uv.xy * float2(0.5f, -0.5f) + 0.5;
#endif

	o.normal = normalize(mul(input.norm, (float3x3)sk_inst[id].world));

	o.world      = world.xyz;
	o.uv         = (input.uv * tex_trans.zw) + tex_trans.xy;
	o.color      = color * input.col * sk_inst[id].color;
	o.light      = sk_lighting(o.normal);
	return o;
}

///////////////////////////////////////////

// Fast
float shadow_factor_fast(float3 uv) {
	//float shadow_depth = shadow_map.Sample(shadow_map_s, uv.xy).r;
	//return (shadow_depth + shadowmap_bias < uv.z) ? 0.1 : 1.0;
	
	//float shadow_depth = shadow_map.SampleCmpLevelZero(shadow_map_s, uv.xy, uv.z - shadowmap_bias);

	return shadow_map.SampleCmpLevelZero(shadow_map_s, uv.xy, uv.z - shadowmap_bias);
}

///////////////////////////////////////////

// Basic 3x3 PCF filter
/*float shadow_factor_pcf3(float3 uv) {
	float shadow_factor = 0.0;

	// Sample 3x3 grid around the shadow map coordinate
	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			float2 offset        = float2(x, y) * shadowmap_size;
			float  sampled_depth = shadow_map.Sample(shadow_map_s, uv.xy + offset).r;
			shadow_factor += (sampled_depth + shadowmap_bias < uv.z) ? 0.1 : 1.0;
		}
	}
	return shadow_factor / 9.0;
}

///////////////////////////////////////////

// https://www.shadertoy.com/view/4djSRW
float hash12(float2 p) {
	float3 p3 = frac(p.xyx * .1031);
	p3 += dot(p3, p3.yzx + 33.33);
	return frac((p3.x + p3.y) * p3.z);
}

float2 sample_offsets[16] = {
	float2(-0.5, -0.5),
	float2( 0.5, -0.5),
	float2(-0.5,  0.5),
	float2( 0.5,  0.5),

	float2( 0.0, -0.5),
	float2(-0.5,  0.0),
	float2( 0.5,  0.0),
	float2( 0.0,  0.5),

	float2(-0.25, -0.25),
	float2( 0.25, -0.25),
	float2(-0.25,  0.25),
	float2( 0.25,  0.25),

	float2(-0.35,  0.15),
	float2( 0.35, -0.15),
	float2(-0.15, -0.35),
	float2( 0.15,  0.35)
};

///////////////////////////////////////////

float shadow_factor_rotated(float2 screen, float3 uv) {
	// This doesn't seem to be producing great results, it can be improved.
	float2 random_offset = hash12(screen);
	random_offset = (random_offset - 0.5) * 2.0 * shadowmap_size;

	const int samples = 16;
	float shadow_factor = 0;
	for (int i = 0; i < samples; i++) {
		float2 sample_pos    = uv.xy + sample_offsets[i] * shadowmap_size + random_offset;
		float  sampled_depth = shadow_map.Sample(shadow_map_s, sample_pos).r;
		shadow_factor += (sampled_depth + shadowmap_bias < uv.z) ? 0.1 : 1.0;
	}

	return shadow_factor / (float)samples;
}

float random(float3 seed, int i) {
	float4 seed4 = float4(seed, i);
	float  dot_product = dot(seed4, float4(12.9898, 78.233, 45.164, 94.673));
	return frac(sin(dot_product) * 43758.5453);
}
float3 quantize(float3 value, float levels) {
	return round(value * levels) / levels;
}
float shadow_factor_rotated_world(float3 world, float3 uv) {
	world = quantize(world, 100);
	const int samples = 256;
	const float shadow_penumbra = 16.0;
	
	// Check the first 4 points
	int   occluded = 0;
	float initial[4];
	int   i = 0;
	for (i = 0; i < 4; i++) {
		float2 sample_pos    = uv.xy + (float2(random(world, i * 2), random(world, i * 2 + 1)) - 0.5) * shadow_penumbra * shadowmap_size;
		float  sampled_depth = shadow_map.Sample(shadow_map_s, sample_pos).r;
		if (sampled_depth + shadowmap_bias < uv.z)
			occluded += 1;
		initial[i] = sampled_depth;
	}
	
	//if (occluded == 4) return 0;
	//if (occluded == 0) return 1;

	float2 max_depths = max(float2(initial[0], initial[1]), float2(initial[2], initial[3]));
	float  max_depth  = max(max_depths.x, max_depths.y);
	float2 min_depths = min(float2(initial[0], initial[1]), float2(initial[2], initial[3]));
	float  min_depth  = min(min_depths.x, min_depths.y);
	
	float scale = lerp(1.0, shadow_penumbra, saturate(((max_depth - min_depth) * depth_distance) / 10));
	//scale = 4.0;
	//return scale / shadow_penumbra;
	//return scale / 32.0;

	// Now do the rest of the samples
	for (i = 4; i < samples; i++)
	{
		float2 sample_pos    = uv.xy + (float2(random(world, i * 2), random(world, i * 2 + 1)) - 0.5) * scale * shadowmap_size;
		float  sampled_depth = shadow_map.Sample(shadow_map_s, sample_pos).r;
		if (sampled_depth + shadowmap_bias < uv.z)
			occluded += 1;
	}

	return lerp(1, 0.1, (float) occluded / (float) samples);
}

float shadow_factor_rotated_early_out(float3 world, float3 uv) {
	world = quantize(world, 1000);
	const int samples = 8;
	const int initial_samples = 8;
	
	int   i = 0;
	int   occluded = 0;
	//float initial[initial_samples];
	//for (i = 0; i < initial_samples; i++) {
	//	float2 sample_pos    = uv.xy + (float2(random(world, i * 2), random(world, i * 2 + 1)) - 0.5) * 2 * shadowmap_size;
	//	float  sampled_depth = shadow_map.Sample(shadow_map_s, sample_pos).r;
	//	if (sampled_depth + shadowmap_bias < uv.z)
	//		occluded += 1;
	//	initial[i] = sampled_depth;
	//}
	//
	//if (occluded == initial_samples) return 0.1;
	//if (occluded == 0) return 1;
	
	for (i = 0; i < samples; i++) {
		float2 sample_pos    = uv.xy + (float2(random(world, i * 2), random(world, i * 2 + 1)) - 0.5) * 2 * shadowmap_size;
		float  sampled_depth = shadow_map.Sample(shadow_map_s, sample_pos).r;
		if (sampled_depth + shadowmap_bias < uv.z)
			occluded += 1;
	}

	return lerp(1, 0.1, (float) occluded / (float) samples);
}

///////////////////////////////////////////

float shadow_factor_pcss(float2 screen, float3 uv) {
	float avg_blocker_depth = 0.0;
	int   num_blockers = 0;

	// Use a small kernel to search for blockers
	[unroll]
	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			float2 sample_offset = float2(x, y) * shadowmap_size;
			float  sampled_depth = shadow_map.Sample(shadow_map_s, uv.xy + sample_offset).r;

			if (sampled_depth + shadowmap_bias < uv.z) { // If there is a blocker
				avg_blocker_depth += sampled_depth;
				num_blockers+=1;
			}
		}
	}

	// If no blockers were found, the pixel is fully lit
	if (num_blockers == 0) return 0.999;

	// Calculate average blocker depth
	avg_blocker_depth /= (float)num_blockers;

	// Estimate Penumbra Size
	float light_radius  = 3.0; // Adjust this to match light source size
	float penumbra_size = ((uv.z - avg_blocker_depth) / avg_blocker_depth) * light_radius;
	float shadow_factor = 0.0;
	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			float2 offset        = float2(x, y) * shadowmap_size * penumbra_size;
			float  sampled_depth = shadow_map.Sample(shadow_map_s, uv.xy + offset).r;
			shadow_factor += (sampled_depth + shadowmap_bias < uv.z) ? 0.1 : 1.0;
		}
	}
	return shadow_factor / 9.0;
}

///////////////////////////////////////////

float shadow_factor_pcf3_opt(float3 uv) {
	
	static const float W3[3][3] = {
		{ 0.5, 1.0, 0.5, },
		{ 1.0, 1.0, 1.0, },
		{ 0.5, 1.0, 0.5, }
	};
	
	float shadow_factor = 0.0;
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) {
			float2 offset  = float2(x-1, y-1) * shadowmap_size;
			shadow_factor += W3[x][y] * shadow_map.SampleCmpLevelZero(shadow_map_s, uv.xy + offset, uv.z - shadowmap_bias).r;
		}
	}
	return shadow_factor / 7.0;
}*/

///////////////////////////////////////////

float4 ps(psIn input) : SV_TARGET {
	float4 col = diffuse.Sample(diffuse_s, input.uv);
	
	float shadow_factor = dot(input.normal, light_direction);
	//if (shadow_factor < 0.1) {
	//	shadow_factor = 0.1;
	//} else {

		shadow_factor = shadow_factor_fast(input.shadow_uv);
		//shadow_factor = min(shadow_factor, shadow_factor_fast(input.shadow_uv));
		//float shadow_factor = shadow_factor_pcf3(input.shadow_uv);
		//shadow_factor = min(shadow_factor, shadow_factor_pcf3_opt(input.shadow_uv));
		//float shadow_factor = shadow_factor_rotated(input.pos.xy, input.shadow_uv);
		//float shadow_factor = shadow_factor_rotated_world(input.world, input.shadow_uv);
		//shadow_factor = min(shadow_factor, shadow_factor_rotated_early_out(input.world, input.shadow_uv));
		//float shadow_factor = shadow_factor_pcss(input.pos.xy, input.shadow_uv);
	//}

	input.light     *= shadow_factor;
	input.color.rgb *= input.light;
	return col * input.color;
}