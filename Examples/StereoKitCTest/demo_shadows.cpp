#include "demo_shadows.h"

#include "skt_shadow.hlsl.h"
#include "skt_shadow_caster.hlsl.h"

#include <stereokit.h>
#include <stereokit_ui.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

using namespace sk;

///////////////////////////////////////////

// Must match the cbuffer in basic_shadow.hlsl
struct shadow_buffer_t {
	matrix shadowmap_transform;
	vec3   light_direction;
	float  shadowmap_bias;
	vec3   light_color;
	float  shadowmap_pixel_size;
};

///////////////////////////////////////////

static const float shadow_map_size       = 2.0f;
static const int   shadow_map_resolution = 1024;
static const float shadow_map_near_clip  = 0.01f;
static const float shadow_map_far_clip   = 20.0f;
static const int   shadow_map_variant    = 1;
static const int   shadow_buffer_slot    = 13;

static tex_t             shadow_map    = {};
static material_buffer_t shadow_buffer = {};
static vec3              light_dir     = {0.577f, 0.577f, 0.0f}; // normalized (1,1,0)

static model_t    shadow_model      = {};
static pose_t     shadow_model_pose = {};

static spherical_harmonics_t old_lighting = {};
static tex_t                 old_tex      = {};

///////////////////////////////////////////

static vec3 quantize_light_pos(vec3 pos, quat light_orientation, float texel_size);
static model_t generate_shadow_model(material_t floor_mat, material_t material);

///////////////////////////////////////////

void demo_shadows_init() {
	// Create shadow buffer
	shadow_buffer = material_buffer_create(sizeof(shadow_buffer_t));

	// Create shadow map texture (depth target)
	shadow_map = tex_create(tex_type_depthtarget, tex_format_depth16);
	tex_set_colors     (shadow_map, shadow_map_resolution, shadow_map_resolution, nullptr);
	tex_set_sample     (shadow_map, tex_sample_linear);
	tex_set_sample_comp(shadow_map, tex_sample_comp_less_or_eq);
	tex_set_address    (shadow_map, tex_address_clamp);

	// Create shadow caster material
	shader_t   caster_shader = shader_create_mem((void*)sks_skt_shadow_caster_hlsl, sizeof(sks_skt_shadow_caster_hlsl));
	material_t caster_mat    = material_create(caster_shader);
	material_set_depth_clamp(caster_mat, true);
	material_set_depth_test (caster_mat, depth_test_less_or_eq);
	shader_release(caster_shader);

	// Create shadow receiver material
	shader_t   shadow_shader = shader_create_mem((void*)sks_skt_shadow_hlsl, sizeof(sks_skt_shadow_hlsl));
	material_t shadow_mat    = material_create(shadow_shader);
	material_set_variant(shadow_mat, shadow_map_variant, caster_mat);
	shader_release(shadow_shader);

	// Create floor material (copy of shadow mat with texture)
	material_t floor_mat = material_copy(shadow_mat);
	tex_t floor_tex = tex_create_file("floor.png");
	material_set_texture(floor_mat, "diffuse", floor_tex);
	material_set_vector4(floor_mat, "tex_trans", vec4{0, 0, 2, 2});
	tex_release(floor_tex);

	// Generate the model
	shadow_model = generate_shadow_model(floor_mat, shadow_mat);

	// Clean up materials (model keeps references)
	material_release(caster_mat);
	material_release(shadow_mat);
	material_release(floor_mat);

	// Set up initial pose
	shadow_model_pose.position    = vec3{0, -0.4f, -0.5f};
	shadow_model_pose.orientation = quat_identity;

	// Save old lighting
	old_lighting = render_get_skylight();
	old_tex      = render_get_skytex();

	// Load environment map and update lighting when loaded
	tex_t env_tex = tex_create_cubemap_file("old_depot.hdr");
	render_set_skytex(env_tex);
	tex_on_load      (env_tex, [](tex_t t, void*) {
		spherical_harmonics_t lighting = tex_get_cubemap_lighting(t);
		render_set_skylight(lighting);
		// Update light direction from dominant light
		light_dir = sh_dominant_dir(lighting);
		light_dir = vec3_normalize(light_dir);
	}, nullptr);
	tex_release(env_tex); // tex_on_load keeps a reference

	// Bind shadow buffer globally
	render_global_buffer(shadow_buffer_slot, shadow_buffer);
}

///////////////////////////////////////////

