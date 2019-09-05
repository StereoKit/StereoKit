#include "demo_ui.h"

#include "../../StereoKitC/stereokit.h"
#include "sk_ui.h"

///////////////////////////////////////////

solid_t     ui_floor_solid;
transform_t ui_floor_tr;
material_t  ui_floor_mat;
model_t     ui_floor_model;

///////////////////////////////////////////

void demo_ui_init() {
	tex2d_t cubemap = tex2d_create_cubemap_file("../../Examples/Assets/Sky/sky.hdr");
	render_set_skytex(cubemap, true);
	tex2d_release(cubemap);

	// Create a PBR floor material
	tex2d_t tex_color = tex2d_create_file("../../Examples/Assets/test.png");
	tex2d_t tex_norm  = tex2d_create_file("../../Examples/Assets/test_normal.png");
	ui_floor_mat = material_create("app/material_floor", shader_find("default/shader_pbr"));
	material_set_texture(ui_floor_mat, "diffuse", tex_color);
	material_set_texture(ui_floor_mat, "normal",  tex_norm);
	material_set_float  (ui_floor_mat, "tex_scale", 6);
	material_set_float  (ui_floor_mat, "roughness", 1.0f);
	material_set_float  (ui_floor_mat, "metallic", 0.5f);
	material_set_queue_offset(ui_floor_mat, 1);
	if (tex_color != nullptr) tex2d_release(tex_color);
	if (tex_norm  != nullptr) tex2d_release(tex_norm);

	// Procedurally create a cube model
	mesh_t mesh_cube = mesh_gen_cube("app/mesh_cube", vec3_one, 0);
	ui_floor_model = model_create_mesh("app/model_cube", mesh_cube, ui_floor_mat);
	mesh_release(mesh_cube);

	// Build a physical floor!
	transform_set(ui_floor_tr, { 0,-1.5f,0 }, vec3{ 5,1,5 }, quat_identity);
	ui_floor_solid = solid_create(ui_floor_tr._position, ui_floor_tr._rotation, solid_type_immovable);
	solid_add_box (ui_floor_solid, ui_floor_tr._scale);

	sk_ui_init();
}

///////////////////////////////////////////

void demo_ui_update() {
	// Render floor
	render_add_model(ui_floor_model, ui_floor_tr);

	sk_ui_begin_frame();
	//sk_ui_window_begin("Main", pose_t{ {0.85f,1,0.9f},{0,0,0,1} });
	sk_ui_window_begin("Main", pose_t{ {0,0.5f,0},{0,0,0,1} });
	//sk_ui_window_begin("Main", input_hand(handed_right).root);

	sk_ui_button("Testing!\nok");
	sk_ui_button("Another");
	sk_ui_nextline();
	if (sk_ui_button("Press me!")) {
		sk_ui_nextline();
		sk_ui_button("DYANMIC BUTTON!!");
	}

	sk_ui_window_end();
}

///////////////////////////////////////////

void demo_ui_shutdown() {
	// Release everything
	solid_release(ui_floor_solid);
	material_release(ui_floor_mat);
	model_release(ui_floor_model);
}