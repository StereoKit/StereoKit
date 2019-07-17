
#include "stereo_kit.h"

#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "transform.h"
#include "camera.h"

#include <directxmath.h> // Matrix math functions and objects
using namespace DirectX;

///////////////////////////////////////////

struct app_transform_buffer_t {
	XMMATRIX world;
	XMMATRIX viewproj;
};

shader_t     app_shader;
shaderargs_t app_shader_transforms;
mesh_t       app_cube;
transform_t  app_cube_transform;
tex2d_t      app_tex;
camera_t     app_camera;
transform_t  app_camera_transform;

void app_init    ();
void app_draw    ();
void app_update  ();
void app_shutdown();


///////////////////////////////////////////

int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int cmdShow) {
	sk_init("Stereo Kit");
	app_init();

	while (sk_step(app_update, app_draw));

	app_shutdown();
	sk_shutdown();
	return 0;
}

///////////////////////////////////////////

void app_init() {
	mesh_create_file  (app_cube,   "cube.obj");
	tex2d_create_file (app_tex,    "test.png");
	shader_create_file(app_shader, "shader.hlsl");
	shaderargs_create(app_shader_transforms, sizeof(app_transform_buffer_t), 0);

	transform_set(app_cube_transform, { 0,0,0 }, { 2,2,2 }, { 0,0,0,1 });
	transform_initialize(app_camera_transform);
	app_camera = {};
	app_camera.fov       = 90;
	app_camera.clip_near = 0.1f;
	app_camera.clip_far  = 50;
}

///////////////////////////////////////////

void app_shutdown() {
	tex2d_destroy(app_tex);
	mesh_destroy (app_cube);

	shader_destroy    (app_shader);
	shaderargs_destroy(app_shader_transforms);
}

///////////////////////////////////////////

void app_draw() {
	transform_set_pos(app_camera_transform, { cosf(sk_timef) * 4, 4, sinf(sk_timef) * 4 });
	transform_lookat (app_camera_transform, { 0,0,0 });

	app_transform_buffer_t transform_buffer;
	camera_viewproj (app_camera, app_camera_transform, transform_buffer.viewproj);
	transform_matrix(app_cube_transform,               transform_buffer.world);

	shader_set_active(app_shader);
	shaderargs_set_data(app_shader_transforms, &transform_buffer);
	shaderargs_set_active(app_shader_transforms);
	tex2d_set_active(app_tex, 0);

	mesh_set_active(app_cube);
	mesh_draw(app_cube);
}

///////////////////////////////////////////

void app_update() {
}