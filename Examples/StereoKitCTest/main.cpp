#include "../../StereoKitC/stereokit.h"

transform_t floor_tr;
transform_t tr;
model_t     gltf;
model_t     box;

solid_t solid;
solid_t hand_solid;

bool runningInMRMode = false;

int main() {
	if (!sk_init("StereoKit C", sk_runtime_mixedreality))
		return 1;

	tex2d_t    tex       = tex2d_create_file("../../Examples/Assets/test.png");
	material_t floor_mat = material_create("app/material/floor", shader_find("default/shader_pbr"));
	material_set_texture(floor_mat, "diffuse", tex);

	gltf = model_create_file("../../Examples/Assets/DamagedHelmet.gltf");
	box  = model_create_mesh("app/model_cube", 
		mesh_gen_cube("app/mesh_cube", { 1,1,1 }, 0), 
		floor_mat);
	transform_set(tr, { 0,0,0 }, { 1,1,1 }, { 0,0,0,1 });
	solid = solid_create({ 0,10,0 }, { 0,0,0,1 });
	solid_add_sphere(solid, 0.05f);

	transform_set(floor_tr, { 0,-1.5f,0 }, vec3{ 10,1,10 }, { 0,0,0,1 });
	solid_t floor = solid_create(floor_tr._position, floor_tr._rotation, solid_type_immovable);
	solid_add_box (floor, floor_tr._scale);

	hand_solid = solid_create({ 0,0,0 }, { 0,0,0,1 }, solid_type_unaffected);
	solid_set_enabled(hand_solid, false);
	solid_add_box    (hand_solid, vec3{ 0.1f, .03f, .2f });

	while (sk_step( []() {
		vec3 lookat = vec3{ cosf(sk_timef()*0.1f), 0, sinf(sk_timef()*0.1f)}* .5f;

		const hand_t &hand = input_hand(handed_right);
		if (hand.state & input_state_pinch)
			lookat = hand.root.position;
		solid_set_enabled(hand_solid, hand.state & input_state_tracked);
		solid_set_pose   (hand_solid, hand.root.position, hand.root.orientation);
		solid_transform(hand_solid, tr);
		render_add_model(box, tr);

		transform_set_position(tr, vec3{ 0,0,0 });
		transform_lookat(tr, vec3{ 0,0,0 }-lookat);
		//render_add_model(gltf, gltf_tr);

		transform_set_scale(tr, vec3{ 1,1,1 }*0.25f);
		solid_transform(solid, tr);
		render_add_model(gltf, tr);

		render_add_model(box, floor_tr);
	}));

	model_release(gltf);
	sk_shutdown();
	return 0;
}