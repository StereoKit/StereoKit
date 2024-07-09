#include "stereokit.hlsli"

//--source = white

//--blit_to = = 0,0,1,1

float4 blit_to;

Texture2D    source   : register(t0);
SamplerState source_s : register(s0);

cbuffer TransformBuffer : register(b3) {
	float sk_width;
	float sk_height;
	float sk_pixel_width;
	float sk_pixel_height;
};

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;
	float4 col  : COLOR0;
};
struct psIn {
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
	uint view_id : SV_RenderTargetArrayIndex;
};

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn o;
	o.view_id = id % sk_view_count;
	o.pos.zw = input.pos.zw;
	o.pos.xy = float2(0.5, 0.5) + input.pos.xy * 0.5;
	o.pos.xy = blit_to.xy + (o.pos.xy * blit_to.zw);
	o.pos.xy = (o.pos.xy - float2(0.5, 0.5)) * 2;
	o.pos.y  = -o.pos.y;
	o.uv     = float2(input.uv.x, 1-input.uv.y);
	return o;
}

float4 ps(psIn input) : SV_TARGET{
	return source.Sample(source_s, input.uv);
}