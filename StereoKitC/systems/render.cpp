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
struct render_inst_buffer {
	int32_t      max;
	skg_buffer_t buffer;
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

typedef enum inst_pool_size_ {
	inst_pool_size_1,
	inst_pool_size_8,
	inst_pool_size_32,
	inst_pool_size_128,
	inst_pool_size_512,
	inst_pool_size_819,
	inst_pool_size_max,
} inst_pool_size_;
const int32_t inst_pool_sizes[] = { 1,8,32,128,512,819 };

struct render_state_t {
	bool32_t                initialized;
	skr_vert_type_t         default_vert_type;

	array_t<render_transform_buffer_t> instance_list;
	array_t<skg_buffer_t>              instance_pool     [inst_pool_size_max];
	int32_t                            instance_pool_used[inst_pool_size_max];

	material_buffer_t       shader_globals;
	skg_buffer_t            shader_blit;
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

	material_t              last_material;
	shader_t                last_shader;
	mesh_t                  last_mesh;

	array_t<render_list_t>  list_stack;
	render_list_t           list_active;
};
static render_state_t local = {};

const int32_t    render_instance_max     = 819;
const int32_t    render_skytex_register  = 11;
const skg_bind_t render_list_global_bind = { 1,  skg_stage_vertex | skg_stage_pixel, skg_register_constant };
const skg_bind_t render_list_inst_bind   = { 2,  skg_stage_vertex | skg_stage_pixel, skg_register_constant };
const skg_bind_t render_list_blit_bind   = { 3,  skg_stage_vertex | skg_stage_pixel, skg_register_constant };

///////////////////////////////////////////

void          render_set_material     (material_t material);
skg_buffer_t *render_fill_inst_buffer (const array_t<render_transform_buffer_t>* list, int32_t* ref_offset, int32_t* out_count);
void          render_reset_buffer_pool();
void          render_save_to_file     (color32* color_buffer, int width, int height, void* context);

void          render_list_prep        (render_list_t list, int32_t material_variant);
void          render_list_add         (const render_item_t *item);
void          render_list_add_to      (render_list_t list, const render_item_t *item);

void          radix_sort7             (render_item_t *a, size_t count);
void          radix_sort_clean        ();
void          radix_sort_init         ();

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
	local.shader_blit     = skg_buffer_create(nullptr, 1, sizeof(render_blit_data_t), skg_buffer_type_constant, skg_use_dynamic);
#if !defined(SKG_OPENGL) && (defined(_DEBUG) || defined(SK_GPU_LABELS))
	skg_buffer_name(&local.shader_blit, "sk/render/blit_buffer");
#endif

	render_global_buffer_internal(1, local.shader_globals);
	
	local.instance_list.resize(render_instance_max);

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

	radix_sort_init();
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
	local.instance_list     .free();

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

	for (int32_t pool_idx = 0; pool_idx < inst_pool_size_max; pool_idx++) {
		array_t<skg_buffer_t>* pool = &local.instance_pool[pool_idx];
		for (int32_t i = 0; i < pool->count; i++) {
			skg_buffer_destroy(&pool->get(i));
		}
		pool->free();
	}

	skg_buffer_destroy(&local.shader_blit);

	// Destroy the default vertex type
	skr_vert_type_destroy(&local.default_vert_type);

	local = {};

	radix_sort_clean();
	hierarchy_shutdown();
}

///////////////////////////////////////////

const skr_vert_type_t* render_get_default_vert() {
	return &local.default_vert_type;
}

///////////////////////////////////////////

