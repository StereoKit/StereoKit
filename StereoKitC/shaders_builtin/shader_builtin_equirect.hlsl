// [name] sk/blit/equirect_convert
cbuffer GlobalBuffer : register(b0) {
	float4x4 sk_view[2];
	float4x4 sk_proj[2];
	float4x4 sk_viewproj[2];
	float3   sk_lighting_sh[9];
	float4   sk_camera_pos[2];
	float4   sk_camera_dir[2];
	float4   sk_fingertip[2];
	float    sk_time;
};
cbuffer TransformBuffer : register(b1) {
	float sk_width;
	float sk_height;
	float sk_pixel_width;
	float sk_pixel_height;
};

cbuffer ParamBuffer : register(b2) {
	// [param] vector up {0,1,0,0}
	float4 up;
	// [param] vector right {1,0,0,0}
	float4 right;
	// [param] vector forward {0,0,-1,0}
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
SamplerState source_sampler : register(s0);

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