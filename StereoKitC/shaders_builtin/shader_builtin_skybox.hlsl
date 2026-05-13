#include "stereokit.hlsli"

//--name = sk/cubemap

struct vsIn {
	float4 pos : SV_Position;
};
struct psIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
};

psIn vs(vsIn input, sk_ids_t ids) {
	psIn o;

	// Force the mesh to always have furthest depth (z=1), setting depth in VS
	// is significantly faster than setting in PS. The mesh should be already
	// in clip space.
	o.pos = float4(input.pos.xy, 1, 1);

	float4 proj_inv = mul(o.pos, sk_proj_inv[ids.view]);
	o.norm = mul(float4(proj_inv.xyz, 0), transpose(sk_view[ids.view])).xyz;
	return o;
}

min16float4 ps(psIn input) : SV_TARGET {
	// Sample directly from mip 0 here, this bypasses any trilinear sampling or
	// derivative calculations for improved performance. This does require a
	// cubemap texture that is the correct size for the screen!
	return sk_cubemap.SampleLevel(sk_cubemap_s, input.norm, 0);
}