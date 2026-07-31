#include <stereokit.hlsli>

///////////////////////////////////////////
// Procedural looping rain: a static cloud of droplet quads in a unit
// cube, wrapped in the vertex shader so the cube follows the head while
// the droplets stay world-fixed and fall over time. Each droplet's quad
// expands into a camera-facing vertical streak diamond, and edge fade
// hides the cube boundary. Ported from the ProcIsland sky renderer,
// minus its depth-map rain occlusion.

//--rain_head_pos = 0, 0, 0, 0
//--rain_params   = 0, 20, 9, 0.15
//--rain_color    = 0.8, 0.8, 0.8, 0.3
//--rain_settings = 1, 1, -1.5, 0
float4 rain_head_pos; // xyz = head world position
float4 rain_params;   // x = time, y = cube_size, z = fall_speed, w = drop_length
float4 rain_color;    // xyz = water albedo tint, w = opacity - lit by the skylight SH
float4 rain_settings; // x = intensity (0-1), y = drop_size multiplier, z = floor world y

///////////////////////////////////////////

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;  // x = vertex index (0-3), y = per-droplet variation
	float4 col  : COLOR0;
};
struct psIn {
	float4 pos   : SV_POSITION;
	float4 color : COLOR0; // Lit tint, a = edge fade * opacity
};

///////////////////////////////////////////

// Wrap x into [0, size) — always positive, unlike fmod
float wrap(float x, float size) {
	return x - floor(x / size) * size;
}

psIn vs(vsIn input, sk_ids_t ids) {
	psIn o;

	// Discard droplets above intensity threshold
	float intensity = rain_settings.x;
	float drop_size = rain_settings.y;
	if (input.uv.y > intensity) {
		o.pos = 0.0 / 0.0;
		return o;
	}

	float  time       = rain_params.x;
	float  cube_size  = rain_params.y;
	float  fall_speed = rain_params.z;
	float  drop_len   = rain_params.w * 2 * drop_size;
	float3 head       = rain_head_pos.xyz;

	// Wrap model positions within the rain cube.
	// Head XZ offset in unit-cube space keeps droplets world-fixed
	// while the cube travels with the head via the world matrix.
	float3 head_offset = head / cube_size;

	float3 local;
	local.x = wrap(input.pos.x - head_offset.x, 1.0) - 0.5;
	local.z = wrap(input.pos.z - head_offset.z, 1.0) - 0.5;
	local.y = wrap(input.pos.y - head_offset.y - fall_speed * time / cube_size, 1.0) - 0.5;

	float3 model_pos = local * cube_size;

	// Rain stops at the ground: the world matrix is pure head translation,
	// so a droplet's world height is just model + head.
	if (model_pos.y + rain_head_pos.y < rain_settings.z) {
		o.pos = 0.0 / 0.0;
		return o;
	}

	// Expand 4 vertices into a diamond shape:
	//   0 = top tip (skinny), 1 = wide left, 2 = wide right, 3 = bottom tip (blunt)
	// Triangles via index buffer: (0,1,2) upper, (2,1,3) lower.
	// Wide point sits 70% down — skinny top, blunt bottom.
	float  width    = 0.002 * drop_size;
	int    vtx      = (int)input.uv.x;
	float  y_offset = (vtx == 0) ?  drop_len * 0.5
	                : (vtx == 3) ? -drop_len * 0.5
	                :              -drop_len * 0.2;
	float  x_offset = (vtx == 1) ? -width
	                : (vtx == 2) ?  width
	                :               0;

	// Camera-facing billboard (horizontal axis only — rain stays vertical)
	// In model space, camera is at the origin
	float3 to_cam = -model_pos;
	to_cam.y      = 0;
	float  len_xz = length(to_cam);
	float3 right  = (len_xz > 0.001)
		? float3(-to_cam.z / len_xz, 0, to_cam.x / len_xz)
		: float3(1, 0, 0);

	model_pos    += right * x_offset;
	model_pos.y  += y_offset;

	// A streak is a thin vertical cylinder: the wide verts sit on its
	// silhouette so their normals point along the billboard's right axis
	// (parallel to the image plane), and the tips round off to +-Y.
	float3 normal = (vtx == 0) ? float3(0,  1, 0)
	              : (vtx == 3) ? float3(0, -1, 0)
	              : (vtx == 1) ? -right
	              :               right;

	// Full skylight SH per vertex: the SH carries the sky's own color,
	// brightness, and direction (and the lightning flash), so rain_color
	// is just a water albedo tint on top - the one knob for the look.
	float3 lit = sk_lighting(normal) * rain_color.rgb;

	// Standard StereoKit transform: model -> world -> clip
	float4 world = mul(float4(model_pos, 1), sk_inst[ids.inst].world);
	o.pos        = mul(world, sk_viewproj[ids.view]);

	// Fade at cube edges to prevent visible wrap popping
	float dist_y = abs(local.y * 2);
	float dist_h = length(float2(local.x, local.z)) * 2;
	float edge   = max(dist_y, dist_h);
	float alpha  = saturate(1.0 - smoothstep(0.8, 1.0, edge));

	o.color = float4(lit.x, lit.y, lit.z, alpha * rain_color.a);
	return o;
}

///////////////////////////////////////////

float4 ps(psIn input) : SV_TARGET {
	return input.color;
}
