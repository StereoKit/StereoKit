#include "stereokit.hlslinc"

//--name = sk/skybox

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;
	float4 col  : COLOR0;
};
struct psIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;
	float4 col  : COLOR0;
	uint view_id : SV_RenderTargetArrayIndex;
};
struct psOut {
    float4 color : SV_Target;
    float  depth : SV_Depth;
};

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn output;
	output.pos     = float4(input.pos.xyz,1);
	output.view_id = sk_inst[id].view_id;
	output.uv  = 0;
	output.col = 0;

	float4 proj_inv = mul(output.pos, sk_proj_inv[output.view_id]);
	output.norm = mul(float4(proj_inv.xyz, 0), transpose(sk_view[output.view_id])).xyz;
	return output;
}

psOut ps(vsIn input) {
	psOut result;
	result.color = sk_cubemap.Sample(sk_cubemap_s, input.norm);
	result.depth = 0.99999;
	return result;
}