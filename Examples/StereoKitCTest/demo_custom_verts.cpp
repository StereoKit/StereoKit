#include "demo_custom_verts.h"

#include <stereokit.h>
#include <stereokit_ui.h>
#include <math.h>
#include <string.h>

#include "skt_vert_custom.hlsl.h"

using namespace sk;

#ifndef _countof
#define _countof(array) (sizeof(array) / sizeof(array[0]))
#endif

///////////////////////////////////////////

// A minimal vertex, just a color and a position! Component order matches
// this struct's memory, not the shader, inputs are matched by semantic.
struct vert_pos_col_t {
	color32 col;
	vec3    pos;
};
const vert_component_t vert_pos_col_fmt[] = {
	{ vert_fmt_u8_normalized, 4, vert_semantic_color,    0 },
	{ vert_fmt_f32,           3, vert_semantic_position, 0 },
};

pose_t     custom_verts_pose = {0,0,-0.5f, quat_identity};
material_t custom_verts_material;
mesh_t     custom_verts_mesh;

///////////////////////////////////////////

void demo_custom_verts_init() {
	shader_t shader = shader_create_mem((void*)sks_skt_vert_custom_hlsl, sizeof(sks_skt_vert_custom_hlsl));
	custom_verts_material = material_create(shader);
	shader_release(shader);

	// An octahedron with a color per corner
	const float   s = 0.1f;
	vert_pos_col_t verts[] = {
		{ {255,  0,  0,255}, { s, 0, 0} },
		{ {  0,255,  0,255}, {-s, 0, 0} },
		{ {  0,  0,255,255}, { 0, s, 0} },
		{ {255,255,  0,255}, { 0,-s, 0} },
		{ {  0,255,255,255}, { 0, 0, s} },
		{ {255,  0,255,255}, { 0, 0,-s} },
	};
	vind_t inds[] = {
		2,4,0,  2,0,5,  2,5,1,  2,1,4,
		3,0,4,  3,5,0,  3,1,5,  3,4,1,
	};

	custom_verts_mesh = mesh_create();
	mesh_set_data_fmt(custom_verts_mesh, vert_pos_col_fmt, _countof(vert_pos_col_fmt), verts, _countof(verts), inds, _countof(inds));

	// Quick sanity checks for the custom format APIs, results land in the
	// log where the test runner can spot them.
	bounds_t bounds = mesh_get_bounds(custom_verts_mesh);
	log_infof("custom_verts bounds:    %s <~BLK>(expected dimensions 0.2)<~clr>",
		fabsf(bounds.dimensions.x - 2*s) < 0.0001f && fabsf(bounds.dimensions.y - 2*s) < 0.0001f && fabsf(bounds.dimensions.z - 2*s) < 0.0001f
		? "<~GRN>pass<~clr>" : "<~RED>FAIL<~clr>");

	// The -z face plane satisfies x+y-z = s, so this ray should hit at
	// z = 0.02-s. Ray is offset from center to land inside a triangle.
	ray_t hit = {};
	ray_t ray = { {0.01f, 0.01f, -1}, {0, 0, 1} };
	log_infof("custom_verts intersect: %s <~BLK>(expected hit at z=%.2f)<~clr>",
		mesh_ray_intersect(custom_verts_mesh, ray, cull_back, &hit, nullptr) && fabsf(hit.pos.z - (0.02f-s)) < 0.0001f
		? "<~GRN>pass<~clr>" : "<~RED>FAIL<~clr>", 0.02f-s);

	vert_component_t* fmt       = nullptr;
	int32_t           fmt_count = 0;
	void*             raw       = nullptr;
	int32_t           raw_count = 0;
	mesh_get_verts_fmt(custom_verts_mesh, &fmt, &fmt_count, &raw, &raw_count, memory_reference);
	log_infof("custom_verts get_fmt:   %s <~BLK>(expected matching format and data)<~clr>",
		fmt_count == _countof(vert_pos_col_fmt) && memcmp(fmt, vert_pos_col_fmt, sizeof(vert_pos_col_fmt)) == 0 &&
		raw_count == _countof(verts)            && memcmp(raw, verts, sizeof(verts)) == 0
		? "<~GRN>pass<~clr>" : "<~RED>FAIL<~clr>");
}

///////////////////////////////////////////

void demo_custom_verts_update() {
	ui_handle_begin("CustomVerts", custom_verts_pose, nullptr, mesh_get_bounds(custom_verts_mesh), false);
	mesh_draw      (custom_verts_mesh, custom_verts_material, matrix_r(quat_from_angles(0, time_totalf() * 45, 0)));
	ui_handle_end  ();
}

///////////////////////////////////////////

void demo_custom_verts_shutdown() {
	mesh_release    (custom_verts_mesh);
	material_release(custom_verts_material);
}