void render_step() {
	profiler_zone();

	render_reset_buffer_pool();

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
	skg_event_begin("Render List Setup");

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

	// Upload shader globals and set them active!
	material_buffer_set_data(local.shader_globals, &local.global_buffer);

	// Activate any material buffers we have
	for (int32_t i = 0; i < _countof(local.global_buffers); i++) {
		if (local.global_buffers[i] != nullptr)
			skg_buffer_bind(&local.global_buffers[i]->buffer, { (uint16_t)i,  skg_stage_vertex | skg_stage_pixel, skg_register_constant });
	}

	// Activate any global textures we have
	for (int32_t i = 0; i < _countof(local.global_textures); i++) {
		if (local.global_textures[i] != nullptr) {
			skg_tex_t *tex = local.global_textures[i]->fallback == nullptr
				? &local.global_textures[i]->tex
				: &local.global_textures[i]->fallback->tex;
			skg_tex_bind(tex, { (uint16_t)i,  skg_stage_vertex | skg_stage_pixel, skg_register_resource });
		}
	}

	skg_event_end();
	skg_event_begin("Execute Render List");

	render_list_execute(list, filter, material_variant, inst_multiplier, 0, INT_MAX);

	skg_event_end();
}

///////////////////////////////////////////

// The screenshots are produced in FIFO order, meaning the
// order of screenshot requests by users is preserved.
void render_check_screenshots() {
	if (local.screenshot_list.count == 0) return;

	skg_tex_t *old_target = skg_tex_target_get();
	for (int32_t i = 0; i < local.screenshot_list.count; i++) {
		skg_event_begin("Screenshot");
		int32_t  w = local.screenshot_list[i].width;
		int32_t  h = local.screenshot_list[i].height;

		// Create the screenshot surface
		size_t   size   = sizeof(color32) * w * h;
		color32 *buffer = (color32*)sk_malloc(size);
		
		// Setup to render the screenshot
		tex_t render_capture_surface = tex_create_rendertarget(w, h, 8, local.screenshot_list[i].tex_format, tex_format_depthstencil);
		skg_tex_target_bind(&render_capture_surface->tex, -1, 0);

		// Set up the viewport if we've got one!
		if (local.screenshot_list[i].viewport.w != 0) {
			int32_t viewport[4] =
			{
				(int32_t)(local.screenshot_list[i].viewport.x),
				(int32_t)(local.screenshot_list[i].viewport.y),
				(int32_t)(local.screenshot_list[i].viewport.w),
				(int32_t)(local.screenshot_list[i].viewport.h)
			};
			skg_viewport(viewport);
		} else {
			int32_t viewport[4] = { 0,0,w,h };
			skg_viewport(viewport);
		}

		// Clear the viewport
		if (local.screenshot_list[i].clear != render_clear_none) {
			skg_target_clear(
				(local.screenshot_list[i].clear & render_clear_depth),
				(local.screenshot_list[i].clear & render_clear_color) ? &local.clear_col.r : (float*)nullptr);
		}

		// Render!
		render_draw_queue(local.list_primary, &local.screenshot_list[i].camera, &local.screenshot_list[i].projection, 0, 1, 1, local.screenshot_list[i].layer_filter, 0);
		skg_tex_target_bind(nullptr, -1, 0);

		tex_t resolve_tex = tex_create_rendertarget(w, h, 1, local.screenshot_list[i].tex_format, tex_format_none);
		skg_tex_copy_to(&render_capture_surface->tex, -1, &resolve_tex->tex, -1);
		tex_get_data(resolve_tex, buffer, size);
#if defined(SKG_OPENGL)
		int32_t line_size = skg_tex_fmt_pitch(resolve_tex->tex.format, resolve_tex->tex.width);
		void* tmp = sk_malloc(line_size);
		for (int32_t y = 0; y < resolve_tex->tex.height / 2; y++) {
			void* top_line = ((uint8_t*)buffer) + line_size * y;
			void* bot_line = ((uint8_t*)buffer) + line_size * ((resolve_tex->tex.height - 1) - y);
			memcpy(tmp,      top_line, line_size);
			memcpy(top_line, bot_line, line_size);
			memcpy(bot_line, tmp,      line_size);
		}
		sk_free(tmp);
#endif
		tex_release(resolve_tex);
		tex_release(render_capture_surface);

		// Notify that the color data is ready!
		local.screenshot_list[i].render_on_screenshot_callback(buffer, w, h, local.screenshot_list[i].context);
		sk_free(buffer);
		skg_event_end();
	}
	local.screenshot_list.clear();
	skg_tex_target_bind(old_target, -1, 0);
}

///////////////////////////////////////////

