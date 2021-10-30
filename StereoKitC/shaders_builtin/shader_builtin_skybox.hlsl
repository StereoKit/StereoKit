#include "stereokit.hlsli"

//--name = sk/skybox

struct vsIn {
	float4 pos : SV_Position;
};
struct psIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	uint view_id : SV_RenderTargetArrayIndex;
};
struct psOut {
	float4 color : SV_Target;
	float  depth : SV_Depth;
};

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn o;
	o.view_id = id % sk_view_count;
	id        = id / sk_view_count;
	o.pos     = float4(input.pos.xyz, 1);

	float4   proj_inv       = mul(o.pos, sk_proj_inv[o.view_id]);
	float4x4 v              = sk_view[o.view_id];
	float4x4 transpose_view = float4x4(
		v._11, v._21, v._31, v._41,
		v._12, v._22, v._32, v._42,
		v._13, v._23, v._33, v._43,
		v._14, v._24, v._34, v._44 );
	o.norm = mul(float4(proj_inv.xyz, 0), transpose_view).xyz;
	return o;
}

psOut ps(psIn input) {
	psOut result;
	result.color = sk_cubemap.Sample(sk_cubemap_s, input.norm);
	result.depth = 0.99999;
	return result;
}