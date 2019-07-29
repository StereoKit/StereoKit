#include "../StereoKitC/stereokit.h"

///////////////////////////////////////////

transform_t app_gltf_tr;
model_t     app_gltf;
model_t     app_hand;
transform_t app_hand_tr;

void app_init    ();
void app_update  ();
void app_shutdown();

///////////////////////////////////////////

//int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int cmdShow) {
int main() {
	if (!sk_init("Stereo Kit", sk_runtime_mixedreality))
		return 1;
	app_init();

	while (sk_step(app_update));

	app_shutdown();
	sk_shutdown();
	return 0;
}

///////////////////////////////////////////

void app_init() {
	app_hand = model_create_mesh("app/model_cube", mesh_gen_cube("app/mesh_cube", { .1f,.1f,.1f }, 0), material_create("default/material", nullptr));// model_create_file("Assets/cube.obj");
	app_gltf = model_create_file("Assets/FlightHelmet/FlightHelmet.gltf");
	//app_gltf = model_create_file("Assets/DamagedHelmet.gltf");
	//app_gltf = model_create_mesh("app/model_cube", nullptr, nullptr);
	transform_set(app_gltf_tr, { 0,0,0 }, { 1,1,1 }, { 0,0,0,1 });
}

///////////////////////////////////////////

void app_shutdown() {
	model_release(app_gltf);
	model_release(app_hand);
}

///////////////////////////////////////////

void app_update() {
	vec3 lookat = { cosf(sk_timef()*0.1f) * .5f, 0, sinf(sk_timef()*0.1f) * .5f };

	vec3 light_dir = { cosf(sk_timef()*1.2f) * 1, -1, sinf(sk_timef()*1.2f) * 1 };
	light_dir = vec3_normalize(light_dir);
	render_set_light(light_dir, 1, { 1,1,1,1 });

	int ct = input_pointer_count();
	for (size_t i = 0; i < ct; i++) {
		pointer_t p = input_pointer(i);

		transform_set(app_hand_tr, p.ray.pos, { 0.01f, 0.04f, 0.05f }, p.orientation);
		//render_add_model(app_hand, app_hand_tr);

		if (p.state & pointer_state_pressed)
			lookat = p.ray.pos;// +p.ray.dir * 1.0f;
	}

	transform_lookat(app_gltf_tr, lookat);
	render_add_model(app_gltf, app_gltf_tr);
}