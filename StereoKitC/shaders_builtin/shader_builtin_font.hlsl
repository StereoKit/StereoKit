#include "stereokit.hlsli"

//--name = sk/font
//--color:color = 1,1,1,1
//--diffuse     = white
float4       color;
Texture2D    diffuse   : register(t0);
SamplerState diffuse_s : register(s0);

struct vsIn {
	float4 pos   : SV_Position;
	float3 norm  : NORMAL0;
	float2 uv    : TEXCOORD0;
	float4 color : COLOR0;
};
struct psIn : sk_ps_input_t {
	float4 pos     : SV_Position;
	float2 uv      : TEXCOORD0;
	float4 color   : COLOR0;
};

psIn vs(vsIn input, sk_vs_input_t sk_in) {
	psIn o;
	uint view_id = sk_view_init(sk_in, o);
	uint id      = sk_inst_id  (sk_in);

	float3 world = mul(float4(input.pos.xyz, 1), sk_inst[id].world).xyz;
	o.pos        = mul(float4(world,         1), sk_viewproj[view_id]);

	o.uv    = input.uv;
	o.color = input.color * color;
	return o;
}

float4 ps(psIn input) : SV_TARGET {
	// From an excellent article about text rendering by Ben Golus:
	// https://medium.com/@bgolus/sharper-mipmapping-using-shader-based-supersampling-ed7aadb47bec
	float2 dx = ddx(input.uv);
	float2 dy = ddy(input.uv); // manually calculate the per axis mip level, clamp to 0 to 1

	float2 uvOffsets = float2(0.125, 0.375);
	float4 offsetUV  = float4(0.0, 0.0, 0.0, -1); // supersampled using 2x2 rotated grid
	half4  col = 0;

	offsetUV.xy = input.uv.xy + uvOffsets.x * dx + uvOffsets.y * dy;
	col += diffuse.SampleBias(diffuse_s, offsetUV.xy, offsetUV.w);
	offsetUV.xy = input.uv.xy - uvOffsets.x * dx - uvOffsets.y * dy;
	col += diffuse.SampleBias(diffuse_s, offsetUV.xy, offsetUV.w);
	offsetUV.xy = input.uv.xy + uvOffsets.y * dx - uvOffsets.x * dy;
	col += diffuse.SampleBias(diffuse_s, offsetUV.xy, offsetUV.w);
	offsetUV.xy = input.uv.xy - uvOffsets.y * dx + uvOffsets.x * dy;
	col += diffuse.SampleBias(diffuse_s, offsetUV.xy, offsetUV.w);
	col *= 0.25;
	float text_value = col.r * input.color.a;
	clip(text_value-0.004); // .004 is 1/255, or one 8bit pixel value!

	return float4(input.color.rgb, text_value);
}