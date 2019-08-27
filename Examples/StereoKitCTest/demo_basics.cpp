#include "demo_basics.h"

#include "../../StereoKitC/stereokit.h"
#include <vector>
using namespace std;

///////////////////////////////////////////

vector<solid_t> phys_objs;

solid_t     floor_solid;
transform_t floor_tr;
material_t  floor_mat;

transform_t tr;
model_t     gltf;
model_t     box;

font_t       font;
text_style_t font_style;
material_t   font_mat;
transform_t  text_tr;

///////////////////////////////////////////

void demo_basics_init() {
	tex2d_t cubemap = tex2d_create_cubemap_file("../../Examples/Assets/Sky/sky.hdr");
	render_set_skytex(cubemap, true);
	tex2d_release(cubemap);

	font_mat   = material_create("app/font_segoe", shader_find("default/shader_font"));
	font       = font_create("C:/Windows/Fonts/segoeui.ttf");
	font_style = text_make_style(font, font_mat, text_align_x_right | text_align_y_center);
	transform_set(text_tr, vec3_up*0.1f, vec3_one*unit_cm(5), quat_identity);

	// Create a PBR floor material
	tex2d_t tex_color = tex2d_create_file("../../Examples/Assets/test.png");
	tex2d_t tex_norm  = tex2d_create_file("../../Examples/Assets/test_normal.png");
	floor_mat = material_create("app/material_floor", shader_find("default/shader_pbr"));
	material_set_texture(floor_mat, "diffuse", tex_color);
	material_set_texture(floor_mat, "normal",  tex_norm);
	material_set_float  (floor_mat, "tex_scale", 6);
	material_set_float  (floor_mat, "roughness", 1.0f);
	material_set_float  (floor_mat, "metallic", 0.5f);
	material_set_queue_offset(floor_mat, 1);
	if (tex_color != nullptr) tex2d_release(tex_color);
	if (tex_norm  != nullptr) tex2d_release(tex_norm);

	// Procedurally create a cube model
	mesh_t mesh_cube = mesh_gen_cube("app/mesh_cube", vec3_one, 0);
	box  = model_create_mesh("app/model_cube", mesh_cube, floor_mat);
	mesh_release(mesh_cube);

	// Load a gltf model
	gltf = model_create_file("../../Examples/Assets/DamagedHelmet.gltf");

	// Build a physical floor!
	transform_set(floor_tr, { 0,-1.5f,0 }, vec3{ 5,1,5 }, quat_identity);
	floor_solid = solid_create(floor_tr._position, floor_tr._rotation, solid_type_immovable);
	solid_add_box (floor_solid, floor_tr._scale);
}

///////////////////////////////////////////

void demo_basics_update() {
	// Do hand input
	if (input_hand(handed_right).state & input_state_justpinch) {
		solid_t new_obj = solid_create({ 0,3,0 }, quat_identity);
		solid_add_sphere(new_obj, 0.45f, 40);
		solid_add_box   (new_obj, vec3_one*0.35f, 40);
		phys_objs.push_back(new_obj);
	}

	// Render solid helmets
	transform_set_scale(tr, vec3_one*0.25f);
	for (size_t i = 0; i < phys_objs.size(); i++) {
		solid_get_transform(phys_objs[i], tr);
		render_add_model   (gltf, tr);
	}

	// Render floor
	render_add_model(box, floor_tr);

	transform_set_position(text_tr, input_hand(handed_right).root.position);
	transform_set_rotation(text_tr, input_hand(handed_right).root.orientation);
	const char *txt = "Testing spaces!!!\n\tAnd newlines?\tAnd Tabs.\nAnother line\n<3";
	vec2 txt_size = text_size(font_style, txt);
	text_add_at(font_style, text_tr, txt, 0, 0);
}

///////////////////////////////////////////

void demo_basics_shutdown() {
	// Release everything
	for (size_t i = 0; i < phys_objs.size(); i++)
		solid_release(phys_objs[i]);
	solid_release(floor_solid);
	model_release(gltf);
	material_release(floor_mat);
	model_release(box);
}