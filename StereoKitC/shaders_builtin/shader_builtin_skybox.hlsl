#include "stereokit.hlslinc"

//--name = sk/skybox
//--blur = 0

float blur; 

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;
	float4 col  : COLOR0;
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
	psIn output;
	output.pos     = mul(float4(input.pos.xyz, 0), sk_viewproj[sk_inst[id].view_id]);
	output.view_id = sk_inst[id].view_id;
	output.norm    = input.norm;
	return output;
}

psOut ps(vsIn input) {
	psOut result;

	float w, h;
	uint mip_levels;
	sk_cubemap.GetDimensions(0, w, h, mip_levels);

	result.color = sk_cubemap.SampleLevel(sk_cubemap_s, input.norm, blur * mip_levels);
	result.depth = 0.99999;
	return result;
}