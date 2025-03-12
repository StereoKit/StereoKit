#include "stereokit.hlsli"

struct vsIn {
	float4 pos : SV_Position;
};
struct psIn : sk_ps_input_t {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
};

psIn vs(vsIn input, sk_vs_input_t sk_in) {
	psIn o;
	uint view_id = sk_view_init(sk_in, o);
	uint id      = sk_inst_id  (sk_in);
	
	o.pos = float4(input.pos.xy, 1, 1);

	float4 proj_inv = mul(o.pos, sk_proj_inv[view_id]);
	o.norm = mul(float4(proj_inv.xyz, 0), transpose(sk_view[view_id])).xyz;
	return o;
}

float4 ps(psIn input) : SV_TARGET {
	return sk_cubemap.Sample(sk_cubemap_s, input.norm);
}