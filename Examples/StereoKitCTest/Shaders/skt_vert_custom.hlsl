#include "stereokit.hlsli"

//--name = app/vert_custom

// Paired with the custom pos+color vertex format in demo_custom_verts.
// Mesh components match these inputs by semantic, order doesn't matter.
struct vsIn {
	float4 pos : SV_Position;
	float4 col : COLOR0;
};
struct psIn {
	float4 pos : SV_POSITION;
	float4 col : COLOR0;
};

psIn vs(vsIn input, sk_ids_t ids) {
	psIn o;

	float4 world = mul(input.pos, sk_inst[ids.inst].world);
	o.pos = mul(world, sk_viewproj[ids.view]);
	o.col = input.col;
	return o;
}

float4 ps(psIn input) : SV_TARGET {
	return input.col;
}