static void setup_shadow_map(vec3 light_direction) {
	// Position the center of the shadow map in front of the user
	pose_t head        = input_head();
	vec3   head_fwd    = head.orientation * vec3_forward;
	vec3   head_fwd_xz = vec3_normalize(vec3{head_fwd.x, 0, head_fwd.z});
	vec3   head_pos_xz = vec3{head.position.x, 0, head.position.z};
	vec3   forward_pos = head_pos_xz + head_fwd_xz * 0.5f * shadow_map_size;

	quat light_orientation = quat_lookat_up(vec3_zero, light_direction, vec3_up);
	vec3 light_pos = quantize_light_pos(
		forward_pos + light_direction * -10.0f,
		light_orientation,
		shadow_map_size / shadow_map_resolution
	);

	// Create rendering matrices for the shadow map
	matrix view = matrix_trs(light_pos, light_orientation, vec3_one);
	matrix proj = matrix_orthographic(shadow_map_size, shadow_map_size, shadow_map_near_clip, shadow_map_far_clip);

	// Calculate shadow bias
	float depth_range = shadow_map_far_clip - shadow_map_near_clip;
	float texel_size  = shadow_map_size / shadow_map_resolution;
	float bias = 2.0f * fmaxf(depth_range / 65535.0f, texel_size);

	// Send shadow map parameters to the shader
	shadow_buffer_t buffer_data = {};
	buffer_data.shadowmap_transform = matrix_transpose(matrix_invert(view) * proj);
	buffer_data.shadowmap_bias      = bias;
	buffer_data.light_direction     = -light_direction;
	buffer_data.light_color         = vec3{1, 1, 1};
	buffer_data.shadowmap_pixel_size = 1.0f / shadow_map_resolution;
	material_buffer_set_data(shadow_buffer, &buffer_data);

	// Render the shadow map
	// First unbind the texture so we can render to it
	render_global_texture(shadow_buffer_slot, nullptr);

	// Render to shadow map (filter out VFX layer)
	render_layer_ layer_filter = (render_layer_)(render_layer_all & ~render_layer_vfx);
	render_to(shadow_map, 0, view, proj, layer_filter, shadow_map_variant, render_clear_all, rect_t{});

	// Rebind the shadow map for reading
	render_global_texture(shadow_buffer_slot, shadow_map);
}

///////////////////////////////////////////

void demo_shadows_update() {
	setup_shadow_map(light_dir);

	// UI handle for the model
	bounds_t model_bounds = model_get_bounds(shadow_model);
	ui_handle_begin("ShadowModel", shadow_model_pose, model_bounds, false, ui_move_exact);
	model_draw(shadow_model, matrix_identity);
	ui_handle_end();
}

///////////////////////////////////////////

void demo_shadows_shutdown() {
	// Restore old lighting
	render_set_skylight(old_lighting);
	render_set_skytex  (old_tex);

	// Unbind global resources
	render_global_buffer (shadow_buffer_slot, nullptr);
	render_global_texture(shadow_buffer_slot, nullptr);

	// Release resources
	tex_release            (shadow_map);
	material_buffer_release(shadow_buffer);
	model_release          (shadow_model);
	tex_release            (old_tex);
}

///////////////////////////////////////////

static model_t generate_shadow_model(material_t floor_mat, material_t material) {
	const float width   = 0.5f;
	const float height  = 0.5f;
	vec3 size_min = {0.04f, 0.04f, 0.04f};
	vec3 size_max = {0.06f, 0.2f,  0.06f};
	float gen_width  = width  - size_max.x;
	float gen_height = height - size_max.z;

	model_t model     = model_create();
	mesh_t  cube_mesh = mesh_find(default_id_mesh_cube);

	// Add floor
	model_node_add(model, "Floor", matrix_s(vec3{width, 0.02f, height}), cube_mesh, floor_mat);

	// Add random cubes
	srand(1); // Fixed seed for reproducibility
	for (int i = 0; i < 20; i++) {
		float x = ((rand() / (float)RAND_MAX) - 0.5f) * gen_width;
		float y = ((rand() / (float)RAND_MAX) - 0.5f) * gen_height;
		vec3 size = {
			size_min.x + (rand() / (float)RAND_MAX) * (size_max.x - size_min.x),
			size_min.y + (rand() / (float)RAND_MAX) * (size_max.y - size_min.y),
			size_min.z + (rand() / (float)RAND_MAX) * (size_max.z - size_min.z)
		};

		char name[32];
		snprintf(name, sizeof(name), "Cube%d", i);
		model_node_add(model, name, matrix_ts(vec3{x, 0.01f + size.y/2, y}, size), cube_mesh, material);
	}

	mesh_release(cube_mesh);
	return model;
}

///////////////////////////////////////////

static vec3 quantize_light_pos(vec3 pos, quat light_orientation, float texel_size) {
	vec3 right = light_orientation * vec3_right;
	vec3 up    = light_orientation * vec3_up;
	vec3 fwd   = light_orientation * vec3_forward;

	// Create plane perpendicular to light direction
	plane_t p = {fwd, 0};

	// Project position onto plane
	vec3 local_p   = plane_point_closest(p, pos);
	vec3 local_off = pos - local_p;

	// Get 2D coordinates on the plane
	float plane_x = vec3_dot(local_p, right);
	float plane_y = vec3_dot(local_p, up);

	// Quantize to texel grid
	float tex_x = roundf(plane_x / texel_size) * texel_size;
	float tex_y = roundf(plane_y / texel_size) * texel_size;

	// Reconstruct position
	return local_off + right * tex_x + up * tex_y;
}
