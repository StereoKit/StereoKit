#include "stereokit.hlsli"

//--color:color = 1,1,1,1
//--tex_trans   = 0,0,1,1
//--diffuse     = white

float4       color;
float4       tex_trans;
Texture2D    diffuse   : register(t0);
SamplerState diffuse_s : register(s0);

cbuffer shadow_buffer : register(b13) {
	float4x4 shadowmap_transform;
	float3   light_direction;
	float    shadowmap_bias;
	float3   light_color;
	float    shadowmap_pixel_size;
};
Texture2D              shadow_map   : register(t13);
SamplerComparisonState shadow_map_s : register(s13);

///////////////////////////////////////////

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;
	float4 col  : COLOR0;
};
struct psIn {
	float4 pos          : SV_Position;
	float2 uv           : TEXCOORD0;
	float3 shadow_uv    : TEXCOORD1;
	float3 world        : TEXCOORD2;
	float  shadow_ndotl : TEXCOORD3;
	float4 color        : COLOR0;
	float3 ambient      : COLOR1;
	uint   view_id      : SV_RenderTargetArrayIndex;
};

///////////////////////////////////////////

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn o;
	o.view_id = id % sk_view_count;
	id        = id / sk_view_count;

	float4 world = mul(input.pos, sk_inst[id].world);
	o.pos        = mul(world,     sk_viewproj[o.view_id]);
	
	float3 normal = normalize(mul(input.norm, (float3x3) sk_inst[id].world));
	o.shadow_ndotl = dot(normal, light_direction);

	// Apply bias to the shadow map position directly in world space:
	// https://c0de517e.blogspot.com/2011/05/shadowmap-bias-notes.html
	float  slope      = saturate(min(1, sqrt(1 - o.shadow_ndotl * o.shadow_ndotl) / o.shadow_ndotl));
	float3 bias       = normal * (shadowmap_bias * slope);
	float4 shadow_pos = mul(float4(world.xyz + bias, 1), shadowmap_transform);
	o.shadow_uv = float3(shadow_pos.xy, shadow_pos.z / shadow_pos.w);
	// In GL, shadow_pos.z/shadow_pos.w seems to be in the range [-1, 1], so we
	// need to adjust it to [0, 1] to match D3D. There's also a flip in the Y
	// direction, but that's more expected.
#ifdef SK_OPENGL
	o.shadow_uv    = o.shadow_uv    * 0.5                 + 0.5;
#else
	o.shadow_uv.xy = o.shadow_uv.xy * float2(0.5f, -0.5f) + 0.5;
#endif
	
	o.world      = world.xyz;
	o.uv         = (input.uv * tex_trans.zw) + tex_trans.xy;
	o.color      = color * input.col * sk_inst[id].color;
	// We darken the ambient light by 0.5, since in the sample this shader is
	// mixed with other materials that don't have the extra directional light.
	// For real applications, this would be 1.0, but all Materials would use
	// the directional light in addition to the ambient light.
	o.ambient    = sk_lighting(normal) * 0.5;
	return o;
}

///////////////////////////////////////////

// Alternative faster shadow
float shadow_factor_fast(float3 uv) {
	return shadow_map.SampleCmpLevelZero(shadow_map_s, uv.xy, uv.z);
}

///////////////////////////////////////////

// Basic 3x3 PCF filter, nice smooth edges, if a bit blurred
float shadow_factor_pcf3(float3 uv, float scale) {
	float radius        = shadowmap_pixel_size * scale;
	float shadow_factor = 0.0;
	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			float2 offset = float2(x, y) * radius;
			shadow_factor += shadow_map.SampleCmpLevelZero(shadow_map_s, uv.xy + offset, uv.z);
		}
	}
	return shadow_factor / 9.0;
}

///////////////////////////////////////////

float4 ps(psIn input) : SV_TARGET {
	float4 col = diffuse.Sample(diffuse_s, input.uv);
	
	float light = input.shadow_ndotl > 0
		? min(input.shadow_ndotl, shadow_factor_pcf3(input.shadow_uv, 1))
		: 0;
	
	col.rgb *= input.ambient + light * light_color;
	return col;
}