void render_draw_viewpoint(render_action_viewpoint_t* vp) {
	skg_event_begin("Viewpoint");
	// Setup to render the screenshot
	skg_tex_target_bind(&vp->rendertarget->tex, vp->rendertarget_index, 0);

	// Clear the viewport
	if (vp->clear != render_clear_none) {
		skg_target_clear(
			(vp->clear & render_clear_depth),
			(vp->clear & render_clear_color) ? &local.clear_col.r : (float *)nullptr);
	}

	// Set up the viewport if we've got one!
	if (vp->viewport.w != 0) {
		int32_t viewport[4] = {
			(int32_t)(vp->viewport.x * vp->rendertarget->width ),
			(int32_t)(vp->viewport.y * vp->rendertarget->height),
			(int32_t)(vp->viewport.w * vp->rendertarget->width ),
			(int32_t)(vp->viewport.h * vp->rendertarget->height) };
		skg_viewport(viewport);
	}

	// Render!
	render_draw_queue(local.list_primary, &vp->camera, &vp->projection, 0, 1, 1, vp->layer_filter, vp->material_variant);
	skg_tex_target_bind(nullptr, -1, 0);

	// Release the reference we added, the user should have their own ref
	tex_release(vp->rendertarget);
	skg_event_end();
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
	//log_infof("draws: %d, instances: %d, material: %d, shader: %d, texture %d, mesh %d", render_stats.draw_calls, render_stats.draw_instances, render_stats.swaps_material, render_stats.swaps_shader, render_stats.swaps_texture, render_stats.swaps_mesh);
	render_list_clear(local.list_primary);

	local.last_material = nullptr;
	local.last_shader   = nullptr;
	local.last_mesh     = nullptr;
}

///////////////////////////////////////////

void render_blit_to_bound(material_t material) {
	material_check_dirty(material);

	// Wipe our swapchain color and depth target clean, and then set them up for rendering!
	float color[4] = { 0,0,0,0 };
	skg_target_clear(true, color);

	skg_tex_t *target = skg_tex_target_get();

	// Setup shader args for the blit operation
	render_blit_data_t data = {};
	data.width  = (float)target->width;
	data.height = (float)target->height;
	data.pixel_width  = 1.0f / target->width;
	data.pixel_height = 1.0f / target->height;

	// Setup render states for blitting
	skg_buffer_set_contents(&local.shader_blit, &data, sizeof(render_blit_data_t));
	skg_buffer_bind        (&local.shader_blit, render_list_blit_bind);
	render_set_material(material);
	skg_mesh_bind(&local.blit_quad->gpu_mesh);
	
	// And draw to it!
	skg_draw(0, 0, local.blit_quad->ind_count, 1);

	local.last_material = nullptr;
	local.last_mesh     = nullptr;
	local.last_shader   = nullptr;
}

///////////////////////////////////////////

