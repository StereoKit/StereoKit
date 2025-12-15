#include "stereokit.hlsli"

struct vsIn {
	float4 pos : SV_Position;
};
struct psIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	uint view_id : SV_RenderTargetArrayIndex;
};

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn o;
	o.view_id = id % sk_view_count;
	id        = id / sk_view_count;

	o.pos = float4(input.pos.xy, 1, 1);

	float4 proj_inv = mul(o.pos, sk_proj_inv[o.view_id]);
	o.norm = mul(float4(proj_inv.xyz, 0), transpose(sk_view[o.view_id])).xyz;
	return o;
}

float4 ps(psIn input) : SV_TARGET {
	return sk_cubemap.Sample(sk_cubemap_s, input.norm);
}