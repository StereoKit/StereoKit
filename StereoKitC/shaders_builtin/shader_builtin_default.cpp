#include "shader_builtin.h"

const char* sk_shader_builtin_default = R"_(
// [name] sk/default
cbuffer GlobalBuffer : register(b0) {
	float4x4 view;
	float4x4 proj;
	float4x4 viewproj;
	float4   light;
	float4   light_color;
	float4   camera_pos;
	float4   camera_dir;
};
struct Inst {
	float4x4 world;
	float4   color;
};
cbuffer TransformBuffer : register(b1) {
	Inst sk_inst[800];
};
cbuffer ParamBuffer : register(b2) {
	// [ param ] vector color {1, 1, 1, 1} tags { data for whatever! }
	float4 _color;
};
struct vsIn {
	float4 pos  : SV_POSITION;
	float3 norm : NORMAL;
	float3 col  : COLOR;
	float2 uv   : TEXCOORD0;
};
struct psIn {
	float4 pos   : SV_POSITION;
	float3 color : COLOR0;
	float2 uv    : TEXCOORD0;
	float3 world : TEXCOORD1;
};

// [texture] diffuse white
Texture2D tex : register(t0);
SamplerState tex_sampler;

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn output;
	output.world = mul(float4(input.pos.xyz, 1), sk_inst[id].world).xyz;
	output.pos   = mul(float4(output.world, 1), viewproj);

	float3 normal = normalize(mul(float4(input.norm, 0), sk_inst[id].world).xyz);

	output.uv    = input.uv;
	output.color = lerp(float3(0.0,0.0,0.0), light_color.rgb, saturate(dot(normal, -light.xyz))) * input.col * sk_inst[id].color.rgb;
	return output;
}
float4 ps(psIn input) : SV_TARGET {
	float3 col   = tex.Sample(tex_sampler, input.uv).rgb;

	col = col * input.color * _color.rgb;

	return float4(col, _color.a); 
})_";