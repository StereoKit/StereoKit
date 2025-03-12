#include "stereokit.hlsli"

//--color:color = 1,1,1,1
//--tex_trans   = 0,0,1,1
//--diffuse     = white

float4       color;
float4       tex_trans;
Texture2D    diffuse   : register(t0);
SamplerState diffuse_s : register(s0);

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;
	float4 col  : COLOR0;
};
struct psIn : sk_ps_input_t {
	float4 pos   : SV_Position;
	float2 uv    : TEXCOORD0;
	float4 color : COLOR0;
};

psIn vs(vsIn input, sk_vs_input_t sk_in) {
	psIn o;
	uint view_id = sk_view_init(sk_in, o);
	uint id      = sk_inst_id  (sk_in);

	float4 world = mul(input.pos, sk_inst[id].world);
	o.pos        = mul(world,     sk_viewproj[view_id]);

	float3 normal = normalize(mul(input.norm, (float3x3)sk_inst[id].world));

	o.uv         = (input.uv * tex_trans.zw) + tex_trans.xy;
	o.color      = color * input.col * sk_inst[id].color;
	o.color.rgb *= sk_lighting(normal);
	return o;
}

float4 ps(psIn input) : SV_TARGET {
	float4 col = diffuse.Sample(diffuse_s, input.uv);
	return col * input.color;
}