#include "shader_builtin.h"

extern const char* sk_shader_builtin_skybox = R"_(
// [name] sk/skybox
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
	Inst sk_inst[800];
};
TextureCube sk_cubemap : register(t11);
SamplerState tex_cube_sampler;

cbuffer ParamBuffer : register(b2) {
	// [param] float blur 0.8
	float blur;
};
struct InOut {
	float4 pos  : SV_POSITION;
	float3 norm : NORMAL;
};
struct psOut {
    float4 color : SV_Target;
    float  depth : SV_Depth;
};

InOut vs(InOut input, uint id : SV_InstanceID) {
	InOut output;
	output.pos  = mul(float4(input.pos.xyz, 0), sk_viewproj);
	output.norm = normalize(mul(float4(input.norm, 0), sk_inst[id].world).xyz);
	return output;
}

psOut ps(InOut input) {
	psOut result;

	float w, h;
	uint mip_levels;
	sk_cubemap.GetDimensions(0, w, h, mip_levels);

	result.color = sk_cubemap.SampleLevel(tex_cube_sampler, input.norm, blur*mip_levels);
	result.depth = 0.99999;
	return result;
}
)_";