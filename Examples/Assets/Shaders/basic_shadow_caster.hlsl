#include "stereokit.hlsli"

///////////////////////////////////////////

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;
	float4 col  : COLOR0;
};
struct psIn : sk_ps_input_t {
	float4 pos : SV_Position;
};

///////////////////////////////////////////

psIn vs(vsIn input, sk_vs_input_t sk_in) {
	psIn o;
	uint view_id = sk_view_init(sk_in, o);
	uint id      = sk_inst_id  (sk_in);

	float4 world = mul(input.pos, sk_inst[id].world);
	o.pos        = mul(world,     sk_viewproj[view_id]);
	return o;
}

///////////////////////////////////////////

float4 ps(psIn input) : SV_TARGET {
	return float4(1, 1, 1, 1);
}