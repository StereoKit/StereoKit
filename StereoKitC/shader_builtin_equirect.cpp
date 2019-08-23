#include "shader_builtin.h"

const char* sk_shader_builtin_equirect = R"_(
cbuffer GlobalBuffer : register(b0) {
	float4x4 sk_view;
	float4x4 sk_proj;
	float4x4 sk_viewproj;
	float4   sk_light;
	float4   sk_light_color;
	float4   sk_camera_pos;
	float4   sk_camera_dir;
};
cbuffer TransformBuffer : register(b1) {
	float sk_width;
	float sk_height;
	float sk_pixel_width;
	float sk_pixel_height;
};

cbuffer ParamBuffer : register(b2) {
	// [param] vector up default{0,1,0,0}
	float4 up;
	// [param] vector right default{1,0,0,0}
	float4 right;
	// [param] vector forward default{0,0,-1,0}
	float4 forward;
};
struct vsIn {
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};
struct psIn {
	float4 pos : SV_POSITION;
	float3 norm : TEXCOORD0;
};

// [texture] source white
Texture2D source : register(t0);
SamplerState source_sampler;

static const float2 invAtan = float2(0.1591, 0.3183);
float2 ToEquirect(float3 v) {
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

psIn vs(vsIn input) {
	psIn output;
	output.pos = input.pos;

	float2 uv = input.uv * 2 - 1;
	output.norm  = forward.xyz + right.xyz*uv.x + up.xyz*uv.y;
	return output;
}

float4 ps(psIn input) : SV_TARGET{
	float4 color = source.Sample(source_sampler, ToEquirect(normalize(input.norm)));
	return color;
}
)_";