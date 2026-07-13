#include "stereokit.hlsli"

//--name = sk/default_ui_quadrant

struct vsIn {
	float4 pos      : SV_Position;
	float3 norm     : NORMAL0;
	float2 quadrant : TEXCOORD0;
	float4 color    : COLOR0;
};
struct psIn {
	float4      pos   : SV_Position;
	float3      world : TEXCOORD1;
	min16float4 color : COLOR0;
};

psIn vs(vsIn input, sk_ids_t ids) {
	psIn o;
	float4x4 world_mat = sk_inst[ids.inst].world;

	// Quadrant offset uses the original (scaled) rows directly:
	// scale * normalized_row = original_row, so no explicit scale needed.
	// This avoids 2 sqrt transcendentals vs the old length() approach.
	float3 row0 = float3(world_mat._11, world_mat._12, world_mat._13);
	float3 row1 = float3(world_mat._21, world_mat._22, world_mat._23);
	float3 quadrant_offset = (input.quadrant.x * 0.5) * row0
	                       + (input.quadrant.y * 0.5) * row1;

	// Normalize rows 0 and 1 via rsqrt + multiply (no divide)
	world_mat[0] *= rsqrt(dot(row0, row0));
	world_mat[1] *= rsqrt(dot(row1, row1));

	float3 normal = normalize(mul(input.norm, (float3x3)world_mat));
	float4 world  = mul(input.pos, world_mat);
	world.xyz    += quadrant_offset;
	o.pos    = mul(world, sk_viewproj[ids.view]);
	o.world  = world.xyz;
	o.color.rgb = input.color.rgb * sk_inst[ids.inst].color.rgb * sk_lighting(normal);
	o.color.a   = input.color.a;
	return o;
}

min16float4 ps(psIn input) : SV_TARGET {
	min16float glow = sk_finger_glow(input.world);
	return min16float4(lerp(input.color.rgb, min16float3(1, 1, 1), glow), input.color.a);
}