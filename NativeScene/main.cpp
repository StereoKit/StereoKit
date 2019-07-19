#include "stereo_kit.h"

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

int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int cmdShow) {
	if (!sk_init("Stereo Kit", sk_runtime_win32))
		return 1;
	app_init();

	while (sk_step(app_update));

	app_shutdown();
	sk_shutdown();
	return 0;
}

///////////////////////////////////////////

void app_init() {
	mesh_create_file  (app_cube,   "cube.obj");
	tex2d_create_file (app_tex,    "test.png");
	shader_create_file(app_shader, "shader.hlsl");
	app_cube_material = { &app_shader, &app_tex };
	transform_set(app_cube_transform, { 0,0,.5f }, { .2f,.2f,.2f }, { 0,0,0,1 });

	transform_initialize(app_camera_transform);
	camera_initialize(app_camera, 90, 0.1f, 50);
	render_set_camera(app_camera, app_camera_transform);
}

///////////////////////////////////////////

void app_shutdown() {
	tex2d_destroy (app_tex);
	mesh_destroy  (app_cube);
	shader_destroy(app_shader);
}

///////////////////////////////////////////

void app_update() {
	transform_set_pos(app_camera_transform, { cosf(sk_timef) * 1, 1, sinf(sk_timef) * 1 });
	transform_lookat (app_camera_transform, { 0,0,0 });

	render_add(app_cube, app_cube_material, app_cube_transform);
}