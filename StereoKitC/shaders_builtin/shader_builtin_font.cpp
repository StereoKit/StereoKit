#include "shader_builtin.h"

const char* sk_shader_builtin_font = R"_(
// [name] sk/font
cbuffer GlobalBuffer : register(b0) {
	float4x4 sk_view;
	float4x4 sk_proj;
	float4x4 sk_viewproj;
	float4   sk_light;
	float4   sk_light_color;
	float4   sk_camera_pos;
	float4   sk_camera_dir;
};
struct Inst {
	float4x4 world;
	float4   color;
};
cbuffer TransformBuffer : register(b1) {
	Inst sk_inst[1];
};
TextureCube sk_cubemap : register(t11);
SamplerState tex_cube_sampler;

cbuffer ParamBuffer : register(b2) {
	// [param] color color {1,1,1,1}
	float4 color;
};
struct vsIn {
	float4 pos : SV_POSITION;
	float3 norm : NORMAL;
	float2 uv : TEXCOORD0;
	float4 color : COLOR0;
};
struct psIn {
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
};

// [texture] diffuse white
Texture2D tex : register(t0);
SamplerState tex_sampler;

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn output;
	float3 world = mul(float4(input.pos.xyz, 1), sk_inst[id].world).xyz;
	output.pos   = mul(float4(world, 1), sk_viewproj);

	output.normal = normalize(mul(float4(input.norm, 0), sk_inst[id].world).xyz);
	output.uv     = input.uv;
	output.color  = input.color * color;
	return output;
}

float4 ps(psIn input, bool frontface : SV_IsFrontFace) : SV_TARGET{
	float text_value = tex.Sample(tex_sampler, input.uv).r;
	clip(text_value-0.5);

	float3 albedo = input.color.rgb;
	float3 normal = normalize(input.normal) * (frontface ? -1:1);

	float w, h;
	uint mip_levels;
	sk_cubemap.GetDimensions(0, w, h, mip_levels);

	float3 irradiance = sk_cubemap.SampleLevel(tex_cube_sampler, normal, 0.8*mip_levels).rgb; // This should be Spherical Harmonics eventually

	return float4(albedo * irradiance, input.color.a);
}
)_";