#include "demo_draw.h"

#include <stereokit.h>
#include <stereokit_ui.h>

using namespace sk;

///////////////////////////////////////////

pose_t     draw_pose = {0,0,-0.5f, quat_identity};
material_t draw_material;
mesh_t     draw_mesh;

///////////////////////////////////////////

void demo_draw_init() {
	draw_material = material_find        (default_id_material);
	draw_mesh     = mesh_gen_rounded_cube(vec3_one * 0.1f, 0.01f, 2);

	material_set_queue_offset(draw_material, 20);
}

///////////////////////////////////////////

void demo_draw_update() {
	ui_handle_begin("Draw", draw_pose, mesh_get_bounds(draw_mesh), false);
	mesh_draw      (draw_mesh, draw_material, matrix_identity);
	ui_handle_end  ();
}

///////////////////////////////////////////

void demo_draw_shutdown() {
	mesh_release    (draw_mesh);
	material_release(draw_material);
}