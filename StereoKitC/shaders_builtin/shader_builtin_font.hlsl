#include "stereokit.hlsli"

//--name = sk/font

float4 color = {1,1,1,1};

//--diffuse = white
Texture2D    diffuse   : register(t0);
SamplerState diffuse_s : register(s0);

struct vsIn {
	float4 pos   : SV_Position;
	float3 norm  : NORMAL0;
	float2 uv    : TEXCOORD0;
	float4 color : COLOR0;
};
struct psIn {
	float4      pos     : SV_Position;
	float2      uv      : TEXCOORD0;
	min16float4 color   : COLOR0;
};

psIn vs(vsIn input, sk_ids_t ids) {

	psIn o;
	float3 world = mul(float4(input.pos.xyz, 1), sk_inst[ids.inst].world).xyz;
	o.pos        = mul(float4(world,         1), sk_viewproj[ids.view]);

	o.uv    = input.uv;
	o.color = input.color * color;
	return o;
}

min16float4 ps(psIn input) : SV_TARGET {
	// From an excellent article about text rendering by Ben Golus:
	// https://medium.com/@bgolus/sharper-mipmapping-using-shader-based-supersampling-ed7aadb47bec
	float2 dx = ddx(input.uv);
	float2 dy = ddy(input.uv); // manually calculate the per axis mip level, clamp to 0 to 1

	// supersampled using 2x2 rotated grid
	// The four taps are +-two vectors, which the compiler doesn't spot on its own.
	const float2 uvOffsets = float2(0.125, 0.375);
	float2 ofs_a = uvOffsets.x * dx + uvOffsets.y * dy;
	float2 ofs_b = uvOffsets.y * dx - uvOffsets.x * dy;
	min16float a = diffuse.SampleBias(diffuse_s, input.uv.xy + ofs_a, -1).r;
	min16float b = diffuse.SampleBias(diffuse_s, input.uv.xy - ofs_a, -1).r;
	min16float c = diffuse.SampleBias(diffuse_s, input.uv.xy + ofs_b, -1).r;
	min16float d = diffuse.SampleBias(diffuse_s, input.uv.xy - ofs_b, -1).r;

	min16float text_value = (a + b + c + d) * 0.25 * input.color.a;
	clip(text_value-0.004); // .004 is 1/255, or one 8bit pixel value!

	return min16float4(input.color.rgb, text_value);
}