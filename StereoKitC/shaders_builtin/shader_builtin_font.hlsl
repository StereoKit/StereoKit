// [name] sk/font

#include <stereokit>

cbuffer ParamBuffer : register(b2) {
	// [param] color color {1,1,1,1}
	float4 color;
};
struct vsIn {
	float4 pos   : SV_POSITION;
	float2 uv    : TEXCOORD0;
	float4 color : COLOR0;
};
struct psIn {
	float4 pos     : SV_POSITION;
	float4 color   : COLOR0;
	float2 uv      : TEXCOORD0;
	uint   view_id : SV_RenderTargetArrayIndex;
};

// [texture] diffuse white
Texture2D    tex         : register(t0);
SamplerState tex_sampler : register(s0);

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn output;
	float3 world = mul(float4(input.pos.xyz, 1), sk_inst[id].world).xyz;
	output.pos   = mul(float4(world,         1), sk_viewproj[sk_inst[id].view_id]);

	output.view_id = sk_inst[id].view_id;
	output.uv      = input.uv;
	output.color   = input.color * color;
	return output;
}

float4 ps(psIn input, bool frontface : SV_IsFrontFace) : SV_TARGET{
	// From an excellent article about text rendering by Ben Golus:
	// https://medium.com/@bgolus/sharper-mipmapping-using-shader-based-supersampling-ed7aadb47bec
	float2 dx = ddx(input.uv);
	float2 dy = ddy(input.uv); // manually calculate the per axis mip level, clamp to 0 to 1

	float2 uvOffsets = float2(0.125, 0.375);
	float4 offsetUV = float4(0.0, 0.0, 0.0, -1); // supersampled using 2x2 rotated grid
	half4  col = 0;

	offsetUV.xy = input.uv.xy + uvOffsets.x * dx + uvOffsets.y * dy;
	col += tex.SampleBias(tex_sampler, offsetUV.xy, offsetUV.w);
	offsetUV.xy = input.uv.xy - uvOffsets.x * dx - uvOffsets.y * dy;
	col += tex.SampleBias(tex_sampler, offsetUV.xy, offsetUV.w);
	offsetUV.xy = input.uv.xy + uvOffsets.y * dx - uvOffsets.x * dy;
	col += tex.SampleBias(tex_sampler, offsetUV.xy, offsetUV.w);
	offsetUV.xy = input.uv.xy - uvOffsets.y * dx + uvOffsets.x * dy;
	col += tex.SampleBias(tex_sampler, offsetUV.xy, offsetUV.w);
	col *= 0.25;
	float text_value = col.r;
	clip(text_value-0.004); // .004 is 1/255, or one 8bit pixel value!

	return float4(input.color.rgb, text_value);
}