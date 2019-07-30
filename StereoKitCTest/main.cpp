#include "../StereoKitC/stereokit.h"

///////////////////////////////////////////

transform_t app_gltf_tr;
model_t     app_gltf;

void app_init    ();
void app_update  ();
void app_shutdown();

///////////////////////////////////////////

int main() {
	if (!sk_init("Stereo Kit", sk_runtime_flatscreen))
		return 1;
	app_init();

	while (sk_step(app_update));

	app_shutdown();
	sk_shutdown();
	return 0;
}

///////////////////////////////////////////

void app_init() {
	app_gltf = model_create_mesh("app/model_cube", 
		mesh_gen_cube  ("app/mesh_cube", { .1f,.1f,.1f }, 0), 
		material_create("default/material", nullptr));
	//app_gltf = model_create_file("Assets/FlightHelmet/FlightHelmet.gltf");
	//app_gltf = model_create_file("Assets/DamagedHelmet.gltf");
	transform_set(app_gltf_tr, { 0,0,0 }, { 1,1,1 }, { 0,0,0,1 });
}

///////////////////////////////////////////

void app_shutdown() {
	model_release(app_gltf);
}

///////////////////////////////////////////

void app_update() {
	vec3 lookat = { cosf(sk_timef()*0.1f) * .5f, 0, sinf(sk_timef()*0.1f) * .5f };

	vec3 light_dir = vec3{ cosf(sk_timef()*1.2f), -1, sinf(sk_timef()*1.2f) } / 1.414;
	render_set_light(light_dir, 1, { 1,1,1,1 });

	const hand_t &hand = input_hand(hand_right);
	if (hand.state & hand_state_pinch)
		lookat = hand.root.position;

	transform_lookat(app_gltf_tr, lookat);
	render_add_model(app_gltf, app_gltf_tr);
}