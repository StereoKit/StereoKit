//--name = sk/blit/linear
//--source = white

#include "stereokit.hlsli"

Texture2D    source   : register(t0);
SamplerState source_s : register(s0);
float4       source_i;

cbuffer TransformBuffer : register(b2) {
	float sk_width;
	float sk_height;
	float sk_pixel_width;
	float sk_pixel_height;
};

struct vsIn {
	float4 pos   : SV_Position;
	float3 norm  : NORMAL0;
	float2 uv    : TEXCOORD0;
	float4 color : COLOR0;
};
struct psIn {
	float4 pos : SV_POSITION;
	float2 uv   : TEXCOORD0;
};

psIn vs(vsIn input) {
	psIn o;
	o.pos = input.pos;
	o.uv  = input.uv;
	return o;
}

float4 ps(psIn input) : SV_TARGET{
	return source.Sample(source_s, input.uv);
}