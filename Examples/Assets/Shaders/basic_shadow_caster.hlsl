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
	uint   view_id : SV_RenderTargetArrayIndex;
};

///////////////////////////////////////////

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn o;
	o.view_id = id % sk_view_count;
	id        = id / sk_view_count;

	float4 world = mul(input.pos, sk_inst[id].world);
	o.pos        = mul(world,     sk_viewproj[o.view_id]);
	return o;
}

///////////////////////////////////////////

float4 ps(psIn input) : SV_TARGET {
	return float4(1, 1, 1, 1);
}