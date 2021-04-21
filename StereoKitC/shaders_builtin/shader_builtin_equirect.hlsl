//--name = sk/blit/equirect_convert
//--up      = 0,1,0,0
//--right   = 1,0,0,0
//--forward = 0,0,-1,0
//--source  = white

float4       up;
float4       right;
float4       forward;
Texture2D    source   : register(t0);
SamplerState source_s : register(s0);

cbuffer GlobalBuffer : register(b1) {
	float4x4 sk_view[2];
	float4x4 sk_proj[2];
	float4x4 sk_viewproj[2];
	float3   sk_lighting_sh[9];
	float4   sk_camera_pos[2];
	float4   sk_camera_dir[2];
	float4   sk_fingertip[2];
	float    sk_time;
};
cbuffer TransformBuffer : register(b2) {
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
};

static const float2 invAtan = float2(0.1591, 0.3183);
float2 ToEquirect(float3 v) {
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

psIn vs(vsIn input) {
	psIn o;
	o.pos = input.pos;

	float2 uv = input.uv * 2 - 1;
	o.norm    = forward.xyz + right.xyz*uv.x + up.xyz*uv.y;
	return o;
}

float4 ps(psIn input) : SV_TARGET {
	float4 color = source.Sample(source_s, ToEquirect(normalize(input.norm)));
	return color;
}