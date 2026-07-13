#include "stereokit.hlsli"

///////////////////////////////////////////

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;
	float4 col  : COLOR0;
};
struct psIn {
	float4 pos     : SV_Position;
};

///////////////////////////////////////////

psIn vs(vsIn input, sk_ids_t ids) {
	psIn o;

	float4 world = mul(input.pos, sk_inst[ids.inst].world);
	o.pos        = mul(world,     sk_viewproj[ids.view]);
	return o;
}

///////////////////////////////////////////

float4 ps(psIn input) : SV_TARGET {
	return float4(1, 1, 1, 1);
}
