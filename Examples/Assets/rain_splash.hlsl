#include <stereokit.hlsli>

///////////////////////////////////////////
// Splashes for the looping rain: flat quads on the ground, scattered in
// the same head-following wrap cube as the droplets. Each one loops its
// own little life - pop in, expand, fade - on a private phase, so the
// ground crawls with sparse impacts that track the rain's intensity.

//--rain_head_pos = 0, 0, 0, 0
//--rain_params   = 0, 20, 9, 0.15
//--rain_color    = 0.8, 0.8, 0.8, 0.3
//--rain_settings = 1, 1, -1.5, 0
float4 rain_head_pos; // xyz = head world position
float4 rain_params;   // x = time, y = cube_size
float4 rain_color;    // xyz = water albedo tint, w = opacity - lit by the skylight SH
float4 rain_settings; // x = intensity (0-1), y = drop_size multiplier, z = floor world y

//--diffuse = white
Texture2D    diffuse;
SamplerState diffuse_s;

///////////////////////////////////////////

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;  // x = vertex index (0-3), y = per-splash variation
	float4 col  : COLOR0;     // r = phase
};
struct psIn {
	float4 pos   : SV_POSITION;
	float2 uv    : TEXCOORD0;
	float4 color : COLOR0; // Lit tint, a = fades
};

///////////////////////////////////////////

float wrap(float x, float size) {
	return x - floor(x / size) * size;
}

psIn vs(vsIn input, sk_ids_t ids) {
	psIn o;

	float intensity = rain_settings.x;
	if (input.uv.y > intensity) {
		o.pos = 0.0 / 0.0;
		return o;
	}

	float  time      = rain_params.x;
	float  cube_size = rain_params.y;
	float3 head      = rain_head_pos.xyz;

	// A private little life, looping: expand fast, fade out.
	float cycle   = 0.35 + 0.3 * input.uv.y;
	float phase   = time / cycle + input.col.r * 7.13;
	float t       = frac (phase);
	float cycle_i = floor(phase);
	float size    = lerp(0.005, 0.05, t) * rain_settings.y;
	float fade    = (1 - t) * (1 - t);

	// A fresh random spot every rebirth: hash the cycle index against the
	// splash's identity so the loop never lands in the same place twice -
	// pulsing fixed positions read as a repeating pattern, where the
	// falling drops hide their loop by always moving.
	float2 spot = float2(
		frac(sin(cycle_i * 12.9898 + input.uv.y * 78.233) * 43758.547),
		frac(sin(cycle_i * 39.3468 + input.uv.y * 11.135) * 24634.635));

	// Same wrap as the droplets, XZ only - splashes live on the floor.
	float3 head_offset = head / cube_size;
	float2 local;
	local.x = wrap(spot.x - head_offset.x, 1.0) - 0.5;
	local.y = wrap(spot.y - head_offset.z, 1.0) - 0.5;

	// Flat quad corners from the vertex index, on the ground plane.
	int    vtx    = (int)input.uv.x;
	float2 corner = float2((vtx == 1 || vtx == 3) ? 1 : -1,
	                       (vtx == 2 || vtx == 3) ? 1 : -1);
	o.uv = corner * 0.5 + 0.5;

	float3 model_pos;
	model_pos.xz = local * cube_size + corner * size;
	model_pos.y  = (rain_settings.z + 0.005) - head.y; // World floor, head-relative model space

	float4 world = mul(float4(model_pos, 1), sk_inst[ids.inst].world);
	o.pos        = mul(world, sk_viewproj[ids.view]);

	// Fade at the cube edge like the droplets do. Splashes lie flat on
	// the ground, so they're lit by the sky straight above them.
	float edge  = length(local) * 2;
	float alpha = fade * saturate(1.0 - smoothstep(0.8, 1.0, edge));
	o.color = float4(rain_color.rgb * sk_lighting(float3(0, 1, 0)), alpha * rain_color.a);

	return o;
}

///////////////////////////////////////////

float4 ps(psIn input) : SV_TARGET {
	// GenParticle's shape lives in the alpha channel - rgb is solid white.
	float circle = diffuse.Sample(diffuse_s, input.uv).a;
	return float4(input.color.rgb, circle * input.color.a);
}
