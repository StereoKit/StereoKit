/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

#include "render.h"
#include "render_.h"
#include "world.h"
#include "defaults.h"
#include "../_stereokit.h"
#include "../device.h"
#include "../libraries/stref.h"
#include "../libraries/profiler.h"
#include "../sk_math_dx.h"
#include "../sk_memory.h"
#include "../spherical_harmonics.h"
#include "../stereokit.h"
#include "../hierarchy.h"
#include "../asset_types/mesh.h"
#include "../asset_types/texture.h"
#include "../asset_types/shader.h"
#include "../asset_types/material.h"
#include "../asset_types/model.h"
#include "../asset_types/animation.h"
#include "../systems/input.h"
#include "../hands/input_hand.h"
#include "../platforms/platform.h"

#include <sk_renderer.h>
#include <limits.h>
#include <stdio.h>

#define STBIW_WINDOWS_UTF8
#include "../libraries/stb_image_write.h"

using namespace DirectX;

namespace sk {

///////////////////////////////////////////

struct render_transform_buffer_t {
	XMMATRIX world;
	color128 color;
};
struct render_global_buffer_t {
	XMMATRIX view[2];
	XMMATRIX proj[2];
	XMMATRIX proj_inv[2];
	XMMATRIX viewproj[2];
	vec4     lighting[9];
	vec4     camera_pos[2];
	vec4     camera_dir[2];
	vec4     fingertip[2];
	vec4     cubemap_i;
	float    time;
	uint32_t view_count;
	uint32_t eye_offset;
};
struct render_blit_data_t {
	float width;
	float height;
	float pixel_width;
	float pixel_height;
};
struct render_screenshot_t {
	void        (*render_on_screenshot_callback)(color32* color_buffer, int32_t width, int32_t height, void* context);
	void*         context;
	matrix        camera;
	matrix        projection;
	rect_t        viewport;
	int32_t       width;
	int32_t       height;
	render_layer_ layer_filter;
	render_clear_ clear;
	tex_format_   tex_format;
};

struct render_viewpoint_t {
	tex_t         rendertarget;
	int32_t       rendertarget_index;
	matrix        camera;
	matrix        projection;
	rect_t        viewport;
	render_layer_ layer_filter;
	render_clear_ clear;
	material_t    override_material;
};

enum render_action_type_ {
	render_action_type_none,
	render_action_type_viewpoint,
	render_action_type_global_texture,
	render_action_type_global_buffer,
};

struct render_action_viewpoint_t {
	tex_t         rendertarget;
	int32_t       rendertarget_index;
	matrix        camera;
	matrix        projection;
	rect_t        viewport;
	render_layer_ layer_filter;
	int32_t       material_variant;
	render_clear_ clear;
};

struct render_action_global_texture_t {
	tex_t   texture;
	int32_t slot;
};

struct render_action_global_buffer_t {
	material_buffer_t buffer;
	int32_t           slot;
};

struct render_action_t {
	render_action_type_ type;
	union {
		render_action_viewpoint_t      viewpoint;
		render_action_global_texture_t global_texture;
		render_action_global_buffer_t  global_buffer;
	};
};

///////////////////////////////////////////

struct render_state_t {
	bool32_t                initialized;
	skr_vert_type_t         default_vert_type;
	skr_render_list_t       gpu_render_list;

	material_buffer_t       shader_globals;
	skr_buffer_t            shader_blit;
	matrix                  camera_root;
	matrix                  camera_root_final;
	matrix                  camera_root_final_inv;
	quat                    camera_root_final_rot;
	matrix                  default_camera_proj;
	matrix                  sim_origin;
	matrix                  sim_head;

	projection_             projection_type;
	vec2                    clip_planes;
	float                   fov;
	float                   ortho_near_clip;
	float                   ortho_far_clip;
	float                   ortho_viewport_height;

	render_global_buffer_t  global_buffer;
	mesh_t                  blit_quad;
	vec4                    lighting[9];
	spherical_harmonics_t   lighting_src;
	color128                clear_col;
	render_list_t           list_primary;
	float                   viewport_scale;
	float                   scale;
	int32_t                 multisample;
	render_layer_           primary_filter;
	render_layer_           capture_filter;
	bool                    use_capture_filter;
	tex_t                   global_textures[16];
	material_buffer_t       global_buffers [16];

	array_t<render_screenshot_t> screenshot_list;
	array_t<render_action_t>     render_action_list;

	mesh_t                  sky_mesh;
	material_t              sky_mat;
	material_t              sky_mat_default;
	bool32_t                sky_show;
	tex_t                   sky_pending_tex;

