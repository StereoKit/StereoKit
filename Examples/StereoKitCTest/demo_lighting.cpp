#include "demo_lighting.h"
#include "skt_lighting.h"

#include "skt_default_lighting.hlsl.h"
#include "skt_light_only.hlsl.h"

#include <stdlib.h>

using namespace sk;

material_t mat_light;
material_t mat_lit;
material_t mat_lit_only;
mesh_t     mesh_sphere;

skt_light_id_t lights[3];

spherical_harmonics_t old_light = {};

// This sample demonstrates advanced use of StereoKit's material and shader
// systems to manually implement a simple point light system. See 
// skt_lighting.h for the lighting system itself.

void demo_lighting_init() {
	// Set up some lights
	skt_lighting_init();
	lights[0] = skt_lighting_add_light(vec3_zero, color_hsv(rand() / (float)RAND_MAX, 0.8f, 1, 1), 2, 1);
	lights[1] = skt_lighting_add_light(vec3_zero, color_hsv(rand() / (float)RAND_MAX, 0.8f, 1, 1), 2, 1);
	lights[2] = skt_lighting_add_light(vec3_zero, color_hsv(rand() / (float)RAND_MAX, 0.8f, 1, 1), 2, 1);

	// Load up some assets
	shader_t lit_shader      = shader_create_mem((void*)sks_skt_default_lighting_hlsl, sizeof(sks_skt_default_lighting_hlsl));
	shader_t lit_only_shader = shader_create_mem((void*)sks_skt_light_only_hlsl,       sizeof(sks_skt_light_only_hlsl));
	mat_light    = material_find  (default_id_material_unlit);
	mat_lit      = material_create(lit_shader);
	mat_lit_only = material_create(lit_only_shader);
	mesh_sphere  = mesh_find      (default_id_mesh_sphere);
	shader_release(lit_shader);
	shader_release(lit_only_shader);

	// Attach the lighting only material to the world/occludion mesh, for
	// 'world affecting' lighting
	world_set_occlusion_enabled (true);
	world_set_occlusion_material(mat_lit_only);

	// Set the environment to be a little dimmer, so the lights stand out.
	old_light = render_get_skylight();
	spherical_harmonics_t new_light = old_light;
	sh_brightness      (new_light, 0.5f);
	render_set_skylight(new_light);
}

void demo_lighting_update() {
	vec3  center = { 0,0,-0.5f };
	float time   = time_totalf();
	for (int32_t i = 0; i < 3; i++) {
		skt_lighting_set_pos(lights[i], vec3{cosf(time*(i+1)), 0, sinf(time*(i+1))} + center);
		render_add_mesh(mesh_sphere, mat_light,
			matrix_trs(skt_lighting_get_pos(lights[i]), quat_identity, vec3_one*0.02f),
			skt_lighting_get_color(lights[i]));
	}

	render_add_mesh(mesh_sphere, mat_lit, matrix_trs(center, quat_identity, vec3_one * 0.2f));

	skt_lighting_step();
}

void demo_lighting_shutdown() {
	render_set_skylight(old_light);
	skt_lighting_shutdown();

	material_release(mat_light);
	material_release(mat_lit);
	material_release(mat_lit_only);
	mesh_release    (mesh_sphere);
}