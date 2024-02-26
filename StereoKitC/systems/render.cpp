#include "render.h"
#include "world.h"
#include "defaults.h"
#include "../_stereokit.h"
#include "../device.h"
#include "../libraries/sk_gpu.h"
#include "../libraries/stref.h"
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
#include "../platforms/platform.h"

#include <limits.h>

#pragma warning(push)
#pragma warning(disable : 26451 26819 6386 6385 )
#if defined(_WIN32)
#define __STDC_LIB_EXT1__
#endif
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#define STBIW_WINDOWS_UTF8
#include "../libraries/stb_image_write.h"
#pragma warning(pop)

using namespace DirectX;

namespace sk {

///////////////////////////////////////////

struct render_item_t {
	XMMATRIX    transform;
	color128    color;
	uint64_t    sort_id;
	mesh_t      mesh;
	material_t  material;
	int32_t     mesh_inds;
	uint16_t    layer;
};

struct _render_list_t {
	array_t<render_item_t> queue;
	render_stats_t         stats;
	render_list_state_     state;
	bool                   prepped;
};

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
	tex_format_	  tex_format;
};
struct render_viewpoint_t {
	tex_t         rendertarget;
	matrix        camera;
	matrix        projection;
	rect_t        viewport;
	material_t    override_material;
	render_layer_ layer_filter;
	render_clear_ clear;
};

///////////////////////////////////////////

struct render_state_t {
	bool32_t                initialized;

	array_t<render_transform_buffer_t> instance_list;
	skg_buffer_t                       instance_buffer;

	material_buffer_t       shader_globals;
	skg_buffer_t            shader_blit;
	matrix                  camera_root;
	matrix                  camera_root_final;
	matrix                  camera_root_final_inv;
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
	float                   scale;
	int32_t                 multisample;
	render_layer_           primary_filter;
	render_layer_           capture_filter;
	bool                    use_capture_filter;
	tex_t                   global_textures[16];

	array_t<render_screenshot_t> screenshot_list;
	array_t<render_viewpoint_t>  viewpoint_list;

	mesh_t                  sky_mesh;
	material_t              sky_mat;
	material_t              sky_mat_default;
	bool32_t                sky_show;

	material_t              last_material;
	shader_t                last_shader;
	mesh_t                  last_mesh;

	array_t< render_list_t> list_stack;
	array_t<_render_list_t> lists;
	render_list_t           list_active;

};
static render_state_t local = {};

const int32_t    render_instance_max     = 819;
const int32_t    render_skytex_register  = 11;
const skg_bind_t render_list_global_bind = { 1,  skg_stage_vertex | skg_stage_pixel, skg_register_constant };
const skg_bind_t render_list_inst_bind   = { 2,  skg_stage_vertex | skg_stage_pixel, skg_register_constant };
const skg_bind_t render_list_blit_bind   = { 2,  skg_stage_vertex | skg_stage_pixel, skg_register_constant };

///////////////////////////////////////////

void          render_set_material     (material_t material);
skg_buffer_t *render_fill_inst_buffer (array_t<render_transform_buffer_t> &list, int32_t &offset, int32_t &out_count);
void          render_save_to_file     (color32* color_buffer, int width, int height, void* context);

void          render_list_prep        (render_list_t list);
void          render_list_add         (const render_item_t *item);
void          render_list_add_to      (render_list_t list, const render_item_t *item);

void          radix_sort7             (render_item_t *a, size_t count);
void          radix_sort_clean        ();
void          radix_sort_init         ();

///////////////////////////////////////////