void render_blit(tex_t to, material_t material) {
	skg_tex_t *old_target = skg_tex_target_get();

	for (int32_t i = 0; i < to->tex.array_count; i++)
	{
		skg_tex_target_bind(&to->tex, i, 0);
		render_blit_to_bound(material);
	}
	skg_tex_target_bind(old_target, -1, 0);
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

void render_set_material(material_t material) {
	if (material == local.last_material)
		return;
	local.last_material = material;
	local.list_active->stats.swaps_material++;

	// Update and bind the material parameter buffer
	if (material->args.buffer != nullptr) {
		skg_buffer_bind(&material->args.buffer_gpu, material->args.buffer_bind);
	}

	// Bind the material textures
	for (int32_t i = 0; i < material->args.texture_count; i++) {
		if (local.global_textures[material->args.textures[i].bind.slot] == nullptr) {
			tex_t tex = material->args.textures[i].tex;
			if (tex->fallback != nullptr)
				tex = tex->fallback;
			skg_tex_bind(&tex->tex, material->args.textures[i].bind);
		}
	}

	// And bind the pipeline
	skg_pipeline_bind(&material->pipeline);
}

///////////////////////////////////////////

inst_pool_size_ inst_pool_from_size(int32_t size) {
	if      (size <= inst_pool_sizes[inst_pool_size_1  ]) return inst_pool_size_1;
	else if (size <= inst_pool_sizes[inst_pool_size_8  ]) return inst_pool_size_8;
	else if (size <= inst_pool_sizes[inst_pool_size_32 ]) return inst_pool_size_32;
	else if (size <= inst_pool_sizes[inst_pool_size_128]) return inst_pool_size_128;
	else if (size <= inst_pool_sizes[inst_pool_size_512]) return inst_pool_size_512;
	else if (size <= inst_pool_sizes[inst_pool_size_819]) return inst_pool_size_819;
	else { log_err("Bad pool size"); return inst_pool_size_1; }
}

///////////////////////////////////////////

void render_reset_buffer_pool() {
	for (int32_t i = 0; i < inst_pool_size_max; i++) {
		int32_t buffer_id = local.instance_pool_used[i] = 0;
	}
}

///////////////////////////////////////////

skg_buffer_t *render_fill_inst_buffer(const array_t<render_transform_buffer_t>* list, int32_t* ref_offset, int32_t* out_count) {
	// Find a buffer that can contain this list! Or the biggest one
	int32_t size  = list->count - *ref_offset;
	int32_t start = *ref_offset;

	// Check if it fits, if it doesn't, then set up data so we only fill what we have! 
	if (size > render_instance_max) {
		*ref_offset += render_instance_max;
		*out_count   = render_instance_max;
	} else {
		// this means we've gotten through the whole list :)
		*ref_offset = 0;
		*out_count  = size;
	}

	// Get the appropriate pool for this size
	inst_pool_size_        pool_idx  = inst_pool_from_size(*out_count);
	array_t<skg_buffer_t>* pool      = &local.instance_pool     [pool_idx];
	int32_t*               pool_used = &local.instance_pool_used[pool_idx];
	// If our pool for this size is empty, add a new buffer
	if (*pool_used >= pool->count) {
		skg_buffer_t new_buffer = skg_buffer_create(nullptr, inst_pool_sizes[pool_idx], sizeof(render_transform_buffer_t), skg_buffer_type_constant, skg_use_dynamic);
#if !defined(SKG_OPENGL) && (defined(_DEBUG) || defined(SK_GPU_LABELS))
		char name[64];
		snprintf(name, sizeof(name), "sk/render/instance_pool_%d_%d", inst_pool_sizes[pool_idx], *pool_used);
		skg_buffer_name(&new_buffer, name);
#endif
		pool->add(new_buffer);
	}

	// Fetch the buffer at the top of the pool
	skg_buffer_t* buffer = &pool->get(*pool_used);
	*pool_used += 1;

	// Copy data into the buffer, and return it!
	skg_buffer_set_contents(buffer, &list->data[start], sizeof(render_transform_buffer_t) * *out_count);
	return buffer;
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
	skg_platform_data_t platform = skg_get_platform_data();
#if defined(SKG_DIRECT3D11)
	*device  = platform._d3d11_device;
	//*context = platform._d3d11_context;
	*context = nullptr;
#elif defined(_SKG_GL_LOAD_EGL)
	*device  = platform._egl_display;
	*context = platform._egl_context;
#elif defined(_SKG_GL_LOAD_WGL)
	*device  = platform._gl_hdc;
	*context = platform._gl_hrc;
#elif defined(_SKG_GL_LOAD_GLX)
	*device  = platform._glx_drawable;
	*context = platform._glx_context;
#else
	log_warn("render_get_device not implemented for sk_gpu!");
#endif
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

inline void render_list_execute_run(_render_list_t *list, material_t material, const skg_mesh_t *mesh, int32_t mesh_inds, uint32_t inst_multiplier) {
	if (mesh_inds == 0) return;

	render_set_material(material);
	skg_mesh_bind      (mesh);
	list->stats.swaps_mesh++;

	// Collect and draw instances
	int32_t offsets = 0, inst_count = 0;
	do {
		skg_buffer_t *instances = render_fill_inst_buffer(&local.instance_list, &offsets, &inst_count);
		skg_buffer_bind(instances, render_list_inst_bind);

		skg_draw(0, 0, mesh_inds, inst_count * inst_multiplier);
		list->stats.draw_calls     += 1;
		list->stats.draw_instances += inst_count;

	} while (offsets != 0);
}

///////////////////////////////////////////

void render_list_execute(render_list_t list, render_layer_ filter, int32_t material_variant, uint32_t inst_multiplier, int32_t queue_start, int32_t queue_end) {
	list->state = render_list_state_rendering;

	if (list->queue.count == 0) {
		list->state = render_list_state_rendered;
		return;
	}
	render_list_prep(list, material_variant);
	uint64_t sort_id_start = render_sort_id_from_queue(queue_start);
	uint64_t sort_id_end   = render_sort_id_from_queue(queue_end);

	render_item_t *run_start    = nullptr;
	material_t     run_material = nullptr;
	for (int32_t i = 0; i < list->queue.count; i++) {
		render_item_t *item = &list->queue[i];
		
		// Skip this item if it's filtered out
		if ((item->layer & filter) == 0 || item->sort_id < sort_id_start) continue;
		// End early if we're past the end of the desired queue range
		if (item->sort_id >= sort_id_end) break;

		// Pick the right material according to the variant
		material_t item_mat = material_variant == 0
			? item->material
			: item->material->variants[material_variant - 1];

		// Skip if the material variant is null
		if (item_mat == nullptr) continue;

		// If it's the first in the run, record the material/mesh
		if (run_start == nullptr) {
			run_start    = item;
			run_material = item_mat;
		}
		// If the material/mesh changed
		else if (run_material != item_mat || run_start->mesh != item->mesh) {
			// Render the run that just ended
			render_list_execute_run(list, run_material, &run_start->mesh->gpu_mesh, run_start->mesh_inds, inst_multiplier);
			local.instance_list.clear();
			// Start the next run
			run_start    = item;
			run_material = item_mat;
		}

		// Add the current item to the run of instances
		XMMATRIX transpose = XMMatrixTranspose(item->transform);
		local.instance_list.add(render_transform_buffer_t{ transpose, item->color });
	}
	// Render the last remaining run, which won't be triggered by the loop's
	// conditions
	if (local.instance_list.count > 0) {
		render_list_execute_run(list, run_material, &run_start->mesh->gpu_mesh, run_start->mesh_inds, inst_multiplier);
		local.instance_list.clear();
	}

	list->state = render_list_state_rendered;

	local.last_material = nullptr;
	local.last_shader   = nullptr;
	local.last_mesh     = nullptr;
}

///////////////////////////////////////////

void render_list_prep(render_list_t list, int32_t material_variant) {
	// Sort the render queue
	radix_sort7(&list->queue[0], list->queue.count);

	// Make sure the material buffers are all up-to-date
	material_t curr = nullptr;
	for (int32_t i = 0; i < list->queue.count; i++) {
		if (curr == list->queue[i].material) continue;
		curr = list->queue[i].material;

		material_t check = material_variant == 0
			? curr
			: curr->variants[material_variant - 1];
		if (check) material_check_dirty(check);
	}
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
	skg_tex_t* old_target = skg_tex_target_get();
	skg_tex_target_bind(&to_rendertarget->tex, -1, 0);

	if (clear != render_clear_none) {
		skg_target_clear(
			(clear & render_clear_depth),
			(clear & render_clear_color) ? &clear_color.r : (float*)nullptr);
	}

	if (viewport_pct.w == 0) viewport_pct.w = 1;
	if (viewport_pct.h == 0) viewport_pct.h = 1;

	int32_t viewport_i[4] = {
		(int32_t)(viewport_pct.x * to_rendertarget->width ),
		(int32_t)(viewport_pct.y * to_rendertarget->height),
		(int32_t)(viewport_pct.w * to_rendertarget->width ),
		(int32_t)(viewport_pct.h * to_rendertarget->height) };
	skg_viewport(viewport_i);

	render_draw_queue(list, &camera, &projection, 0, 1, 1, layer_filter, material_variant);

	skg_tex_target_bind(old_target, -1, 0);
}

///////////////////////////////////////////
// Radix render sorting!                 //
///////////////////////////////////////////

// https://travisdowns.github.io/blog/2019/05/22/sorting.html

#if _MSC_VER
#include <intrin.h>
#endif

const size_t   RADIX_BITS   = 8;
const size_t   RADIX_SIZE   = (size_t)1 << RADIX_BITS;
const size_t   RADIX_LEVELS = (63 / RADIX_BITS) + 1;
const uint64_t RADIX_MASK   = RADIX_SIZE - 1;

using freq_array_type = size_t [RADIX_LEVELS][RADIX_SIZE];

// Since this sort is specifically for the render queue, we'll reserve a
// chunk of memory that sticks around, and resizes if it's too small.
render_item_t *radix_queue_area = nullptr;
size_t         radix_queue_size = 0;

void radix_sort_init() {
	radix_queue_area = nullptr;
	radix_queue_size = 0;
}

void radix_sort_clean() {
	sk_free(radix_queue_area);
	radix_queue_area = nullptr;
	radix_queue_size = 0;
}

// never inline just to make it show up easily in profiles (inlining this lengthly function doesn't
// really help anyways)
static void count_frequency(render_item_t *a, size_t count, freq_array_type freqs) {
	for (size_t i = 0; i < count; i++) {
		uint64_t value = a[i].sort_id;
		for (size_t pass = 0; pass < RADIX_LEVELS; pass++) {
			freqs[pass][value & RADIX_MASK]++;
			value >>= RADIX_BITS;
		}
	}
}

/**
* Determine if the frequencies for a given level are "trivial".
* 
* Frequencies are trivial if only a single frequency has non-zero
* occurrences. In that case, the radix step just acts as a copy so we can
* skip it.
*/
static bool is_trivial(size_t freqs[RADIX_SIZE], size_t count) {
	for (size_t i = 0; i < RADIX_SIZE; i++) {
		size_t freq = freqs[i];
		if (freq != 0) {
			return freq == count;
		}
	}
	assert(count == 0); // we only get here if count was zero
	return true;
}

void radix_sort7(render_item_t *a, size_t count) {
	// Resize up if needed
	if (radix_queue_size < count) {
		sk_free(radix_queue_area);
		radix_queue_area = sk_malloc_t(render_item_t, count);
		radix_queue_size = count;
	}
	freq_array_type freqs = {};
	count_frequency(a, count, freqs);

	render_item_t *from = a, *to = radix_queue_area;

	for (size_t pass = 0; pass < RADIX_LEVELS; pass++) {

		if (is_trivial(freqs[pass], count)) {
			// this pass would do nothing, just skip it
			continue;
		}

		uint64_t shift = pass * RADIX_BITS;

		// array of pointers to the current position in each queue, which we set up based on the
		// known final sizes of each queue (i.e., "tighly packed")
		render_item_t *queue_ptrs[RADIX_SIZE], *next = to;
		for (size_t i = 0; i < RADIX_SIZE; i++) {
			queue_ptrs[i] = next;
			next += freqs[pass][i];
		}

		// copy each element into the appropriate queue based on the current RADIX_BITS sized
		// "digit" within it
		for (size_t i = 0; i < count; i++) {
			render_item_t value = from[i];
			size_t        index = (value.sort_id >> shift) & RADIX_MASK;
			*queue_ptrs[index]++ = value;
#ifdef _MSC_VER
	#if defined(_M_ARM) || defined(_M_ARM64)
			__prefetch (queue_ptrs[index] + 1);
	#else
			_m_prefetch(queue_ptrs[index] + 1);
	#endif
#else
			__builtin_prefetch(queue_ptrs[index] + 1);
#endif
		}

		// swap from and to areas
		render_item_t *tmp = to;
		to   = from;
		from = tmp;
	}

	// because of the last swap, the "from" area has the sorted payload: if it's
	// not the original array "a", do a final copy
	if (from != a) {
		memcpy(a, from, count*sizeof(render_item_t));
	}
}

} // namespace sk
