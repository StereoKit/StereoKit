#include "../../StereoKitC/stereokit.h"

#include <vector>
using namespace std;
vector<solid_t> phys_objs;

transform_t floor_tr;
transform_t tr;
model_t     gltf;
model_t     box;

bool runningInMRMode = false;

int main() {
	if (!sk_init("StereoKit C", sk_runtime_flatscreen))
		return 1;

	tex2d_t    tex_color = tex2d_create_file("../../Examples/Assets/floor_color.jpg");
	//tex2d_t    tex_mr    = tex2d_create_file("../../Examples/Assets/floor_mr.jpg");
	tex2d_t    tex_norm  = tex2d_create_file("../../Examples/Assets/floor_normal.jpg");
	material_t floor_mat = material_create("app/material/floor", shader_find("default/shader_pbr"));
	material_set_texture(floor_mat, "diffuse", tex_color);
	//material_set_texture(floor_mat, "metal",   tex_mr);
	material_set_texture(floor_mat, "normal",  tex_norm);
	tex2d_release(tex_color);
	//tex2d_release(tex_mr);
	tex2d_release(tex_norm);

	gltf = model_create_file("../../Examples/Assets/DamagedHelmet.gltf");
	box  = model_create_mesh("app/model_cube", 
		mesh_gen_cube("app/mesh_cube", { 1,1,1 }, 0), 
		floor_mat);
	transform_initialize(tr);

	// Floor!
	transform_set(floor_tr, { 0,-1.5f,0 }, vec3{ 5,1,5 }, { 0,0,0,1 });
	solid_t floor = solid_create(floor_tr._position, floor_tr._rotation, solid_type_immovable);
	solid_add_box (floor, floor_tr._scale);

	while (sk_step( []() {
		vec3 lookat = vec3{ cosf(sk_timef()*0.1f), 0, sinf(sk_timef()*0.1f)}* .5f;

		// Do hand input
		const hand_t &hand = input_hand(handed_right);
		if (hand.state & input_state_justpinch) {
			lookat = hand.root.position;

			solid_t new_obj = solid_create({ 0,3,0 }, { 0,0,0,1 });
			solid_add_sphere(new_obj, 0.45f, 40);
			solid_add_box   (new_obj, vec3{1,1,1}*0.35f, 40);
			phys_objs.push_back(new_obj);
		}

		// Render solid helmets
		transform_set_scale(tr, vec3{ 1,1,1 }*0.25f);
		for (size_t i = 0; i < phys_objs.size(); i++) {
			solid_get_transform(phys_objs[i], tr);
			render_add_model   (gltf, tr);
		}
		
		// Render floor
		render_add_model(box, floor_tr);
	}));

	for (size_t i = 0; i < phys_objs.size(); i++) {
		solid_release(phys_objs[i]);
	}
	solid_release(floor);

	model_release(gltf);
	sk_shutdown();
	return 0;
}