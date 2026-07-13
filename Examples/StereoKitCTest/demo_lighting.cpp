#include "demo_lighting.h"
#include "skt_lighting.h"

#include "skt_default_lighting.hlsl.h"
#include "skt_light_only.hlsl.h"

#include <stdlib.h>

using namespace sk;

material_t mat_light;
material_t mat_lit;
mesh_t     mesh_sphere;
shader_t   old_occlusion_shader;

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
	mat_light   = material_find  (default_id_material_unlit);
	mat_lit     = material_create(lit_shader);
	mesh_sphere = mesh_find      (default_id_mesh_sphere);
	shader_release(lit_shader);

	// Replace the shader on the default world occlusion material so it
	// responds to our custom lighting system.
	material_t occlusion_mat = material_find("sk/world/material");
	old_occlusion_shader = material_get_shader(occlusion_mat);
	material_set_shader(occlusion_mat, lit_only_shader);
	material_release(occlusion_mat);
	shader_release(lit_only_shader);

	world_set_occlusion(occlusion_caps_mesh);

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

	// Restore the original shader on the occlusion material
	material_t occlusion_mat = material_find("sk/world/material");
	material_set_shader(occlusion_mat, old_occlusion_shader);
	material_release(occlusion_mat);
	shader_release(old_occlusion_shader);

	material_release(mat_light);
	material_release(mat_lit);
	mesh_release    (mesh_sphere);
}