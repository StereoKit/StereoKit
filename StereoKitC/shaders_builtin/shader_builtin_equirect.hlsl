#include "stereokit.hlsli"

//--up      = 0,1,0,0
//--right   = 1,0,0,0
//--forward = 0,0,-1,0
//--source  = white

float4       up;
float4       right;
float4       forward;
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
	float3 norm : TEXCOORD0;
	uint view_id : SV_RenderTargetArrayIndex;
};

static const float2 invAtan = float2(0.1591, 0.3183);
float2 ToEquirect(float3 v) {
	float2 uv = float2(atan2(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	return uv;
}

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn o;
	o.view_id = id % sk_view_count;
	o.pos     = input.pos;

	float2 uv = input.uv * 2 - 1;
	o.norm    = forward.xyz + right.xyz*uv.x + up.xyz*uv.y;
	return o;
}

float4 ps(psIn input) : SV_TARGET {
	float4 color = source.Sample(source_s, ToEquirect(normalize(input.norm)));
	return color;
}