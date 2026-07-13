#include "stereokit.hlsli"
//--name = sk/depth_prepass

Texture2DArray depth_tex   : register(t1);
SamplerState   depth_tex_s : register(s1);

float4x4 depth_view_proj_inv_l = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
float4x4 depth_view_proj_inv_r = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
float4   depth_dims            = float4(1, 1, 1, 1);

struct psIn {
	float4 pos : SV_POSITION;
};

psIn vs(uint id : SV_VertexID, sk_ids_t ids) {
	psIn o;

	// Derive grid UV from vertex ID and depth texture dimensions
	uint  gw       = (uint)depth_dims.x;
	float2 depth_uv = float2(
		((float)(id % gw) + 0.5) * depth_dims.z,
		((float)(id / gw) + 0.5) * depth_dims.w);

	float depth_raw = depth_tex.SampleLevel(depth_tex_s, float3(depth_uv, (float)ids.view), 0).r;

	// Invalid depth -> NaN position to guarantee triangle cull
	if (depth_raw <= 0 || depth_raw >= 1) {
		o.pos = asfloat(0x7FC00000);
		return o;
	}

	// Unproject depth texel to world space via inverse depth VP
	// depth_raw is a z-buffer value in [0,1], convert to NDC [-1,1]
	float4x4 dvp_inv = (ids.view == 0) ? depth_view_proj_inv_l : depth_view_proj_inv_r;
	float4 world_h   = mul(dvp_inv, float4(depth_uv * 2.0 - 1.0, depth_raw * 2.0 - 1.0, 1));
	float3 world_pos = world_h.xyz / world_h.w;

	// Project through rendering camera for correct z-buffer depth
	o.pos = mul(float4(world_pos, 1), sk_viewproj[ids.view]);
	return o;
}

float4 ps(psIn input) : SV_TARGET {
	return 0;
}
