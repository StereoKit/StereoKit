#include "../StereoKitC/stereokit.h"

///////////////////////////////////////////

camera_t    app_camera;
transform_t app_camera_transform;
transform_t app_cube_tr;
transform_t app_gltf_tr;
model_t     app_cube;
model_t     app_gltf;

void app_init    ();
void app_update  ();
void app_shutdown();

///////////////////////////////////////////

//int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int cmdShow) {
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
	app_cube = model_create_file("Assets/cube.obj");
	app_gltf = model_create_file("Assets/DamagedHelmet.gltf");
	
	transform_set(app_cube_tr, { 0,0,0 }, { .1f,.1f,.1f }, { 0,0,0,1 });
	transform_set(app_gltf_tr, { 0,0,0 }, { 1,1,1 }, { 0,0,0,1 });

	transform_initialize(app_camera_transform);
	camera_initialize(app_camera, 90, 0.1f, 50);
	transform_set_position(app_camera_transform, { 1, 1, 1 });
	transform_lookat (app_camera_transform, { 0,0,0 });
	render_set_camera(app_camera, app_camera_transform);
}

///////////////////////////////////////////

void app_shutdown() {
	model_release(app_cube);
	model_release(app_gltf);
}

///////////////////////////////////////////

void app_update() {
	vec3 lookat = { cosf(sk_timef()) * .5f, 0, sinf(sk_timef()) * .5f };

	int ct = input_pointer_count();
	for (size_t i = 0; i < ct; i++) {
		pointer_t p = input_pointer(i);
		if (!(p.state & pointer_state_pressed))
			continue;

		lookat = p.ray.pos + p.ray.dir;
		transform_set_rotation(app_cube_tr, p.orientation);
		transform_set_position(app_cube_tr, lookat);
		render_add_model(app_cube, app_cube_tr);
	}

	transform_lookat(app_gltf_tr, { -lookat.x,-lookat.y,-lookat.z });
	render_add_model(app_gltf, app_gltf_tr);
}