#include "shader_builtin.h"

const char* sk_shader_builtin_ui = R"_(
// [name] sk/default_ui
cbuffer GlobalBuffer : register(b0) {
	float4x4 sk_view;
	float4x4 sk_proj;
	float4x4 sk_viewproj;
	float4   sk_light;
	float4   sk_light_color;
	float4   sk_camera_pos;
	float4   sk_camera_dir;
	float4   sk_fingertip[2];
	float    sk_time;
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
	// [param] color color {1, 1, 1, 1}
	float4 _color;
};
struct vsIn {
	float4 pos  : SV_POSITION;
	float3 norm : NORMAL;
	float3 col  : COLOR;
};
struct psIn {
	float4 pos   : SV_POSITION;
	float3 color : COLOR0;
	float3 world : TEXCOORD1;
	float3 normal: NORMAL;
};

// [texture] diffuse white
Texture2D tex : register(t0);
SamplerState tex_sampler;

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn output;
	output.world = mul(float4(input.pos.xyz, 1), sk_inst[id].world).xyz;
	output.pos   = mul(float4(output.world,  1), sk_viewproj);

	output.normal = normalize(mul(float4(input.norm, 0), sk_inst[id].world).xyz);

	float w, h;
	uint mip_levels;
	sk_cubemap.GetDimensions(0, w, h, mip_levels);
	float3 irradiance = sk_cubemap.SampleLevel(tex_cube_sampler, output.normal, (0.9)*mip_levels).rgb;

	output.color = _color.rgb * input.col * sk_inst[id].color.rgb * irradiance;
	return output;
}
float4 ps(psIn input) : SV_TARGET {
	float dist = 1;
	for	(int i=0;i<2;i++) {
		float3 delta = input.world - sk_fingertip[i].xyz;
		dist = min( dist, dot(delta,delta) / (0.02 * 0.02));
	}
	float3 col = lerp(float3(0,0,0), input.color, dist);

	return float4(col, _color.a); 
})_";