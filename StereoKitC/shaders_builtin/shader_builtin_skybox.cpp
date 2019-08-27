#include "shader_builtin.h"

extern const char* sk_shader_builtin_skybox = R"_(
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
	float4x4 sk_world[1000];
};
TextureCube sk_cubemap : register(t11);
SamplerState tex_cube_sampler;

cbuffer ParamBuffer : register(b2) {
	// [param] float blur default 0.8
	float blur;
};
struct vsIn {
	float4 pos  : SV_POSITION;
	float3 norm : NORMAL;
};
struct psIn {
	float4 pos    : SV_POSITION;
	float3 normal : NORMAL;
};
struct psOut {
    float4 color : SV_Target;
    float  depth : SV_Depth;
};

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn output;
	output.pos    = mul(float4(input.pos.xyz, 0), sk_viewproj);
	output.normal = normalize(mul(float4(input.norm, 0), sk_world[id]).xyz);
	return output;
}

psOut ps(psIn input) {
	psOut result;

	float w, h;
	uint mip_levels;
	sk_cubemap.GetDimensions(0, w, h, mip_levels);

	result.color = sk_cubemap.SampleLevel(tex_cube_sampler, input.normal, blur*mip_levels);
	result.depth = 0.99999;
	return result;
}
)_";