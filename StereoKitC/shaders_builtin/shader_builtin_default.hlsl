#include "stereokit.hlsli"
//--name = sk/default_shader

float4 color     = float4(1,1,1,1);
float4 tex_trans = float4(0,0,1,1);

//--diffuse = white
Texture2D    diffuse   : register(t0);
SamplerState diffuse_s : register(s0);

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;
	float4 col  : COLOR0;
};
struct psIn {
	float4      pos   : SV_Position;
	float2      uv    : TEXCOORD0;
	min16float4 color : COLOR0;
};

psIn vs(vsIn input, sk_ids_t ids) {
	psIn o;
	float4 world = mul(input.pos, sk_inst[ids.inst].world);
	o.pos        = mul(world,     sk_viewproj[ids.view]);

	float3 normal = normalize(mul(input.norm, (float3x3)sk_inst[ids.inst].world));

	o.uv         = (input.uv * tex_trans.zw) + tex_trans.xy;
	o.color      = color * input.col * sk_inst[ids.inst].color;
	o.color.rgb *= sk_lighting(normal);
	return o;
}

min16float4 ps(psIn input) : SV_TARGET {
	return diffuse.Sample(diffuse_s, input.uv) * input.color;
}