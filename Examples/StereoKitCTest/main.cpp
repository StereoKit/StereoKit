#include "../../StereoKitC/stereokit.h"
using namespace sk;

#include "scene.h"
#include "demo_basics.h"
#include "demo_ui.h"
#include "demo_sprites.h"

#include <stdio.h>

solid_t     floor_solid;
matrix      floor_tr;
material_t  floor_mat;
model_t     floor_model;

scene_t demo_basics = {
	demo_basics_init,
	demo_basics_update,
	demo_basics_shutdown,
};
scene_t demo_ui = {
	demo_ui_init,
	demo_ui_update,
	demo_ui_shutdown,
};
scene_t demo_sprites = {
	demo_sprites_init,
	demo_sprites_update,
	demo_sprites_shutdown,
};

void common_init();
void common_update();
void common_shutdown();

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
const char* file_cache      = "Assets";
const char* file_cubemap    = "Assets/Sky/sky.hdr";
const char* file_tex        = "Assets/test.png";
const char* file_tex_normal = "Assets/test_normal.png";
#else
const char* file_cache      = "../../Examples/Assets";
const char* file_cubemap    = "../../Examples/Assets/Sky/sky.hdr";
const char* file_tex        = "../../Examples/Assets/test.png";
const char* file_tex_normal = "../../Examples/Assets/test_normal.png";
#endif

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
#else
int main() {
#endif
	settings_t settings = {};
	sprintf_s(settings.shader_cache, file_cache);
	sk_set_settings(settings);

	if (!sk_init("StereoKit C", runtime_mixedreality))
		return 1;

	common_init();

	scene_set_active(demo_basics);

	while (sk_step( []() {
		scene_update();
		common_update();
	}));

	scene_shutdown();
	common_shutdown();
	sk_shutdown();
	return 0;
}

void common_init() {
	tex2d_t cubemap = tex2d_create_cubemap_file(file_cubemap);
	render_set_skytex(cubemap, true);
	tex2d_release(cubemap);

	// Create a PBR floor material
	tex2d_t tex_color = tex2d_create_file(file_tex);
	tex2d_t tex_norm  = tex2d_create_file(file_tex_normal);
	floor_mat = material_create(shader_find("default/shader_pbr"));
	material_set_texture(floor_mat, "diffuse", tex_color);
	material_set_texture(floor_mat, "normal",  tex_norm);
	material_set_float  (floor_mat, "tex_scale", 6);
	material_set_float  (floor_mat, "roughness", 1.0f);
	material_set_float  (floor_mat, "metallic", 0.5f);
	material_set_queue_offset(floor_mat, 1);
	if (tex_color != nullptr) tex2d_release(tex_color);
	if (tex_norm  != nullptr) tex2d_release(tex_norm);

	// Procedurally create a cube model
	mesh_t mesh_cube = mesh_gen_cube(vec3_one, 0);
	floor_model  = model_create_mesh(mesh_cube, floor_mat);
	mesh_release(mesh_cube);
	
	// Build a physical floor!
	vec3 pos   = vec3{ 0,-1.5f,0 };
	vec3 scale = vec3{ 5,1,5 };
	floor_tr = matrix_trs(pos, quat_identity, scale);
	floor_solid = solid_create(pos, quat_identity, solid_type_immovable);
	solid_add_box (floor_solid, scale);
}
void common_update() {
	// Render floor
	render_add_model(floor_model, floor_tr);
}
void common_shutdown() {
	solid_release   (floor_solid);
	material_release(floor_mat);
	model_release   (floor_model);
}