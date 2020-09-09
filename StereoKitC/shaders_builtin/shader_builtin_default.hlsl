#include "stereokit.hlslinc"

//--name = sk/default

//--color:color = 1,1,1,1
//--tex_scale   = 1
//--diffuse     = white

float4       color;
float        tex_scale;
Texture2D    diffuse   : register(t0);
SamplerState diffuse_s : register(s0);

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL;
	float4 col  : COLOR;
	float2 uv   : TEXCOORD0;
};
struct psIn {
	float4 pos   : SV_Position;
	float4 color : COLOR0;
	float2 uv    : TEXCOORD0;
	uint view_id : SV_RenderTargetArrayIndex;
};

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn output;
	float4 world = mul(input.pos, sk_inst[id].world);
	output.pos   = mul(world,     sk_viewproj[sk_inst[id].view_id]);

	float3 normal = normalize(mul(input.norm, (float3x3)sk_inst[id].world));

	output.view_id = sk_inst[id].view_id;
	output.uv      = input.uv * tex_scale;
	output.color   = color * input.col * sk_inst[id].color;
	output.color.rgb *= Lighting(normal);
	return output;
}

float4 ps(psIn input) : SV_TARGET {
	float4 col = diffuse.Sample(diffuse_s, input.uv);
	return col * input.color;
}