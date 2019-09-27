#include "demo_ui.h"

#include "../../StereoKitC/stereokit.h"
#include "sk_ui.h"

///////////////////////////////////////////

solid_t     ui_floor_solid;
matrix      ui_floor_tr;
material_t  ui_floor_mat;
model_t     ui_floor_model;
sprite_t    ui_sprite;

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
	vec3 pos   = vec3{ 0,-1.5f,0 };
	vec3 scale = vec3{ 5,1,5 };
	ui_floor_tr = matrix_trs(pos, quat_identity, scale);
	ui_floor_solid = solid_create(pos, quat_identity, solid_type_immovable);
	solid_add_box (ui_floor_solid, scale);

	ui_sprite = sprite_create_file("../../Examples/Assets/test.png", sprite_type_single);

	sk_ui_init();
}

///////////////////////////////////////////

void demo_ui_update() {
	// Render floor
	render_add_model(ui_floor_model, ui_floor_tr);

	sk_ui_begin_frame();

	static pose_t window_pose = //pose_t{ {0,0.5f,0},{0,0,0,1} };
		pose_t{ {0.85f,1,0.9f}, quat_lookat(vec3_zero, -vec3_one) };
	// input_hand(handed_right).root;
	sk_ui_window_begin("Main", window_pose, vec2{ 24 }*cm2m);

	sk_ui_button("Testing!\nok");
	sk_ui_button("Another");
	sk_ui_nextline();
	static float val = 0.5f;
	static float val2 = 0.5f;
	sk_ui_hslider("slider", val, 0, 1, 0.2f, 72*mm2m);
	sk_ui_hslider("slider2", val2, 0, 1, 0, 72*mm2m);
	sk_ui_nextline();
	sk_ui_image(ui_sprite, vec2{ 6,0 }*cm2m);
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
	sprite_release(ui_sprite);
	solid_release(ui_floor_solid);
	material_release(ui_floor_mat);
	model_release(ui_floor_model);
}