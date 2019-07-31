#include "../StereoKitC/stereokit.h"

transform_t gltf_tr;
model_t     gltf;

int main() {
	sk_init("StereoKit C", sk_runtime_flatscreen);
	
	//gltf = model_create_file("Assets/DamagedHelmet.gltf");
	gltf = model_create_mesh("app/model_cube", 
		mesh_gen_cube  ("app/mesh_cube", { .1f,.1f,.1f }, 0), 
		material_create("default/material", nullptr));
	transform_set(gltf_tr, { 0,0,0 }, { 1,1,1 }, { 0,0,0,1 });

	while (sk_step( []() {
		vec3 lookat = vec3{ cosf(sk_timef()*0.1f), 0, sinf(sk_timef()*0.1f)}* .5f;

		const hand_t &hand = input_hand(handed_right);
		if (hand.state & input_state_pinch)
			lookat = hand.root.position;

		transform_lookat(gltf_tr, lookat);
		render_add_model(gltf, gltf_tr);
	}));

	model_release(gltf);
	sk_shutdown();
	return 0;
}