bool render_init() {
	local = {};
	local.initialized           = true;
	local.camera_root           = matrix_identity;
	local.camera_root_final     = matrix_identity;
	local.camera_root_final_inv = matrix_identity;
	local.sim_origin            = matrix_identity;
	local.sim_head              = matrix_identity;
	local.clip_planes           = {0.02f, 50};
	local.fov                   = 90;
	local.ortho_near_clip       = 0.0f;
	local.ortho_far_clip        = 50.0f;
	local.ortho_viewport_height = 1.0f;
	local.clear_col             = color128{0,0,0,0};
	local.list_primary          = -1;
	local.scale                 = 1;
	local.multisample           = 1;
	local.primary_filter        = render_layer_all_first_person;
	local.capture_filter        = render_layer_all_first_person;
	local.list_active           = -1;

	local.shader_globals  = material_buffer_create(1, sizeof(local.global_buffer));
	local.shader_blit     = skg_buffer_create(nullptr, 1, sizeof(render_blit_data_t), skg_buffer_type_constant, skg_use_dynamic);
#if !defined(SKG_OPENGL) && (defined(_DEBUG) || defined(SK_GPU_LABELS))
	skg_buffer_name(&local.shader_blit, "sk/render/blit_buffer");
#endif
	
	local.instance_buffer = skg_buffer_create(nullptr, render_instance_max, sizeof(render_transform_buffer_t), skg_buffer_type_constant, skg_use_dynamic);
#if !defined(SKG_OPENGL) && (defined(_DEBUG) || defined(SK_GPU_LABELS))
	skg_buffer_name(&local.instance_buffer, "sk/render/instance_buffer");
#endif
	local.instance_list.resize(render_instance_max);

	// Setup a default camera
	render_set_clip(local.clip_planes.x, local.clip_planes.y);

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
	render_list_push(local.list_primary);

	radix_sort_init();
	hierarchy_init();

	render_update_projection();

	return true;
}

///////////////////////////////////////////

void render_shutdown() {
	for (int32_t i = 0; i < local.lists.count; i++) {
		render_list_release(i);
	}
	local.lists          .free();
	local.list_stack     .free();
	local.screenshot_list.free();
	local.viewpoint_list .free();
	local.instance_list  .free();

	for (int32_t i = 0; i < _countof(local.global_textures); i++) {
		tex_release(local.global_textures[i]);
		local.global_textures[i] = nullptr;
	}
	material_release       (local.sky_mat_default);
	material_release       (local.sky_mat);
	mesh_release           (local.sky_mesh);
	mesh_release           (local.blit_quad);
	material_buffer_release(local.shader_globals);

	skg_buffer_destroy(&local.instance_buffer);
	skg_buffer_destroy(&local.shader_blit);

	local = {};

	radix_sort_clean();
	hierarchy_shutdown();
}

///////////////////////////////////////////

void render_step() {
	hierarchy_step();

	if (local.sky_show && device_display_get_blend() == display_blend_opaque) {
		render_add_mesh(local.sky_mesh, local.sky_mat, matrix_identity, {1,1,1,1}, render_layer_vfx);
	}
}

///////////////////////////////////////////

