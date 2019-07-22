#include "../StereoKitC/stereokit.h"

///////////////////////////////////////////

camera_t    app_camera;
transform_t app_camera_transform;
shader_t    app_shader;
tex2d_t     app_tex;
mesh_t      app_cube;
transform_t app_cube_transform;
material_t  app_cube_material;

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
	app_cube          = mesh_create_file  ("Assets/cube.obj");
	app_tex           = tex2d_create_file ("Assets/test.png");
	app_shader        = shader_create_file("Assets/shader.hlsl");
	app_cube_material = material_create   (app_shader, app_tex);
	transform_set(app_cube_transform, { 0,0,.5f }, { .5f,.5f,.5f }, { 0,0,0,1 });

	transform_initialize(app_camera_transform);
	camera_initialize(app_camera, 90, 0.1f, 50);
	transform_set_pos(app_camera_transform, { 1, 1, 1 });
	transform_lookat (app_camera_transform, { 0,0,0 });
	render_set_camera(app_camera, app_camera_transform);
}

///////////////////////////////////////////

void app_shutdown() {
	tex2d_release   (app_tex);
	mesh_release    (app_cube);
	shader_release  (app_shader);
	material_release(app_cube_material);
}

///////////////////////////////////////////

void app_update() {
	transform_set_scale(app_cube_transform, { 0.5f,0.5f,0.5f });
	transform_set_pos(app_cube_transform, { cosf(sk_timef()) * .5f, 0, sinf(sk_timef()) * .5f });
	transform_lookat (app_cube_transform, { 0,0,0 });

	render_add(app_cube, app_cube_material, app_cube_transform);

	transform_set_scale(app_cube_transform, { 0.1f,0.1f,0.1f });
	int ct = input_pointer_count();
	for (size_t i = 0; i < ct; i++) {
		transform_set_pos(app_cube_transform, input_pointer(i).ray.pos);
		render_add(app_cube, app_cube_material, app_cube_transform);
	}
}