	array_t<render_list_t>  list_stack;
	render_list_t           list_active;
};
static render_state_t local = {};

const int32_t render_skytex_register = 11;

///////////////////////////////////////////

void render_save_to_file(color32* color_buffer, int width, int height, void* context);
void render_list_add    (const render_item_t *item);
void render_list_add_to (render_list_t list, const render_item_t *item);

///////////////////////////////////////////

bool render_init() {
	profiler_zone();

	local = {};

	// Initialize the default vertex type for vert_t
	skr_vert_component_t vert_components[] = {
		{ skr_vertex_fmt_f32,            3, skr_semantic_position, 0, 0 },
		{ skr_vertex_fmt_f32,            3, skr_semantic_normal,   0, 0 },
		{ skr_vertex_fmt_f32,            2, skr_semantic_texcoord, 0, 0 },
		{ skr_vertex_fmt_ui8_normalized, 4, skr_semantic_color,    0, 0 },
	};
	skr_vert_type_create(vert_components, _countof(vert_components), &local.default_vert_type);
	local.initialized           = true;
	local.sim_origin            = matrix_identity;
	local.sim_head              = matrix_identity;
	local.clip_planes           = {0.02f, 50};
	local.fov                   = 90;
	local.ortho_near_clip       = 0.0f;
	local.ortho_far_clip        = 50.0f;
	local.ortho_viewport_height = 1.0f;
	local.clear_col             = color128{0,0,0,0};
	local.list_primary          = nullptr;
	local.viewport_scale        = 1;
	local.scale                 = sk_get_settings_ref()->render_scaling;
	local.multisample           = sk_get_settings_ref()->render_multisample;
	local.primary_filter        = render_layer_all_first_person;
	local.capture_filter        = render_layer_all_first_person;
	local.list_active           = nullptr;

	local.shader_globals  = material_buffer_create(sizeof(local.global_buffer));
	skr_buffer_create(nullptr, 1, sizeof(render_blit_data_t), skr_buffer_type_constant, skr_use_dynamic, &local.shader_blit);
	skr_buffer_set_name(&local.shader_blit, "sk/render/blit_buffer");

	skr_render_list_create(&local.gpu_render_list);

	// Setup a default camera
	render_set_clip         (local.clip_planes.x, local.clip_planes.y);
	render_set_cam_root     (matrix_identity);
	render_update_projection();

	// Set up resources for doing blit operations
	local.blit_quad = mesh_find(default_id_mesh_screen_quad);

	// Create a default skybox
	local.sky_mesh = mesh_create();
	vind_t inds [] = {2,1,0, 3,2,0};
	vert_t verts[] = {
		vert_t{ {-1, 1,1}, {0,0,1}, {0,0}, {255,255,255,255} },
		vert_t{ { 1, 1,1}, {0,0,1}, {1,0}, {255,255,255,255} },
		vert_t{ { 1,-1,1}, {0,0,1}, {1,1}, {255,255,255,255} },
		vert_t{ {-1,-1,1}, {0,0,1}, {0,1}, {255,255,255,255} }, };
	mesh_set_data(local.sky_mesh, verts, _countof(verts), inds, _countof(inds));
	mesh_set_id  (local.sky_mesh, "sk/render/skybox_mesh");

	// Create a default skybox material
	shader_t shader_sky = shader_find(default_id_shader_sky);
	local.sky_mat_default = material_create(shader_sky);
	material_set_id          (local.sky_mat_default, "sk/render/skybox_material");
	material_set_queue_offset(local.sky_mat_default, 100);
	material_set_depth_write (local.sky_mat_default, false);
	material_set_depth_test  (local.sky_mat_default, depth_test_less_or_eq);
	render_set_skymaterial(local.sky_mat_default);
	shader_release(shader_sky);

	// Create a default skybox texture
	tex_t sky_cubemap = tex_find(default_id_cubemap);
	render_set_skytex   (sky_cubemap);
	render_set_skylight (sk_default_lighting);
	render_enable_skytex(true);
	tex_release(sky_cubemap);
	
	local.list_primary = render_list_create();
	render_list_set_id(local.list_primary, "sk/render/primary_renderlist");
	render_list_push  (local.list_primary);

	hierarchy_init();

	return true;
}

///////////////////////////////////////////

void render_shutdown() {
	render_list_pop();
	render_list_release(local.list_active);
	render_list_release(local.list_primary);
	local.list_stack        .free();
	local.screenshot_list   .free();
	local.render_action_list.free();

	for (int32_t i = 0; i < _countof(local.global_textures); i++) {
		tex_release(local.global_textures[i]);
		local.global_textures[i] = nullptr;
	}
	for (int32_t i = 0; i < _countof(local.global_buffers); i++) {
		material_buffer_release(local.global_buffers[i]);
		local.global_buffers[i] = {};
	}
	tex_release            (local.sky_pending_tex);
	material_release       (local.sky_mat_default);
	material_release       (local.sky_mat);
	mesh_release           (local.sky_mesh);
	mesh_release           (local.blit_quad);
	material_buffer_release(local.shader_globals);

	skr_buffer_destroy     (&local.shader_blit);
	skr_render_list_destroy(&local.gpu_render_list);
	skr_vert_type_destroy  (&local.default_vert_type);

	local = {};

	hierarchy_shutdown();
}

///////////////////////////////////////////

const skr_vert_type_t* render_get_default_vert() {
	return &local.default_vert_type;
}

///////////////////////////////////////////

void render_step() {
	profiler_zone();

	hierarchy_step();

	if (local.sky_show && device_display_get_blend() == display_blend_opaque) {
		render_add_mesh(local.sky_mesh, local.sky_mat, matrix_identity, {1,1,1,1}, render_layer_vfx);
	}
}

///////////////////////////////////////////

inline uint64_t render_sort_id(material_t material, mesh_t mesh) {
	return ((uint64_t)(material->alpha_mode*1000 + material->queue_offset) << 32) | (material->header.index << 16) | mesh->header.index;
}
inline uint64_t render_sort_id_from_queue(int32_t queue_position) {
	return (uint64_t)(queue_position) << 32;
}

///////////////////////////////////////////

void render_set_clip(float near_plane, float far_plane) {
	// near_plane will throw divide by zero errors if it's zero! So we'll
	// clamp it :) Anything this low will probably look bad due to depth
	// artifacts though.
	near_plane = fmaxf(0.001f, near_plane);
	local.clip_planes = { near_plane, far_plane };
	render_update_projection();
}

///////////////////////////////////////////

void render_get_clip(float* out_near_plane, float* out_far_plane) {
	if (out_near_plane) *out_near_plane = local.clip_planes.x;
	if (out_far_plane ) *out_far_plane  = local.clip_planes.y;
}

///////////////////////////////////////////

void render_set_fov(float vertical_field_of_view_degrees) {
	local.fov = vertical_field_of_view_degrees;
	render_update_projection();
}

///////////////////////////////////////////

float render_get_fov() {
	return local.fov;
}

///////////////////////////////////////////

void render_set_ortho_size(float viewport_height_meters) {
	local.ortho_viewport_height = viewport_height_meters;
	render_update_projection();
}

///////////////////////////////////////////

float render_get_ortho_size() {
	return local.ortho_viewport_height;
}

///////////////////////////////////////////

void render_set_ortho_clip(float near_plane, float far_plane) {
	local.ortho_near_clip = near_plane;
	local.ortho_far_clip  = far_plane;
	render_update_projection();
}

///////////////////////////////////////////

void render_set_projection(projection_ proj) {
	local.projection_type = proj;
	render_update_projection();
}

///////////////////////////////////////////

projection_ render_get_projection() {
	return local.projection_type;
}

///////////////////////////////////////////

float render_get_ortho_view_height() {
	return local.ortho_viewport_height;
}

///////////////////////////////////////////

void render_update_projection() {
	if (!local.initialized) return;

	// Both paths need the aspect ratio
	float aspect = (float)device_display_get_width() / device_display_get_height();

	if (local.projection_type == projection_perspective) {
		
		math_fast_to_matrix(
			XMMatrixPerspectiveFovRH(
				local.fov * deg2rad,
				aspect,
				local.clip_planes.x,
				local.clip_planes.y),
			&local.default_camera_proj);

		// Update the FoV data
		backend_xr_type_ type = backend_xr_get_type();
		if (type == backend_xr_type_none ||
			type == backend_xr_type_simulator) {

			fov_info_t fov;
			fov.top    = local.fov * 0.5f;
			fov.bottom = -fov.top;
			fov.right  =  fov.top * aspect;
			fov.left   = -fov.right;
			device_data.display_fov = fov;
		}
	} else {

		math_fast_to_matrix(
			XMMatrixOrthographicRH(
				aspect*local.ortho_viewport_height,
				local.ortho_viewport_height,
				local.ortho_near_clip,
				local.ortho_far_clip),
			&local.default_camera_proj);
	}
}

///////////////////////////////////////////

const char *render_fmt_name(tex_format_ format) {
	switch (format) {
	case tex_format_bgra32:        return "bgra32_sRGB";
	case tex_format_bgra32_linear: return "bgra32_linear";
	case tex_format_rgba32:        return "rgba32_sRGB";
	case tex_format_rgba32_linear: return "rgba32_linear";
	case tex_format_rgb10a2:       return "rgb10a2";
	case tex_format_rg11b10:       return "rg11b10";
	case tex_format_rgba64u:       return "rgba64u";
	case tex_format_rgba64s:       return "rgba64s";
	case tex_format_rgba64f:       return "rgba64f";
	case tex_format_rgba128:       return "rgba128";
	case tex_format_r8:            return "r8";
	case tex_format_r16:           return "r16";
	case tex_format_r32:           return "r32";
	case tex_format_depthstencil:  return "depth24_stencil8";
	case tex_format_depth32:       return "depth32";
	case tex_format_depth16:       return "depth16";
	case tex_format_none:          return "none";
	default:                       return "Unknown";
	}
}

///////////////////////////////////////////

tex_format_ render_preferred_depth_fmt() {
	depth_mode_ mode = sk_get_settings().depth_mode;
	switch (mode) {
	case depth_mode_default:
#if defined(SK_OS_WINDOWS_UWP) || defined(SK_OS_ANDROID)
		return tex_format_depth16;
#else
		return tex_format_depth32;
#endif
		break;
	case depth_mode_d16:     return tex_format_depth16;
	case depth_mode_d32:     return tex_format_depth32;
	case depth_mode_stencil: return tex_format_depthstencil;
	default: return tex_format_depth16;
	}
}

///////////////////////////////////////////

matrix render_get_projection_matrix() {
	return local.default_camera_proj;
}

///////////////////////////////////////////

vec2 render_get_clip() {
	return local.clip_planes;
}

///////////////////////////////////////////

matrix render_get_cam_root() {
	return local.camera_root;
}

///////////////////////////////////////////

matrix render_get_cam_final() {
	return local.camera_root_final;
}

///////////////////////////////////////////

matrix render_get_cam_final_inv() {
	return local.camera_root_final_inv;
}

///////////////////////////////////////////

pose_t render_cam_final_transform(pose_t local_space) {
	return pose_t{ 
		matrix_transform_pt(local.camera_root_final, local_space.position),
		local_space.orientation * local.camera_root_final_rot
	};
}

///////////////////////////////////////////

render_list_t render_get_primary_list() {
	render_list_addref(local.list_primary);
	return local.list_primary;
}

///////////////////////////////////////////

void render_set_cam_root(const matrix &cam_root) {
	local.camera_root           = cam_root;
	local.camera_root_final     = local.sim_head * cam_root * local.sim_origin;
	local.camera_root_final_rot = matrix_extract_rotation(local.camera_root_final);
	matrix_inverse(local.camera_root_final, local.camera_root_final_inv);
}

///////////////////////////////////////////

void render_set_sim_origin(pose_t pose) {
	local.sim_origin = pose_matrix_inv(pose);
	render_set_cam_root(render_get_cam_root());
}

///////////////////////////////////////////

void render_set_sim_head(pose_t pose) {
	local.sim_head = pose_matrix(pose);
	render_set_cam_root(render_get_cam_root());
}

///////////////////////////////////////////

void render_check_pending_skytex() {
	if (local.sky_pending_tex == nullptr) return;

	asset_state_ state = tex_asset_state(local.sky_pending_tex);

	// Check if it's errored out
	if (state < 0) {
		tex_release(local.sky_pending_tex);
		local.sky_pending_tex = nullptr;
		return;
	}

	// Check if it's a valid asset, we'll know for sure once the metadata is
	// loaded.
	if (state >= asset_state_loaded_meta && (local.sky_pending_tex->type & tex_type_cubemap) == 0) {
		log_warnf("Skytex must be a cubemap, ignoring %s", tex_get_id(local.sky_pending_tex));

		tex_release(local.sky_pending_tex);
		local.sky_pending_tex = nullptr;
		return;
	}

	// Check if it's loaded
	if (state >= asset_state_loaded) {
		render_global_texture(render_skytex_register, local.sky_pending_tex);

		tex_release(local.sky_pending_tex);
		local.sky_pending_tex = nullptr;
		return;
	}
}

///////////////////////////////////////////

void render_set_skytex(tex_t sky_texture) {
	if (sky_texture == nullptr) return;

	tex_addref(sky_texture);
	local.sky_pending_tex = sky_texture;

	// This is also checked every step, but if the texture is already valid, we
	// can set it up right away and avoid any potential frame delays.
	render_check_pending_skytex();
}

///////////////////////////////////////////

tex_t render_get_skytex() {
	if (local.sky_pending_tex != nullptr) {
		tex_addref(local.sky_pending_tex);
		return local.sky_pending_tex;
	}

	if (local.global_textures[render_skytex_register] != nullptr) {
		tex_addref(local.global_textures[render_skytex_register]);
		return local.global_textures[render_skytex_register];
	}

	return nullptr;
}

///////////////////////////////////////////

void render_set_skymaterial(material_t sky_material) {
	// Don't allow null, fall back to the default sky material on null.
	if (sky_material == nullptr) {
		sky_material = local.sky_mat_default;
	}

	// Safe swap the material reference
	material_addref(sky_material);
	if (local.sky_mat != nullptr) material_release(local.sky_mat);
	local.sky_mat = sky_material;
}

///////////////////////////////////////////

material_t render_get_skymaterial(void) {
	material_addref(local.sky_mat);
	return local.sky_mat;
}

///////////////////////////////////////////

void render_set_skylight(const spherical_harmonics_t &light_info) {
	local.lighting_src = light_info;
	sh_to_fast(light_info, local.lighting);
}

///////////////////////////////////////////

spherical_harmonics_t render_get_skylight() {
	return local.lighting_src;
}

///////////////////////////////////////////

render_layer_ render_get_filter() {
	return local.primary_filter;
}

///////////////////////////////////////////

void render_set_filter(render_layer_ layer_filter) {
	local.primary_filter = layer_filter;
}

///////////////////////////////////////////

void render_set_scaling(float texture_scale) {
	local.scale = fminf(2, fmaxf(0.2f, texture_scale));
}

///////////////////////////////////////////

float render_get_scaling() {
	return local.scale;
}

///////////////////////////////////////////

void render_set_viewport_scaling(float viewport_rect_scale) {
	local.viewport_scale = fmaxf(0,fminf(1,viewport_rect_scale));
}

///////////////////////////////////////////

float render_get_viewport_scaling(void) {
	return local.viewport_scale;
}

///////////////////////////////////////////

void render_set_multisample(int32_t display_tex_multisample) {
	if      (display_tex_multisample <= 1)  local.multisample = 1;
	else if (display_tex_multisample <= 3)  local.multisample = 2;
	else if (display_tex_multisample <= 7)  local.multisample = 4;
	else                                    local.multisample = 8;
}

///////////////////////////////////////////

int32_t render_get_multisample() {
	return local.multisample;
}

///////////////////////////////////////////

void render_override_capture_filter(bool32_t use_override_filter, render_layer_ layer_filter) {
	local.use_capture_filter = use_override_filter;
	local.capture_filter    = layer_filter;
}

///////////////////////////////////////////

render_layer_ render_get_capture_filter() {
	return local.use_capture_filter
		? local.capture_filter
		: local.primary_filter;
}

///////////////////////////////////////////

bool32_t render_has_capture_filter() {
	return local.use_capture_filter;
}

///////////////////////////////////////////

void render_enable_skytex(bool32_t show_sky) {
	local.sky_show = show_sky;
}

///////////////////////////////////////////

bool32_t render_enabled_skytex() {
	return local.sky_show;
}

///////////////////////////////////////////

void render_global_texture_internal(int32_t register_slot, tex_t texture) {
	if (local.global_textures[register_slot] == texture) return;

	if (local.global_textures[register_slot] != nullptr)
		tex_release(local.global_textures[register_slot]);

	local.global_textures[register_slot] = texture;

	if (local.global_textures[register_slot] != nullptr)
		tex_addref(local.global_textures[register_slot]);
}

///////////////////////////////////////////

void render_global_texture(int32_t register_slot, tex_t texture) {
	if (register_slot < 0 || register_slot >= _countof(local.global_textures)) {
		log_errf("render_global_texture: Register_slot should be 0-16. Received %d.", register_slot);
		return;
	}

	if (texture != nullptr) tex_addref(texture);

	render_action_t action = {};
	action.type = render_action_type_global_texture;
	action.global_texture.texture = texture;
	action.global_texture.slot    = register_slot;
	local.render_action_list.add(action);
}

///////////////////////////////////////////

void render_global_buffer_internal(int32_t register_slot, material_buffer_t buffer) {
	if (register_slot >= 0) {
		if (buffer)
			material_buffer_addref(buffer);
		if (local.global_buffers[register_slot] != nullptr)
			material_buffer_release(local.global_buffers[register_slot]);
		local.global_buffers[register_slot] = buffer;
	}
}

///////////////////////////////////////////

void render_global_buffer(int32_t register_slot, material_buffer_t buffer) {
	if ((register_slot >= 0 && register_slot < 3) || register_slot >= (sizeof(local.global_buffers) / sizeof(local.global_buffers[0]))) {
		log_errf("render_global_buffer: bad slot id '%d', use 3-%d.", register_slot, (sizeof(local.global_buffers) / sizeof(local.global_buffers[0])) - 1);
		return;
	}

	if (buffer) material_buffer_addref(buffer);

	render_action_t action = {};
	action.type = render_action_type_global_buffer;
	action.global_buffer.buffer = buffer;
	action.global_buffer.slot   = register_slot;
	local.render_action_list.add(action);
}

///////////////////////////////////////////

void render_set_clear_color(color128 color) {
	local.clear_col = color_to_linear(color);
}

//////////////////////////////////////////

color128 render_get_clear_color() {
	return color_to_gamma(local.clear_col);
}

//////////////////////////////////////////

color128 render_get_clear_color_ln() {
	return local.clear_col;
}

///////////////////////////////////////////

void render_add_mesh(mesh_t mesh, material_t material, const matrix &transform, color128 color_linear, render_layer_ layer) {
	render_list_add_mesh(local.list_active, mesh, material, transform, color_linear, layer);
}

///////////////////////////////////////////

void render_add_model_mat(model_t model, material_t material_override, const matrix& transform, color128 color_linear, render_layer_ layer) {
	render_list_add_model_mat(local.list_active, model, material_override, transform, color_linear, layer);
}

///////////////////////////////////////////

void render_add_model(model_t model, const matrix &transform, color128 color_linear, render_layer_ layer) {
	render_list_add_model_mat(local.list_active, model, nullptr, transform, color_linear, layer);
}

///////////////////////////////////////////

void render_draw_queue(render_list_t list, const matrix *views, const matrix *projections, int32_t eye_offset, int32_t view_count, int32_t inst_multiplier, render_layer_ filter, int32_t material_variant) {
	// A temporary fix for multiview trying to render to mono rendertargets
	if (view_count == 1) {
		memset(&local.global_buffer.view      [1], 0, sizeof(local.global_buffer.view      [1]));
		memset(&local.global_buffer.proj      [1], 0, sizeof(local.global_buffer.proj      [1]));
		memset(&local.global_buffer.proj_inv  [1], 0, sizeof(local.global_buffer.proj_inv  [1]));
		memset(&local.global_buffer.viewproj  [1], 0, sizeof(local.global_buffer.viewproj  [1]));
		memset(&local.global_buffer.camera_pos[1], 0, sizeof(local.global_buffer.camera_pos[1]));
		memset(&local.global_buffer.camera_dir[1], 0, sizeof(local.global_buffer.camera_dir[1]));
	}

	// Copy camera information into the global buffer
	for (int32_t i = 0; i < view_count; i++) {
		XMMATRIX view_f, projection_f;
		math_matrix_to_fast(views      [eye_offset+i], &view_f      );
		math_matrix_to_fast(projections[eye_offset+i], &projection_f);

		XMMATRIX view_inv = XMMatrixInverse(nullptr, view_f      );
		XMMATRIX proj_inv = XMMatrixInverse(nullptr, projection_f);

		XMVECTOR cam_pos = XMVector3Transform      (DirectX::g_XMIdentityR3,    view_inv);
		XMVECTOR cam_dir = XMVector3TransformNormal(DirectX::g_XMNegIdentityR2, view_inv);
		XMStoreFloat3((XMFLOAT3*)&local.global_buffer.camera_pos[i], cam_pos);
		XMStoreFloat3((XMFLOAT3*)&local.global_buffer.camera_dir[i], cam_dir);

		local.global_buffer.view    [i] = XMMatrixTranspose(view_f);
		local.global_buffer.proj    [i] = XMMatrixTranspose(projection_f);
		local.global_buffer.proj_inv[i] = XMMatrixTranspose(proj_inv);
		local.global_buffer.viewproj[i] = XMMatrixTranspose(view_f * projection_f);
	}

	// Copy in the other global shader variables
	memcpy(local.global_buffer.lighting, local.lighting, sizeof(vec4) * 9);
	local.global_buffer.time       = time_totalf();
	local.global_buffer.view_count = view_count;
	local.global_buffer.eye_offset = eye_offset;
	for (int32_t i = 0; i < handed_max; i++) {
		const hand_t* hand = input_hand((handed_)i);
		vec3 tip = (hand->tracked_state & button_state_active) != 0 && input_get_finger_glow()
			? hand->fingers[1][4].position
			: vec3{ 0,-1000,0 };
		local.global_buffer.fingertip[i] = { tip.x, tip.y, tip.z, 0 };
	}

	// TODO: This is a little odd now that textures like this go through the
	// render_global_textures system.
	tex_t sky_tex = local.global_textures[render_skytex_register];
	local.global_buffer.cubemap_i = sky_tex != nullptr
		? vec4{ (float)sky_tex->width, (float)sky_tex->height, floorf(log2f((float)sky_tex->width)), 0 }
		: vec4{};

	// Upload shader globals
	material_buffer_set_data(local.shader_globals, &local.global_buffer);

	// Set global constant buffers for sk_renderer
	for (int32_t i = 0; i < _countof(local.global_buffers); i++) {
		if (local.global_buffers[i] != nullptr)
			skr_renderer_set_global_constants(i, &local.global_buffers[i]->buffer);
	}

	// Set global textures for sk_renderer
	for (int32_t i = 0; i < _countof(local.global_textures); i++) {
		if (local.global_textures[i] != nullptr) {
			skr_tex_t *tex = local.global_textures[i]->fallback == nullptr
				? &local.global_textures[i]->gpu_tex
				: &local.global_textures[i]->fallback->gpu_tex;
			skr_renderer_set_global_texture(i, tex);
		}
	}

	render_list_execute(list, filter, material_variant, inst_multiplier, 0, INT_MAX);
}

///////////////////////////////////////////

// The screenshots are produced in FIFO order, meaning the
// order of screenshot requests by users is preserved.
void render_check_screenshots() {
	if (local.screenshot_list.count == 0) return;

	for (int32_t i = 0; i < local.screenshot_list.count; i++) {
		int32_t  w = local.screenshot_list[i].width;
		int32_t  h = local.screenshot_list[i].height;

		// Create the screenshot surface
		size_t   size   = sizeof(color32) * w * h;
		color32 *buffer = (color32*)sk_malloc(size);

		// Create render targets for screenshot
		tex_t color_surface = tex_create_rendertarget(w, h, 8, local.screenshot_list[i].tex_format, tex_format_none);
		tex_t depth_surface = tex_create_rendertarget(w, h, 8, tex_format_depthstencil, tex_format_none);
		tex_t resolve_tex   = tex_create_rendertarget(w, h, 1, local.screenshot_list[i].tex_format, tex_format_none);

		// Set up viewport
		skr_rect_t  viewport = { 0, 0, (float)w, (float)h };
		skr_recti_t scissor  = { 0, 0, w, h };
		if (local.screenshot_list[i].viewport.w != 0) {
			viewport = {
				local.screenshot_list[i].viewport.x,
				local.screenshot_list[i].viewport.y,
				local.screenshot_list[i].viewport.w,
				local.screenshot_list[i].viewport.h };
			scissor = {
				(int32_t)local.screenshot_list[i].viewport.x,
				(int32_t)local.screenshot_list[i].viewport.y,
				(int32_t)local.screenshot_list[i].viewport.w,
				(int32_t)local.screenshot_list[i].viewport.h };
		}

		// Determine clear flags
		skr_clear_ clear_flags = skr_clear_none;
		if (local.screenshot_list[i].clear & render_clear_color) clear_flags = (skr_clear_)(clear_flags | skr_clear_color);
		if (local.screenshot_list[i].clear & render_clear_depth) clear_flags = (skr_clear_)(clear_flags | skr_clear_depth | skr_clear_stencil);

		// Begin render pass
		skr_vec4_t clear_color = { local.clear_col.r, local.clear_col.g, local.clear_col.b, local.clear_col.a };
		skr_renderer_begin_pass(&color_surface->gpu_tex, &depth_surface->gpu_tex, &resolve_tex->gpu_tex, clear_flags, clear_color, 1.0f, 0);
		skr_renderer_set_viewport(viewport);
		skr_renderer_set_scissor (scissor);

		// Render!
		render_draw_queue(local.list_primary, &local.screenshot_list[i].camera, &local.screenshot_list[i].projection, 0, 1, 1, local.screenshot_list[i].layer_filter, 0);

		// End render pass
		skr_renderer_end_pass();

		// Read back the resolved texture
		tex_get_data(resolve_tex, buffer, size);

		tex_release(resolve_tex);
		tex_release(depth_surface);
		tex_release(color_surface);

		// Notify that the color data is ready!
		local.screenshot_list[i].render_on_screenshot_callback(buffer, w, h, local.screenshot_list[i].context);
		sk_free(buffer);
	}
	local.screenshot_list.clear();
}

///////////////////////////////////////////

void render_draw_viewpoint(render_action_viewpoint_t* vp) {
	int32_t w = vp->rendertarget->width;
	int32_t h = vp->rendertarget->height;

	// Use the render target's depth buffer if it has one
	tex_t      depth_surface = vp->rendertarget->depth_buffer;
	skr_tex_t* depth_tex     = depth_surface ? &depth_surface->gpu_tex : nullptr;

	// Set up viewport
	skr_rect_t  viewport = { 0, 0, (float)w, (float)h };
	skr_recti_t scissor  = { 0, 0, w, h };
	if (vp->viewport.w != 0) {
		viewport = {
			vp->viewport.x * w,
			vp->viewport.y * h,
			vp->viewport.w * w,
			vp->viewport.h * h };
		scissor = {
			(int32_t)(vp->viewport.x * w),
			(int32_t)(vp->viewport.y * h),
			(int32_t)(vp->viewport.w * w),
			(int32_t)(vp->viewport.h * h) };
	}

	// Determine clear flags
	skr_clear_ clear_flags = skr_clear_none;
	if (vp->clear & render_clear_color) clear_flags = (skr_clear_)(clear_flags | skr_clear_color);
	if (vp->clear & render_clear_depth) clear_flags = (skr_clear_)(clear_flags | skr_clear_depth | skr_clear_stencil);

	// Begin render pass
	skr_vec4_t clear_color = { local.clear_col.r, local.clear_col.g, local.clear_col.b, local.clear_col.a };
	skr_renderer_begin_pass(&vp->rendertarget->gpu_tex, depth_tex, nullptr, clear_flags, clear_color, 1.0f, 0);
	skr_renderer_set_viewport(viewport);
	skr_renderer_set_scissor (scissor);

	// Render!
	render_draw_queue(local.list_primary, &vp->camera, &vp->projection, 0, 1, 1, vp->layer_filter, vp->material_variant);

	// End render pass
	skr_renderer_end_pass();

	// Release the reference we added, the user should have their own ref
	tex_release(vp->rendertarget);
}

///////////////////////////////////////////

void render_action_list_execute() {
	for (int32_t i = 0; i < local.render_action_list.count; i++) {
		render_action_t* a = &local.render_action_list[i];
		switch (a->type) {
		case render_action_type_viewpoint:      render_draw_viewpoint(&a->viewpoint); break;
		case render_action_type_global_buffer:  render_global_buffer_internal ( a->global_buffer .slot, a->global_buffer .buffer ); material_buffer_release(a->global_buffer .buffer ); break;
		case render_action_type_global_texture: render_global_texture_internal( a->global_texture.slot, a->global_texture.texture); tex_release            (a->global_texture.texture); break;
		default: break;
		}
	}
	local.render_action_list.clear();
}

///////////////////////////////////////////

void render_clear() {
	render_list_clear(local.list_primary);
}

///////////////////////////////////////////

void render_blit(tex_t to, material_t material) {
	material_check_dirty(material);

	skr_vec3i_t size = skr_tex_get_size(&to->gpu_tex);
	skr_recti_t bounds = { 0, 0, size.x, size.y };
	skr_renderer_blit(&material->gpu_mat, &to->gpu_tex, bounds);
}

///////////////////////////////////////////

struct screenshot_ctx_t {
	char*   filename;
	int32_t quality;
};

void render_save_to_file(color32* color_buffer, int width, int height, void* context) {
	screenshot_ctx_t *ctx = (screenshot_ctx_t*)context;
	if (string_endswith(ctx->filename, ".png", false)) {
		stbi_write_png(ctx->filename, width, height, 4, color_buffer, 0);
	} else {
		stbi_write_jpg(ctx->filename, width, height, 4, color_buffer, ctx->quality);
	}
	sk_free(ctx->filename);
	sk_free(ctx);
}

///////////////////////////////////////////

void render_screenshot(const char* file_utf8, int32_t file_quality_100, pose_t viewpoint, int32_t width, int32_t height, float fov_degrees) {
	screenshot_ctx_t *ctx = sk_malloc_t(screenshot_ctx_t, 1);
	ctx->filename = string_copy(file_utf8);
	ctx->quality  = file_quality_100;

	matrix view = pose_matrix_inv(viewpoint);
	matrix proj = matrix_perspective(fov_degrees, (float)width / height, local.clip_planes.x, local.clip_planes.y);
	local.screenshot_list.add(render_screenshot_t{ render_save_to_file, ctx, view, proj, rect_t{}, width, height, render_layer_all, render_clear_all, tex_format_rgba32 });
}

///////////////////////////////////////////

void render_screenshot_capture(void (*render_on_screenshot_callback)(color32* color_buffer, int32_t width, int32_t height, void* context), pose_t viewpoint, int32_t width, int32_t height, float fov_degrees, tex_format_ tex_format, void* context) {
	matrix view = pose_matrix_inv(viewpoint);
	matrix proj = matrix_perspective(fov_degrees, (float)width / height, local.clip_planes.x, local.clip_planes.y);
	local.screenshot_list.add(render_screenshot_t{ render_on_screenshot_callback, context, view, proj, rect_t{}, width, height, render_layer_all, render_clear_all, tex_format });
}

///////////////////////////////////////////

void render_screenshot_viewpoint(void (*render_on_screenshot_callback)(color32* color_buffer, int32_t width, int32_t height, void* context), matrix camera, matrix projection, int32_t width, int32_t height, render_layer_ layer_filter, render_clear_ clear, rect_t viewport, tex_format_ tex_format, void* context) {
	matrix inv_cam = matrix_invert(camera);
	local.screenshot_list.add(render_screenshot_t{ render_on_screenshot_callback, context, inv_cam, projection, viewport, width, height, layer_filter, clear, tex_format });
}

///////////////////////////////////////////

void render_to(tex_t to_rendertarget, int32_t to_target_index, const matrix& camera, const matrix& projection, render_layer_ layer_filter, int32_t material_variant, render_clear_ clear, rect_t viewport) {
	if (!(to_rendertarget->type & tex_type_rendertarget || to_rendertarget->type & tex_type_depthtarget || to_rendertarget->type & tex_type_zbuffer)) {
		log_err("render_to texture must be a render target texture type!");
		return;
	}
	tex_addref(to_rendertarget);

	matrix inv_cam;
	matrix_inverse(camera, inv_cam);
	render_action_t action = {};
	action.type = render_action_type_viewpoint;
	action.viewpoint.rendertarget      = to_rendertarget;
	action.viewpoint.rendertarget_index= to_target_index;
	action.viewpoint.camera            = inv_cam;
	action.viewpoint.projection        = projection;
	action.viewpoint.layer_filter      = layer_filter;
	action.viewpoint.viewport          = viewport;
	action.viewpoint.clear             = clear;
	action.viewpoint.material_variant  = material_variant;
	local.render_action_list.add(action);
}

///////////////////////////////////////////

vec3 render_unproject_pt(vec3 normalized_screen_pt) {
	XMMATRIX fast_proj, fast_view;
	math_matrix_to_fast(render_get_projection_matrix(), &fast_proj);
	math_matrix_to_fast(local.camera_root_final_inv,  &fast_view);
	XMVECTOR result = XMVector3Unproject(math_vec3_to_fast(normalized_screen_pt),
		0, 0, (float)sk_system_info().display_width, (float)sk_system_info().display_height,
		0, 1,
		fast_proj, fast_view, XMMatrixIdentity());
		
	return math_fast_to_vec3(result);
}

///////////////////////////////////////////

void render_get_device(void **device, void **context) {
	// sk_renderer uses Vulkan
	*device  = skr_get_vk_device();
	*context = skr_get_vk_instance();
}

///////////////////////////////////////////
// Render List                           //
///////////////////////////////////////////

render_list_t render_list_find(const char* id) {
	render_list_t result = (render_list_t)assets_find(id, asset_type_render_list);
	if (result != nullptr) {
		render_list_addref(result);
		return result;
	}
	return nullptr;
}

///////////////////////////////////////////

render_list_t render_list_create() {
	render_list_t result = (render_list_t)assets_allocate(asset_type_render_list);
	return result;
}

///////////////////////////////////////////

void render_list_set_id(render_list_t list, const char* id) {
	assets_set_id(&list->header, id);
}

///////////////////////////////////////////

const char* render_list_get_id(const render_list_t list) {
	return list->header.id_text;
}

///////////////////////////////////////////

void render_list_addref(render_list_t list) {
	assets_addref(&list->header);
}

///////////////////////////////////////////

void render_list_release(render_list_t list) {
	if (list == nullptr)
		return;
	assets_releaseref(&list->header);
}

///////////////////////////////////////////

void render_list_destroy(render_list_t list) {
	if (list == nullptr) return;
	render_list_clear(list);
	list->queue.free();
	*list = {};
}

///////////////////////////////////////////

void render_list_push(render_list_t list) {
	render_list_addref(list);
	local.list_stack.add(list);

	local.list_active = list;
}

///////////////////////////////////////////

void render_list_pop() {
	render_list_release(local.list_stack[local.list_stack.count-1]);
	local.list_stack.pop();
	local.list_active = local.list_stack.count > 0 
		? local.list_stack[local.list_stack.count-1]
		: nullptr;
}

///////////////////////////////////////////

void render_list_add(const render_item_t *item) {
	local.list_active->queue.add(*item);
	assets_addref(&item->material->header);
	assets_addref(&item->mesh->header);
}

///////////////////////////////////////////

void render_list_add_to(render_list_t list, const render_item_t *item) {
	list->queue.add(*item);
	assets_addref(&item->material->header);
	assets_addref(&item->mesh->header);
}

///////////////////////////////////////////

void render_list_execute(render_list_t list, render_layer_ filter, int32_t material_variant, uint32_t inst_multiplier, int32_t queue_start, int32_t queue_end) {
	list->state = render_list_state_rendering;

	if (list->queue.count == 0) {
		list->state = render_list_state_rendered;
		return;
	}

	// Make sure all materials are up-to-date
	material_t curr = nullptr;
	for (int32_t i = 0; i < list->queue.count; i++) {
		if (curr == list->queue[i].material) continue;
		curr = list->queue[i].material;

		material_t check = material_variant == 0
			? curr
			: curr->variants[material_variant - 1];
		if (check) material_check_dirty(check);
	}

	// Calculate sort_id range for queue filtering
	uint64_t sort_id_start = render_sort_id_from_queue(queue_start);
	uint64_t sort_id_end   = render_sort_id_from_queue(queue_end);

	// Clear and populate the sk_renderer render list
	skr_render_list_clear(&local.gpu_render_list);

	for (int32_t i = 0; i < list->queue.count; i++) {
		render_item_t *item = &list->queue[i];

		// Skip this item if it's filtered out
		if ((item->layer & filter) == 0) continue;
		if (item->sort_id < sort_id_start || item->sort_id >= sort_id_end) continue;

		// Pick the right material according to the variant
		material_t item_mat = material_variant == 0
			? item->material
			: item->material->variants[material_variant - 1];

		// Skip if the material variant is null
		if (item_mat == nullptr) continue;

		// Prepare instance data - transpose the matrix for GPU
		XMMATRIX transpose = XMMatrixTranspose(item->transform);
		render_transform_buffer_t inst = { transpose, item->color };

		// Add to sk_renderer's render list - it handles sorting and batching
		skr_render_list_add(&local.gpu_render_list,
			&item->mesh->gpu_mesh,
			&item_mat->gpu_mat,
			&inst, sizeof(inst), 1);
	}

	// Draw everything via sk_renderer
	skr_renderer_draw(&local.gpu_render_list,
		&local.global_buffer, sizeof(local.global_buffer),
		inst_multiplier);

	list->state = render_list_state_rendered;
}

///////////////////////////////////////////

void render_list_clear(render_list_t list) {
	list->prev_count = list->queue.count;
	for (int32_t i = 0; i < list->queue.count; i++) {
		assets_releaseref(&list->queue[i].material->header);
		assets_releaseref(&list->queue[i].mesh    ->header);
	}
	list->queue.clear();
	list->stats = {};
	list->state = render_list_state_empty;
}

///////////////////////////////////////////

int32_t render_list_item_count(render_list_t list) {
	return list->queue.count;
}

///////////////////////////////////////////

int32_t render_list_prev_count(render_list_t list) {
	return list->prev_count;
}

///////////////////////////////////////////

void render_list_add_mesh(render_list_t list, mesh_t mesh, material_t material, matrix transform, color128 color_linear, render_layer_ layer) {
	render_item_t item;
	item.mesh      = mesh;
	item.mesh_inds = mesh->ind_draw;
	item.color     = color_linear;
	item.layer     = (uint16_t)layer;
	if (hierarchy_use_top()) matrix_mul         (transform, hierarchy_top(), item.transform);
	else                     math_matrix_to_fast(transform, &item.transform);

	material_t curr = material;
	while (curr != nullptr) {
		item.material = curr;
		item.sort_id  = render_sort_id(curr, mesh);
		render_list_add_to(list, &item);
		curr = curr->chain;
	}
}

///////////////////////////////////////////

void render_list_add_model(render_list_t list, model_t model, matrix transform, color128 color_linear, render_layer_ layer) {
	render_list_add_model_mat(list, model, nullptr, transform, color_linear, layer);
}

///////////////////////////////////////////

void render_list_add_model_mat(render_list_t list, model_t model, material_t material_override, matrix transform, color128 color_linear, render_layer_ layer) {
	XMMATRIX root;
	if (hierarchy_use_top()) matrix_mul         (transform, hierarchy_top(), root);
	else                     math_matrix_to_fast(transform, &root);

	anim_update_model(model);
	for (int32_t i = 0; i < model->visuals.count; i++) {
		const model_visual_t *vis = &model->visuals[i];
		if (vis->visible == false || vis->mesh == nullptr || vis->material == nullptr) continue;
		
		render_item_t item;
		item.mesh      = vis->mesh;
		item.mesh_inds = vis->mesh->ind_count;
		item.color     = color_linear;
		item.layer     = (uint16_t)layer;
		matrix_mul(vis->transform_model, root, item.transform);

		material_t curr = material_override == nullptr ? vis->material : material_override;
		while (curr != nullptr) {
			item.material = curr;
			item.sort_id  = render_sort_id(curr, vis->mesh);
			render_list_add_to(list, &item);
			curr = curr->chain;
		}
	}

	if (model->transforms_changed && model->anim_data.skeletons.count > 0) {
		model->transforms_changed = false;
		anim_update_skin(model);
	}
}

///////////////////////////////////////////

void render_list_draw_now(render_list_t list, tex_t to_rendertarget, matrix camera, matrix projection, color128 clear_color, render_clear_ clear, rect_t viewport_pct, render_layer_ layer_filter, int32_t material_variant) {
	int32_t w = to_rendertarget->width;
	int32_t h = to_rendertarget->height;

	// Create a temporary depth buffer
	tex_t depth_surface = tex_create_rendertarget(w, h, 1, tex_format_depthstencil, tex_format_none);

	// Set up viewport
	if (viewport_pct.w == 0) viewport_pct.w = 1;
	if (viewport_pct.h == 0) viewport_pct.h = 1;

	skr_rect_t  viewport = {
		viewport_pct.x * w,
		viewport_pct.y * h,
		viewport_pct.w * w,
		viewport_pct.h * h };
	skr_recti_t scissor = {
		(int32_t)(viewport_pct.x * w),
		(int32_t)(viewport_pct.y * h),
		(int32_t)(viewport_pct.w * w),
		(int32_t)(viewport_pct.h * h) };

	// Determine clear flags
	skr_clear_ clear_flags = skr_clear_none;
	if (clear & render_clear_color) clear_flags = (skr_clear_)(clear_flags | skr_clear_color);
	if (clear & render_clear_depth) clear_flags = (skr_clear_)(clear_flags | skr_clear_depth | skr_clear_stencil);

	// Begin render pass
	skr_vec4_t skr_clear_color = { clear_color.r, clear_color.g, clear_color.b, clear_color.a };
	skr_renderer_begin_pass(&to_rendertarget->gpu_tex, &depth_surface->gpu_tex, nullptr, clear_flags, skr_clear_color, 1.0f, 0);
	skr_renderer_set_viewport(viewport);
	skr_renderer_set_scissor (scissor);

	// Render!
	render_draw_queue(list, &camera, &projection, 0, 1, 1, layer_filter, material_variant);

	// End render pass
	skr_renderer_end_pass();

	// Cleanup
	tex_release(depth_surface);
}

} // namespace sk