inline uint64_t render_sort_id(material_t material, mesh_t mesh) {
	// Top 32 bits for developer's desired queue offset + which queue it's in, next 16 bits for material asset index, next 16 bits for mesh asset index

	// We determine "which queue" we're in based on the material's transparency mode. 
	// Opaque materials are rendered first, then alpha blend materials, then additive materials.

	// You want to render opaque materials first, because alpha blended materials depend on the pixels _underneath_ them already being non-empty.
	// Additive materials coming after that is somewhat arbitrary.

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

void render_set_fov(float field_of_view_degrees) {
	local.fov = field_of_view_degrees;
	render_update_projection();
}

///////////////////////////////////////////

void render_set_ortho_size(float viewport_height_meters) {
	local.ortho_viewport_height = viewport_height_meters;
	render_update_projection();
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
	case depth_mode_balanced:
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

render_list_t render_get_primary_list() {
	return local.list_primary;
}

///////////////////////////////////////////

void render_set_cam_root(const matrix &cam_root) {
	local.camera_root       = cam_root;
	local.camera_root_final = local.sim_head * cam_root * local.sim_origin;
	matrix_inverse(local.camera_root_final, local.camera_root_final_inv);

	// TODO: May want to also update controllers/hands?
	quat rot = matrix_extract_rotation(local.camera_root_final);
	input_head_pose_world.position    = local.camera_root_final * input_head_pose_local.position;
	input_head_pose_world.orientation = rot * input_head_pose_local.orientation;
	input_eyes_pose_world.position    = local.camera_root_final * input_eyes_pose_local.position;
	input_eyes_pose_world.orientation = rot * input_eyes_pose_local.orientation;

	world_refresh_transforms();
	input_update_poses(false);
}

///////////////////////////////////////////

void render_set_sim_origin(pose_t pose) {
	local.sim_origin = matrix_invert(pose_matrix(pose));
	render_set_cam_root(render_get_cam_root());
}

///////////////////////////////////////////

void render_set_sim_head(pose_t pose) {
	local.sim_head = pose_matrix(pose);
	render_set_cam_root(render_get_cam_root());
}

///////////////////////////////////////////

void render_set_skytex(tex_t sky_texture) {
	if (sky_texture != nullptr && !(sky_texture->type & tex_type_cubemap)) {
		log_err("render_set_skytex: Attempting to set the skybox texture to a texture that's not a cubemap! Sorry, but cubemaps only here please!");
		return;
	}

	if (sky_texture != nullptr && local.global_textures[render_skytex_register] != nullptr) {
		tex_set_fallback(sky_texture, local.global_textures[render_skytex_register]);
	}
	render_global_texture(render_skytex_register, sky_texture);
}

///////////////////////////////////////////

tex_t render_get_skytex() {
	if (local.global_textures[render_skytex_register] != nullptr)
		tex_addref(local.global_textures[render_skytex_register]);
	return local.global_textures[render_skytex_register];
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

void render_global_texture(int32_t register_slot, tex_t texture) {
	if (register_slot < 0 || register_slot >= _countof(local.global_textures)) {
		log_errf("render_global_texture: Register_slot should be 0-16. Received %d.", register_slot);
		return;
	}

	if (local.global_textures[register_slot] == texture) return;

	if (local.global_textures[register_slot] != nullptr)
		tex_release(local.global_textures[register_slot]);

	local.global_textures[register_slot] = texture;

	if (local.global_textures[register_slot] != nullptr)
		tex_addref(local.global_textures[register_slot]);
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
	render_item_t item;
	item.mesh      = mesh;
	item.mesh_inds = mesh->ind_draw;
	item.color     = color_linear;
	item.layer     = (uint16_t)layer;
	if (hierarchy_use_top()) {
		matrix_mul(transform, hierarchy_top(), item.transform);
	} else {
		math_matrix_to_fast(transform, &item.transform);
	}

	material_t curr = material;
	while (curr != nullptr) {
		item.material = curr;
		item.sort_id  = render_sort_id(curr, mesh);
		render_list_add(&item);
		curr = curr->chain;
	}
}

///////////////////////////////////////////

void render_add_model_mat(model_t model, material_t material_override, const matrix& transform, color128 color_linear, render_layer_ layer) {
	XMMATRIX root;
	if (hierarchy_use_top()) {
		matrix_mul(transform, hierarchy_top(), root);
	} else {
		math_matrix_to_fast(transform, &root);
	}

	anim_update_model(model);
	for (int32_t i = 0; i < model->visuals.count; i++) {
		const model_visual_t *vis = &model->visuals[i];
		if (vis->visible == false) continue;
		
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
			render_list_add(&item);
			curr = curr->chain;
		}
	}

	if (model->transforms_changed && model->anim_data.skeletons.count > 0) {
		model->transforms_changed = false;
		anim_update_skin(model);
	}
}

///////////////////////////////////////////

void render_add_model(model_t model, const matrix &transform, color128 color_linear, render_layer_ layer) {
	render_add_model_mat(model, nullptr, transform, color_linear, layer);
}

///////////////////////////////////////////

void render_draw_queue(const matrix *views, const matrix *projections, int32_t eye_offset, int32_t view_count, render_layer_ filter) {
	skg_event_begin("Render List Setup");

	// Copy camera information into the global buffer
	for (int32_t i = 0; i < view_count; i++) {
		XMMATRIX view_f, projection_f;
		math_matrix_to_fast(views      [i], &view_f      );
		math_matrix_to_fast(projections[i], &projection_f);

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
		vec3          tip  = hand->tracked_state & button_state_active ? hand->fingers[1][4].position : vec3{ 0,-1000,0 };
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
	for (int32_t i = 0; i < _countof(material_buffers); i++) {
		if (material_buffers[i].size != 0)
			skg_buffer_bind(&material_buffers[i].buffer, { (uint16_t)i,  skg_stage_vertex | skg_stage_pixel, skg_register_constant }, 0);
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

	render_list_execute(local.list_primary, filter, view_count, 0, INT_MAX);

	skg_event_end();
}

///////////////////////////////////////////

void render_draw_matrix(const matrix* views, const matrix* projections, int32_t eye_offset, int32_t count, render_layer_ render_filter) {
	render_check_viewpoints();
	render_draw_queue(views, projections, eye_offset, count, render_filter);
	render_check_screenshots();
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

		tex_t render_capture_surface = tex_create(tex_type_image_nomips | tex_type_rendertarget, local.screenshot_list[i].tex_format);
		tex_set_color_arr(render_capture_surface, w, h, nullptr, 1, nullptr, 8);
		tex_release(tex_add_zbuffer(render_capture_surface));

		// Setup to render the screenshot
		skg_tex_target_bind(&render_capture_surface->tex);

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
			float color[4] = {
				local.clear_col.r / 255.f,
				local.clear_col.g / 255.f,
				local.clear_col.b / 255.f,
				local.clear_col.a / 255.f };
			skg_target_clear(
				(local.screenshot_list[i].clear & render_clear_depth),
				(local.screenshot_list[i].clear & render_clear_color) ? &color[0] : (float*)nullptr);
		}

		// Render!
		render_draw_queue(&local.screenshot_list[i].camera, &local.screenshot_list[i].projection, 0, 1, local.screenshot_list[i].layer_filter);
		skg_tex_target_bind(nullptr);

		tex_t resolve_tex = tex_create(tex_type_image_nomips, local.screenshot_list[i].tex_format);
		tex_set_colors(resolve_tex, w, h, nullptr);
		skg_tex_copy_to(&render_capture_surface->tex, &resolve_tex->tex);
		tex_get_data(resolve_tex, buffer, size);
#if defined(SKG_OPENGL)
		int32_t line_size = skg_tex_fmt_size(resolve_tex->tex.format) * resolve_tex->tex.width;
		void* tmp = sk_malloc(line_size);
		for (int32_t y = 0; y < resolve_tex->tex.height / 2; y++) {
			void* top_line = ((uint8_t*)buffer) + line_size * y;
			void* bot_line = ((uint8_t*)buffer) + line_size * ((resolve_tex->tex.height - 1) - y);
			memcpy(tmp, top_line, line_size);
			memcpy(top_line, bot_line, line_size);
			memcpy(bot_line, tmp, line_size);
		}
		sk_free(tmp);
#endif
		tex_release(render_capture_surface);
		tex_release(resolve_tex);

		// Notify that the color data is ready!
		local.screenshot_list[i].render_on_screenshot_callback(buffer, w, h, local.screenshot_list[i].context);
		sk_free(buffer);
		skg_event_end();
	}
	local.screenshot_list.clear();
	skg_tex_target_bind(old_target);
}

///////////////////////////////////////////

void render_check_viewpoints() {
	if (local.viewpoint_list.count == 0) return;

	skg_tex_t *old_target = skg_tex_target_get();
	for (int32_t i = 0; i < local.viewpoint_list.count; i++) {
		skg_event_begin("Viewpoint");
		// Setup to render the screenshot
		skg_tex_target_bind(&local.viewpoint_list[i].rendertarget->tex);

		// Clear the viewport
		if (local.viewpoint_list[i].clear != render_clear_none) {
			float color[4] = {
				local.clear_col.r / 255.f,
				local.clear_col.g / 255.f,
				local.clear_col.b / 255.f,
				local.clear_col.a / 255.f };
			skg_target_clear(
				(local.viewpoint_list[i].clear & render_clear_depth),
				(local.viewpoint_list[i].clear & render_clear_color) ? &color[0] : (float *)nullptr);
		}

		// Set up the viewport if we've got one!
		if (local.viewpoint_list[i].viewport.w != 0) {
			int32_t viewport[4] = {
				(int32_t)(local.viewpoint_list[i].viewport.x * local.viewpoint_list[i].rendertarget->width ),
				(int32_t)(local.viewpoint_list[i].viewport.y * local.viewpoint_list[i].rendertarget->height),
				(int32_t)(local.viewpoint_list[i].viewport.w * local.viewpoint_list[i].rendertarget->width ),
				(int32_t)(local.viewpoint_list[i].viewport.h * local.viewpoint_list[i].rendertarget->height) };
			skg_viewport(viewport);
		}

		// Render!
		render_draw_queue(&local.viewpoint_list[i].camera, &local.viewpoint_list[i].projection, 0, 1, local.viewpoint_list[i].layer_filter);
		skg_tex_target_bind(nullptr);

		// Release the reference we added, the user should have their own ref
		tex_release(local.viewpoint_list[i].rendertarget);
		skg_event_end();
	}
	local.viewpoint_list.clear();
	skg_tex_target_bind(old_target);
}

///////////////////////////////////////////

void render_clear() {
	//log_infof("draws: %d, instances: %d, material: %d, shader: %d, texture %d, mesh %d", render_stats.draw_calls, render_stats.draw_instances, render_stats.swaps_material, render_stats.swaps_shader, render_stats.swaps_texture, render_stats.swaps_mesh);
	render_list_clear(local.list_active);

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
	skg_buffer_bind        (&local.shader_blit, render_list_blit_bind, 0);
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

	skg_tex_target_bind (&to->tex  );
	render_blit_to_bound(material  );
	skg_tex_target_bind (old_target);
}

///////////////////////////////////////////

void render_screenshot(const char* file_utf8, vec3 from_viewpt, vec3 at, int32_t width, int32_t height, float fov_degrees) {
	render_screenshot_pose(file_utf8, 90, { from_viewpt, quat_lookat(from_viewpt, at) }, width, height, fov_degrees);
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

void render_screenshot_pose(const char* file_utf8, int32_t file_quality_100, pose_t viewpoint, int32_t width, int32_t height, float fov_degrees) {
	screenshot_ctx_t *ctx = sk_malloc_t(screenshot_ctx_t, 1);
	ctx->filename = string_copy(file_utf8);
	ctx->quality  = file_quality_100;

	matrix view = matrix_invert(pose_matrix(viewpoint));
	matrix proj = matrix_perspective(fov_degrees, (float)width / height, local.clip_planes.x, local.clip_planes.y);
	local.screenshot_list.add(render_screenshot_t{ render_save_to_file, ctx, view, proj, rect_t{}, width, height, render_layer_all, render_clear_all, tex_format_rgba32 });
}

///////////////////////////////////////////

void render_screenshot_capture(void (*render_on_screenshot_callback)(color32* color_buffer, int32_t width, int32_t height, void* context), pose_t viewpoint, int32_t width, int32_t height, float fov_degrees, tex_format_ tex_format, void* context) {
	matrix view = matrix_invert(pose_matrix(viewpoint));
	matrix proj = matrix_perspective(fov_degrees, (float)width / height, local.clip_planes.x, local.clip_planes.y);
	local.screenshot_list.add(render_screenshot_t{ render_on_screenshot_callback, context, view, proj, rect_t{}, width, height, render_layer_all, render_clear_all, tex_format });
}

///////////////////////////////////////////

void render_screenshot_viewpoint(void (*render_on_screenshot_callback)(color32* color_buffer, int32_t width, int32_t height, void* context), matrix camera, matrix projection, int32_t width, int32_t height, render_layer_ layer_filter, render_clear_ clear, rect_t viewport, tex_format_ tex_format, void* context) {
	matrix inv_cam = matrix_invert(camera);
	local.screenshot_list.add(render_screenshot_t{ render_on_screenshot_callback, context, inv_cam, projection, viewport, width, height, layer_filter, clear, tex_format });
}

///////////////////////////////////////////

void render_to(tex_t to_rendertarget, const matrix &camera, const matrix &projection, render_layer_ layer_filter, render_clear_ clear, rect_t viewport) {
	render_material_to(to_rendertarget, nullptr, camera, projection, layer_filter, clear, viewport);
}

///////////////////////////////////////////

void render_material_to(tex_t to_rendertarget, material_t override_material, const matrix& camera, const matrix& projection, render_layer_ layer_filter, render_clear_ clear, rect_t viewport) {
	if ((to_rendertarget->type & tex_type_rendertarget) == 0) {
		log_err("render_to texture must be a render target texture type!");
		return;
	}
	tex_addref(to_rendertarget);

	matrix inv_cam;
	matrix_inverse(camera, inv_cam);
	render_viewpoint_t viewpoint = {};
	viewpoint.rendertarget      = to_rendertarget;
	viewpoint.camera            = inv_cam;
	viewpoint.projection        = projection;
	viewpoint.layer_filter      = layer_filter;
	viewpoint.viewport          = viewport;
	viewpoint.clear             = clear;
	viewpoint.override_material = override_material;
	local.viewpoint_list.add(viewpoint);
}

///////////////////////////////////////////

void render_set_material(material_t material) {
	if (material == local.last_material)
		return;
	local.last_material = material;
	local.lists[local.list_active].stats.swaps_material++;

	// Update and bind the material parameter buffer
	if (material->args.buffer != nullptr) {
		skg_buffer_bind(&material->args.buffer_gpu, material->args.buffer_bind, 0);
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

skg_buffer_t *render_fill_inst_buffer(array_t<render_transform_buffer_t> &list, int32_t &offset, int32_t &out_count) {
	// Find a buffer that can contain this list! Or the biggest one
	int32_t size  = list.count - offset;
	int32_t start = offset;

	// Check if it fits, if it doesn't, then set up data so we only fill what we have!
	if (size > render_instance_max) {
		offset   += render_instance_max;
		out_count = render_instance_max;
	} else {
		// this means we've gotten through the whole list :)
		offset    = 0;
		out_count = size;
	}

	// Copy data into the buffer, and return it!
	skg_buffer_set_contents(&local.instance_buffer, &list[start], sizeof(render_transform_buffer_t) * out_count);
	return &local.instance_buffer;
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

render_list_t render_list_create() {
	int32_t id = local.lists.index_where(&_render_list_t::state, render_list_state_destroyed);
	if (id == -1)
		id = local.lists.add({});
	return id;
}

///////////////////////////////////////////

void render_list_release(render_list_t list) {
	local.lists[list].queue.free();
	local.lists[list] = {};
	local.lists[list].state = render_list_state_destroyed;
}

///////////////////////////////////////////

void render_list_push(render_list_t list) {
	local.list_active = local.list_stack.add(list);
	local.lists[list].state = render_list_state_used;
}

///////////////////////////////////////////

void render_list_pop() {
	local.list_stack.pop();
	local.list_active = local.list_stack.count - 1;
}

///////////////////////////////////////////

void render_list_add(const render_item_t *item) {
	local.lists[local.list_active].queue.add(*item);
	assets_addref(&item->material->header);
	assets_addref(&item->mesh->header);
}

///////////////////////////////////////////

void render_list_add_to(render_list_t list, const render_item_t *item) {
	local.lists[list].queue.add(*item);
	assets_addref(&item->material->header);
	assets_addref(&item->mesh->header);
}

///////////////////////////////////////////

inline void render_list_execute_run(_render_list_t *list, material_t material, const skg_mesh_t *mesh, int32_t mesh_inds, uint32_t view_count) {
	render_set_material(material);
	skg_mesh_bind      (mesh);
	list->stats.swaps_mesh++;

	// Collect and draw instances
	int32_t offsets = 0, inst_count = 0;
	do {
		skg_buffer_t *instances = render_fill_inst_buffer(local.instance_list, offsets, inst_count);
		skg_buffer_bind(instances, render_list_inst_bind, 0);

		skg_draw(0, 0, mesh_inds, inst_count * view_count);
		list->stats.draw_calls     += 1;
		list->stats.draw_instances += inst_count;

	} while (offsets != 0);
}

///////////////////////////////////////////

void render_list_execute(render_list_t list_id, render_layer_ filter, uint32_t view_count, int32_t queue_start, int32_t queue_end) {
	_render_list_t *list = &local.lists[list_id];
	list->state = render_list_state_rendering;

	if (list->queue.count == 0) {
		list->state = render_list_state_rendered; 
		return;
	}
	render_list_prep(list_id);
	uint64_t sort_id_start = render_sort_id_from_queue(queue_start);
	uint64_t sort_id_end   = render_sort_id_from_queue(queue_end);

	render_item_t *run_start = nullptr;
	for (int32_t i = 0; i < list->queue.count; i++) {
		render_item_t *item = &list->queue[i];
		
		// Skip this item if it's filtered out
		if ((item->layer & filter) == 0 || item->sort_id < sort_id_start) continue;
		// End early if we're past the end of the desired queue range
		if (item->sort_id >= sort_id_end) break;

		// If it's the first in the run, record the material/mesh
		if (run_start == nullptr) {
			run_start = item;
		}
		// If the material/mesh changed
		else if (run_start->material != item->material || run_start->mesh != item->mesh) {
			// Render the run that just ended
			render_list_execute_run(list, run_start->material, &run_start->mesh->gpu_mesh, run_start->mesh_inds, view_count);
			local.instance_list.clear();
			// Start the next run
			run_start = item;
		}

		// Add the current item to the run of instances
		XMMATRIX transpose = XMMatrixTranspose(item->transform);
		local.instance_list.add(render_transform_buffer_t{ transpose, item->color });
	}
	// Render the last remaining run, which won't be triggered by the loop's
	// conditions
	if (local.instance_list.count > 0) {
		render_list_execute_run(list, run_start->material, &run_start->mesh->gpu_mesh, run_start->mesh_inds, view_count);
		local.instance_list.clear();
	}

	list->state = render_list_state_rendered;
}

///////////////////////////////////////////

void render_list_execute_material(render_list_t list_id, render_layer_ filter, uint32_t view_count, int32_t queue_start, int32_t queue_end, material_t override_material) {
	_render_list_t *list = &local.lists[list_id];
	list->state = render_list_state_rendering;

	if (list->queue.count == 0) {
		list->state = render_list_state_rendered;
		return;
	}
	// TODO: this isn't entirely optimal here, this would be best if sorted
	// solely by the mesh id since we only have one single material.
	render_list_prep(list_id);
	material_check_dirty(override_material);
	uint64_t sort_id_start = render_sort_id_from_queue(queue_start);
	uint64_t sort_id_end   = render_sort_id_from_queue(queue_end);

	render_item_t *run_start = nullptr;
	for (int32_t i = 0; i < list->queue.count; i++) {
		render_item_t *item = &list->queue[i];

		// Skip this item if it's filtered out
		if ((item->layer & filter) == 0 || item->sort_id < sort_id_start) continue;
		// End early if we're past the end of the desired queue range
		if (item->sort_id >= sort_id_end) break;

		// If it's the first in the run, record the material/mesh
		if (run_start == nullptr) {
			run_start = item;
		}
		// If the mesh changed
		else if (run_start->mesh != item->mesh) {
			// Render the run that just ended
			render_list_execute_run(list, override_material, &run_start->mesh->gpu_mesh, run_start->mesh_inds, view_count);
			local.instance_list.clear();
			// Start the next run
			run_start = item;
		}

		// Add the current item to the run of instances
		XMMATRIX transpose = XMMatrixTranspose(item->transform);
		local.instance_list.add(render_transform_buffer_t{ transpose, item->color });
	}
	// Render the last remaining run, which won't be triggered by the loop's
	// conditions
	if (local.instance_list.count > 0) {
		render_list_execute_run(list, override_material, &run_start->mesh->gpu_mesh, run_start->mesh_inds, view_count);
		local.instance_list.clear();
	}

	list->state = render_list_state_rendered;
}

///////////////////////////////////////////

void render_list_prep(render_list_t list_id) {
	_render_list_t *list = &local.lists[list_id];
	if (list->prepped) return;

	// Sort the render queue
	radix_sort7(&list->queue[0], list->queue.count);

	// Make sure the material buffers are all up-to-date
	material_t curr = nullptr;
	for (int32_t i = 0; i < list->queue.count; i++) {
		if (curr == list->queue[i].material) continue;
		curr = list->queue[i].material;
		material_check_dirty(curr);
	}

	list->prepped = true;
}

///////////////////////////////////////////

void render_list_clear(render_list_t list) {
	for (int32_t i = 0; i < local.lists[list].queue.count; i++) {
		assets_releaseref(&local.lists[list].queue[i].material->header);
		assets_releaseref(&local.lists[list].queue[i].mesh->header);
	}
	local.lists[list].queue.clear();
	local.lists[list].stats   = {};
	local.lists[list].prepped = false;
	local.lists[list].state   = render_list_state_empty;
}

///////////////////////////////////////////

int32_t render_list_item_count(render_list_t list) {
	return local.lists[list].queue.count;
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
