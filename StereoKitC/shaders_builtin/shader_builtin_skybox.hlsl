// [name] sk/skybox

#include <stereokit>

cbuffer ParamBuffer : register(b2) {
	// [param] float blur 0.0
	float blur;
};
struct vsIn {
	float4 pos  : SV_POSITION;
	float3 norm : NORMAL0;
};
struct psIn {
	float4 pos  : SV_POSITION;
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

	result.color = sk_cubemap.SampleLevel(tex_cube_sampler, input.norm, blur*mip_levels);
	result.depth = 0.99999;
	return result;
}