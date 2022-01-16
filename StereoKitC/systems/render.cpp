#include "render.h"
#include "world.h"
#include "defaults.h"
#include "../_stereokit.h"
#include "../libraries/sk_gpu.h"
#include "../libraries/stref.h"
#include "../sk_math.h"
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
#include "../asset_types/render_list.h"
#include "../systems/input.h"
#include "../systems/platform/flatscreen_input.h"
#include "../systems/platform/platform_utils.h"

#pragma warning(push)
#pragma warning(disable : 26451 26819 6386 6385 )
#if defined(_WIN32)
#define __STDC_LIB_EXT1__
#endif
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../libraries/stb_image_write.h"
#pragma warning(pop)

#include <DirectXMath.h> // Matrix math functions and objects
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
	char   *filename;
	vec3    from;
	vec3    at;
	int32_t width;
	int32_t height;
	float   fov_degrees;
	tex_t   surface;
	tex_t   copy_surface;
};
struct render_viewpoint_t {
	tex_t                  rendertarget;
	render_list_t          render_list;
	render_pass_settings_t settings;
	matrix                *viewpoints;
	matrix                *viewpoint_projections;
	int32_t                viewpoint_count;
};

///////////////////////////////////////////

array_t<render_transform_buffer_t> render_instance_list   = {};
skg_buffer_t                       render_instance_buffer = {};
const int32_t                      render_instance_max    = 819;

material_buffer_t       render_shader_globals;
skg_buffer_t            render_shader_blit;
matrix                  render_camera_root           = matrix_identity;
matrix                  render_camera_root_final     = matrix_identity;
matrix                  render_camera_root_final_inv = matrix_identity;
matrix                  render_default_camera_proj;
vec2                    render_clip_planes           = {0.02f, 50};
float                   render_fov                   = 90;
render_global_buffer_t  render_global_buffer;
mesh_t                  render_blit_quad;
vec4                    render_lighting[9]           = {};
spherical_harmonics_t   render_lighting_src          = {};
color128                render_clear_col             = {0,0,0,1};
render_layer_           render_primary_filter        = render_layer_all;
render_layer_           render_capture_filter        = render_layer_all;
bool                    render_use_capture_filter    = false;
tex_t                   render_global_textures[16]   = {};
render_list_t           render_list_primary          = nullptr;

array_t<render_screenshot_t>  render_screenshot_list        = {};
array_t<render_viewpoint_t>   render_viewpoint_list         = {};
array_t<matrix>               render_viewpoint_matrix_cache = {};

const int32_t           render_skytex_register = 11;
mesh_t                  render_sky_mesh    = nullptr;
material_t              render_sky_mat     = nullptr;
bool32_t                render_sky_show    = false;
vec4                    render_sky_dims    = {};

material_t              render_last_material;
shader_t                render_last_shader;
mesh_t                  render_last_mesh;

skg_bind_t              render_bind_globals   = { 1,  skg_stage_vertex | skg_stage_pixel, skg_register_constant };
skg_bind_t              render_bind_instances = { 2,  skg_stage_vertex | skg_stage_pixel, skg_register_constant };
skg_bind_t              render_bind_blit      = { 2,  skg_stage_vertex | skg_stage_pixel, skg_register_constant };

///////////////////////////////////////////

void          render_set_material           (material_t material);
skg_buffer_t *render_fill_inst_buffer       (array_t<render_transform_buffer_t> &list, int32_t &offset, int32_t &out_count);
void          render_check_screenshots      ();
void          render_viewpoints             ();
void          render_draw_viewpoint         (render_viewpoint_t *view);
void          render_instanced_list         (const array_t<render_item_t> items, render_layer_ filter, uint32_t view_count, render_stats_t *stats);
void          render_instanced_list_material(const array_t<render_item_t> items, render_layer_ filter, uint32_t view_count, material_t override_material, render_stats_t *stats);

///////////////////////////////////////////

void render_set_clip(float near_plane, float far_plane) {
	// near_plane will throw divide by zero errors if it's zero! So we'll
	// clamp it :) Anything this low will probably look bad due to depth
	// artifacts though.
	near_plane = fmaxf(0.001f, near_plane);
	render_clip_planes = { near_plane, far_plane };
	render_update_projection();
}

///////////////////////////////////////////

void render_set_fov(float field_of_view_degrees) {
	render_fov = field_of_view_degrees;
	render_update_projection();
}

///////////////////////////////////////////

void render_update_projection() {
	math_fast_to_matrix( XMMatrixPerspectiveFovRH(
		render_fov * deg2rad, 
		(float)sk_system_info().display_width/sk_system_info().display_height, 
		render_clip_planes.x, 
		render_clip_planes.y), &render_default_camera_proj);
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

skg_tex_fmt_ render_preferred_depth_fmt() {
	depth_mode_ mode = sk_get_settings().depth_mode;
	switch (mode) {
	case depth_mode_balanced:
#if defined(SK_OS_WINDOWS_UWP) || defined(SK_OS_ANDROID)
		return skg_tex_fmt_depth16;
#else
		return skg_tex_fmt_depth32;
#endif
		break;
	case depth_mode_d16:     return skg_tex_fmt_depth16;
	case depth_mode_d32:     return skg_tex_fmt_depth32;
	case depth_mode_stencil: return skg_tex_fmt_depthstencil;
	default: return skg_tex_fmt_depth16;
	}
}

///////////////////////////////////////////

matrix render_get_projection() {
	return render_default_camera_proj;
}

///////////////////////////////////////////

vec2 render_get_clip() {
	return render_clip_planes;
}

///////////////////////////////////////////

matrix render_get_cam_root() {
	return render_camera_root;
}

///////////////////////////////////////////

matrix render_get_cam_final() {
	return render_camera_root_final;
}

///////////////////////////////////////////

matrix render_get_cam_final_inv() {
	return render_camera_root_final_inv;
}

///////////////////////////////////////////

void render_set_cam_root(const matrix &cam_root) {
	render_camera_root       = cam_root;
	render_camera_root_final = fltscr_transform * cam_root;
	matrix_inverse(render_camera_root_final, render_camera_root_final_inv);

	// TODO: May want to also update controllers/hands?
	quat rot = matrix_extract_rotation(render_camera_root_final);
	input_head_pose_world.position    = render_camera_root_final * input_head_pose_local.position;
	input_head_pose_world.orientation = rot * input_head_pose_local.orientation;
	input_eyes_pose_world.position    = render_camera_root_final * input_eyes_pose_local.position;
	input_eyes_pose_world.orientation = rot * input_eyes_pose_local.orientation;

	world_refresh_transforms();
	input_update_poses(false);
}

///////////////////////////////////////////

void render_set_skytex(tex_t sky_texture) {
	if (sky_texture != nullptr && !(sky_texture->type & tex_type_cubemap)) {
		log_err("render_set_skytex: Attempting to set the skybox texture to a texture that's not a cubemap! Sorry, but cubemaps only here please!");
		return;
	}

	render_global_texture(render_skytex_register, sky_texture);
	render_sky_dims = sky_texture != nullptr 
		? vec4{ (float)sky_texture->tex.width, (float)sky_texture->tex.height, floorf(log2f((float)sky_texture->tex.width)), 0 }
		: vec4{};
}

///////////////////////////////////////////

tex_t render_get_skytex() {
	if (render_global_textures[render_skytex_register] != nullptr)
		tex_addref(render_global_textures[render_skytex_register]);
	return render_global_textures[render_skytex_register];
}

///////////////////////////////////////////

void render_set_skylight(const spherical_harmonics_t &light_info) {
	render_lighting_src = light_info;
	sh_to_fast(light_info, render_lighting);
}

///////////////////////////////////////////

spherical_harmonics_t render_get_skylight() {
	return render_lighting_src;
}

///////////////////////////////////////////

void render_set_primary_list(render_list_t render_list) {
	if (render_list == nullptr) {
		log_err("render_set_primary_list: must be a valid list!");
		return;
	}
	assets_safeswap_ref(
		(asset_header_t**)&render_list_primary,
		(asset_header_t* ) render_list);
}

///////////////////////////////////////////

render_list_t render_get_primary_list() {
	render_list_addref(render_list_primary);
	return render_list_primary;
}

///////////////////////////////////////////

render_layer_ render_get_filter() {
	return render_primary_filter;
}

///////////////////////////////////////////

void render_set_filter(render_layer_ layer_filter) {
	render_primary_filter = layer_filter;
}

///////////////////////////////////////////

void render_override_capture_filter(bool32_t use_override_filter, render_layer_ layer_filter) {
	render_use_capture_filter = use_override_filter;
	render_capture_filter    = layer_filter;
}

///////////////////////////////////////////

render_layer_ render_get_capture_filter() {
	return render_use_capture_filter
		? render_capture_filter
		: render_primary_filter;
}

///////////////////////////////////////////

bool32_t render_has_capture_filter() {
	return render_use_capture_filter;
}

///////////////////////////////////////////

void render_enable_skytex(bool32_t show_sky) {
	render_sky_show = show_sky;
}

///////////////////////////////////////////

bool32_t render_enabled_skytex() {
	return render_sky_show;
}

///////////////////////////////////////////

void render_global_texture(int32_t register_slot, tex_t texture) {
	if (register_slot < 0 || register_slot >= _countof(render_global_textures)) {
		log_errf("render_global_texture: Register_slot should be 0-16. Received %d.", register_slot);
		return;
	}

	if (render_global_textures[register_slot] == texture) return;

	if (texture != nullptr)
		tex_addref(texture);
	if (render_global_textures[register_slot] != nullptr)
		tex_release(render_global_textures[register_slot]);

	render_global_textures[register_slot] = texture;
}

///////////////////////////////////////////

void render_set_clear_color(color128 color) {
	render_clear_col = color_to_linear(color);
}

//////////////////////////////////////////

color128 render_get_clear_color() {
	return render_clear_col;
}

///////////////////////////////////////////

void render_add_mesh(mesh_t mesh, material_t material, const matrix &transform, color128 color, render_layer_ layer) {
	render_list_add_mesh(render_list_primary, mesh, material, transform, color, layer);
}

///////////////////////////////////////////

void render_add_model(model_t model, const matrix &transform, color128 color, render_layer_ layer) {
	render_list_add_model(render_list_primary, model, transform, color, layer);
}

///////////////////////////////////////////

void render_all() {
	render_check_screenshots();
	render_viewpoints();
}

///////////////////////////////////////////

void render_viewpoints() {
	if (render_viewpoint_list.count == 0) return;

	skg_tex_t *old_target = skg_tex_target_get();
	for (size_t i = 0; i < render_viewpoint_list.count; i++) {

		skg_tex_target_bind  (&render_viewpoint_list[i].rendertarget->tex);
		render_draw_viewpoint(&render_viewpoint_list[i]);
		//skg_tex_target_bind(nullptr);

		// Release the references we added, the user should have their own
		// references
		tex_release        (render_viewpoint_list[i].rendertarget);
		render_list_release(render_viewpoint_list[i].render_list);
	}
	render_viewpoint_list        .clear();
	render_viewpoint_matrix_cache.clear();
	skg_tex_target_bind(old_target);
}

///////////////////////////////////////////

void render_list_draw_to(render_list_t list, tex_t to_rendertarget, const matrix *viewpoints, const matrix *viewpoint_projections, uint32_t viewpoint_count, render_pass_settings_t pass_settings) {
	if ((to_rendertarget->type & tex_type_rendertarget) == 0) {
		log_err("render_to texture must be a render target texture type!");
		return;
	}

	tex_addref        (to_rendertarget);
	render_list_addref(list);

	int32_t viewpoint_start = render_viewpoint_matrix_cache.count;
	for (int32_t i = 0; i < viewpoint_count; i++)
		render_viewpoint_matrix_cache.add(matrix_invert(viewpoints[i]));
	int32_t projection_start = render_viewpoint_matrix_cache.count;
	for (int32_t i = 0; i < viewpoint_count; i++)
		render_viewpoint_matrix_cache.add(viewpoint_projections[i]);

	render_viewpoint_t viewpoint = {};
	viewpoint.render_list           = list;
	viewpoint.rendertarget          = to_rendertarget;
	viewpoint.settings              = pass_settings;
	viewpoint.viewpoint_count       = viewpoint_count;
	viewpoint.viewpoints            = &render_viewpoint_matrix_cache[viewpoint_start];
	viewpoint.viewpoint_projections = &render_viewpoint_matrix_cache[projection_start];
	render_viewpoint_list.add(viewpoint);
}

///////////////////////////////////////////

void render_draw_viewpoint(render_viewpoint_t *view) {
	// Clear the viewport
	render_clear_ clear = view->settings.surface_clear == render_clear_default
		? render_clear_all
		: view->settings.surface_clear;
	if (clear != render_clear_none) {
		float *color = (clear & render_clear_color)
			? &view->settings.clear_color_linear.r
			: (float *)nullptr;
		skg_target_clear(clear & render_clear_depth, color);
	}

	// Set up the viewport if we've got one!
	if (view->settings.viewport.w != 0) {
		skg_tex_t *target = skg_tex_target_get();
		int32_t    w = 0, h = 0;
		if (target) {
			w = target->width;
			h = target->height;
		}
		int32_t viewport[4] = {
			(int32_t)(view->settings.viewport.x * w),
			(int32_t)(view->settings.viewport.y * h),
			(int32_t)(view->settings.viewport.w * w),
			(int32_t)(view->settings.viewport.h * h) };
		skg_viewport(viewport);
	}

	// Copy camera information into the global buffer
	for (int32_t i = 0; i < view->viewpoint_count; i++) {
		XMMATRIX view_f, projection_f;
		math_matrix_to_fast(view->viewpoints           [i], &view_f      );
		math_matrix_to_fast(view->viewpoint_projections[i], &projection_f);

		XMMATRIX view_inv = XMMatrixInverse(nullptr, view_f      );
		XMMATRIX proj_inv = XMMatrixInverse(nullptr, projection_f);

		XMVECTOR cam_pos = XMVector3Transform      (DirectX::g_XMIdentityR3,    view_inv);
		XMVECTOR cam_dir = XMVector3TransformNormal(DirectX::g_XMNegIdentityR2, view_inv);
		XMStoreFloat3((XMFLOAT3*)&render_global_buffer.camera_pos[i], cam_pos);
		XMStoreFloat3((XMFLOAT3*)&render_global_buffer.camera_dir[i], cam_dir);

		render_global_buffer.view    [i] = XMMatrixTranspose(view_f);
		render_global_buffer.proj    [i] = XMMatrixTranspose(projection_f);
		render_global_buffer.proj_inv[i] = XMMatrixTranspose(proj_inv);
		render_global_buffer.viewproj[i] = XMMatrixTranspose(view_f * projection_f);
	}

	// Copy in the other global shader variables
	memcpy(render_global_buffer.lighting, render_lighting, sizeof(vec4) * 9);
	render_global_buffer.time       = time_getf();
	render_global_buffer.view_count = view->viewpoint_count;
	vec3 tip = input_hand(handed_right)->tracked_state & button_state_active ? input_hand(handed_right)->fingers[1][4].position : vec3{0,-1000,0};
	render_global_buffer.fingertip[0] = { tip.x, tip.y, tip.z, 0 };
	tip = input_hand(handed_left)->tracked_state & button_state_active ? input_hand(handed_left)->fingers[1][4].position : vec3{0,-1000,0};
	render_global_buffer.fingertip[1] = { tip.x, tip.y, tip.z, 0 };
	render_global_buffer.cubemap_i    = render_sky_dims;

	// Upload shader globals and set them active!
	material_buffer_set_data(render_shader_globals, &render_global_buffer);

	// Activate any material buffers we have
	for (size_t i = 0; i < _countof(material_buffers); i++) {
		if (material_buffers[i].size != 0)
			skg_buffer_bind(&material_buffers[i].buffer, { (uint16_t)i,  skg_stage_vertex | skg_stage_pixel, skg_register_constant }, 0);
	}

	// Activate any global textures we have
	for (size_t i = 0; i < _countof(render_global_textures); i++) {
		if (render_global_textures[i] != nullptr)
			skg_tex_bind(&render_global_textures[i]->tex, { (uint16_t)i,  skg_stage_vertex | skg_stage_pixel, skg_register_resource});
	}

	// Draw the list!
	render_list_sort(view->render_list);
	if (view->settings.override_material == nullptr)
		render_instanced_list         (view->render_list->queue, view->settings.layer_filter, view->viewpoint_count, &view->render_list->stats);
	else
		render_instanced_list_material(view->render_list->queue, view->settings.layer_filter, view->viewpoint_count, view->settings.override_material, &view->render_list->stats);
}

///////////////////////////////////////////

void render_check_screenshots() {
	if (render_screenshot_list.count == 0) return;

	// Each screenshot works in 3 passes:
	// - Frame 1: Render the screenshot to a rendertarget
	// - Frame 2: Copy the rendertarget to a texture we can read from
	// - Frame 3: Read from final texture, and write to file

	for (size_t i = 0; i < render_screenshot_list.count; i++) {
		int32_t  w = render_screenshot_list[i].width;
		int32_t  h = render_screenshot_list[i].height;

		if (render_screenshot_list[i].copy_surface != nullptr) {

			// Frame 3: Save the screenshot to file, and remove this from our
			// list of screenshots.
			size_t   size   = sizeof(color32) * w * h;
			color32 *buffer = (color32*)sk_malloc(size);
			tex_get_data  (render_screenshot_list[i].copy_surface, buffer, size);
			stbi_write_jpg(render_screenshot_list[i].filename, w, h, 4, buffer, 90);

			tex_release (render_screenshot_list[i].copy_surface);
			free(buffer);
			free(render_screenshot_list[i].filename);

			render_screenshot_list.remove(i);
			i--;

		} else if (render_screenshot_list[i].surface != nullptr) {

			// Frame 2: Set up a copy to get data from the rendertarget a
			// readable surface.
			/*tex_t downsampled = tex_create(tex_type_image_nomips | tex_type_rendertarget);
			tex_set_colors(downsampled, w*2, h*2, nullptr);
			material_set_texture(sk_default_material_blit_linear, "source", render_screenshot_list[i].surface);
			render_blit(downsampled, sk_default_material_blit_linear); */

			tex_t downsampled2 = tex_create(tex_type_image_nomips | tex_type_rendertarget);
			tex_set_colors ( downsampled2, w, h, nullptr);
			material_set_texture(sk_default_material_blit_linear, "source", render_screenshot_list[i].surface);//downsampled);
			render_blit(downsampled2, sk_default_material_blit_linear); 

			render_screenshot_list[i].copy_surface = tex_create(tex_type_image_nomips);
			tex_set_colors ( render_screenshot_list[i].copy_surface, w, h, nullptr);
			skg_tex_copy_to(&downsampled2->tex, &render_screenshot_list[i].copy_surface->tex);

			tex_release    (render_screenshot_list[i].surface);
			//tex_release    ( downsampled);
			tex_release    (downsampled2);

		} else {
			// Frame 1: Prepare to render the screenshot so we can save it to
			// file on later frames

			// Create the screenshot surface
			render_screenshot_list[i].surface = tex_create(tex_type_image_nomips | tex_type_rendertarget);
			tex_set_color_arr(render_screenshot_list[i].surface, w*2, h*2, nullptr, 1, nullptr, 1);
			tex_add_zbuffer  (render_screenshot_list[i].surface);

			// Set up the render
			matrix view = matrix_trs(
				render_screenshot_list[i].from,
				quat_lookat(render_screenshot_list[i].from, render_screenshot_list[i].at));
			matrix proj = matrix_perspective(render_screenshot_list[i].fov_degrees, (float)w / h, render_clip_planes.x, render_clip_planes.y);
			render_pass_settings_t settings = {};
			settings.layer_filter       = render_get_filter();
			settings.clear_color_linear = render_get_clear_color();
			render_list_draw_to(render_list_primary, render_screenshot_list[i].surface, &view, &proj, 1, settings);
		}
	}
}

///////////////////////////////////////////

void render_screenshot(const char *file, vec3 from_viewpt, vec3 at, int width, int height, float fov_degrees) {
	char *file_copy = string_copy(file);
	render_screenshot_list.add( render_screenshot_t{ file_copy, from_viewpt, at, width, height, fov_degrees });
}

///////////////////////////////////////////

void render_clear() {
	//log_infof("draws: %d, instances: %d, material: %d, shader: %d, texture %d, mesh %d", render_stats.draw_calls, render_stats.draw_instances, render_stats.swaps_material, render_stats.swaps_shader, render_stats.swaps_texture, render_stats.swaps_mesh);
	render_list_clear(render_list_primary);

	render_last_material = nullptr;
	render_last_shader   = nullptr;
	render_last_mesh     = nullptr;
}

///////////////////////////////////////////

bool render_init() {
	render_shader_globals  = material_buffer_create(1, sizeof(render_global_buffer));
	render_shader_blit     = skg_buffer_create(nullptr, 1, sizeof(render_blit_data_t), skg_buffer_type_constant, skg_use_dynamic);
	
	render_instance_buffer = skg_buffer_create(nullptr, render_instance_max, sizeof(render_transform_buffer_t), skg_buffer_type_constant, skg_use_dynamic);
	render_instance_list.resize(render_instance_max);

	// Setup a default camera
	render_set_clip(render_clip_planes.x, render_clip_planes.y);

	// Set up resources for doing blit operations
	render_blit_quad = mesh_find(default_id_mesh_screen_quad);

	// Create a default skybox
	render_sky_mesh = mesh_create();
	vind_t inds [] = {2,1,0, 3,2,0};
	vert_t verts[] = {
		vert_t{ {-1, 1,1}, {0,0,1}, {0,0}, {255,255,255,255} },
		vert_t{ { 1, 1,1}, {0,0,1}, {1,0}, {255,255,255,255} },
		vert_t{ { 1,-1,1}, {0,0,1}, {1,1}, {255,255,255,255} },
		vert_t{ {-1,-1,1}, {0,0,1}, {0,1}, {255,255,255,255} }, };
	mesh_set_inds (render_sky_mesh, inds,  _countof(inds));
	mesh_set_verts(render_sky_mesh, verts, _countof(verts));
	mesh_set_id   (render_sky_mesh, "render/skybox_mesh");

	shader_t shader_sky = shader_find(default_id_shader_sky);
	render_sky_mat = material_create(shader_sky);
	shader_release(shader_sky);

	material_set_id          (render_sky_mat, "render/skybox_material");
	material_set_queue_offset(render_sky_mat, 100);
	
	render_list_primary = render_list_create();
	render_list_set_id(render_list_primary, "render/list");

	return true;
}

///////////////////////////////////////////

void render_update() {
	if (hierarchy_stack.count > 0)
		log_err("Render transform stack doesn't have matching begin/end calls!");

	if (render_sky_show && sk_system_info().display_type == display_opaque) {
		render_add_mesh(render_sky_mesh, render_sky_mat, matrix_identity, {1,1,1,1}, render_layer_vfx);
	}
}

///////////////////////////////////////////

void render_shutdown() {
	// There may be remaining copy and file write frames remaining here, this
	// should total no more than 2 steps at most.
	while (render_screenshot_list.count > 0)
		render_check_screenshots();

	render_screenshot_list       .free();
	render_viewpoint_list        .free();
	render_viewpoint_matrix_cache.free();
	render_instance_list         .free();

	for (size_t i = 0; i < _countof(render_global_textures); i++) {
		tex_release(render_global_textures[i]);
	}
	material_release       (render_sky_mat);
	mesh_release           (render_sky_mesh);
	mesh_release           (render_blit_quad);
	material_buffer_release(render_shader_globals);
	render_list_release    (render_list_primary);

	skg_buffer_destroy(&render_instance_buffer);
	skg_buffer_destroy(&render_shader_blit);

	//radix_sort_clean();
}

///////////////////////////////////////////

void render_blit_to_bound(material_t material) {
	// Wipe our swapchain color and depth target clean, and then set them
	// up for rendering!
	float color[4] = { 0,0,0,0 };
	skg_target_clear(true, color);

	skg_tex_t *target = skg_tex_target_get();
	float w = sk_info.display_width;
	float h = sk_info.display_height;
	if (target != nullptr) {
		w = target->width;
		h = target->height;
	}

	// Setup shader args for the blit operation
	render_blit_data_t data = {};
	data.width  = w;
	data.height = h;
	data.pixel_width  = 1.0f / w;
	data.pixel_height = 1.0f / h;

	// Setup render states for blitting
	skg_buffer_set_contents(&render_shader_blit, &data, sizeof(render_blit_data_t));
	skg_buffer_bind        (&render_shader_blit, render_bind_blit, 0);
	render_set_material(material);
	skg_mesh_bind(&render_blit_quad->gpu_mesh);
	
	// And draw to it!
	skg_draw(0, 0, render_blit_quad->ind_count, 1);

	render_last_material = nullptr;
	render_last_mesh     = nullptr;
	render_last_shader   = nullptr;
}

///////////////////////////////////////////

void render_blit(tex_t to, material_t material) {
	if (!(to->type & tex_type_rendertarget)) {
		log_err("render_blit: target must be a rendertexture!");
		return;
	}
	skg_tex_t *old_target = skg_tex_target_get();

	skg_tex_target_bind (&to->tex  );
	render_blit_to_bound(material  );
	skg_tex_target_bind (old_target);
}

///////////////////////////////////////////

void render_to(tex_t to_rendertarget, const matrix &camera, const matrix &projection, render_layer_ layer_filter, render_clear_ clear, rect_t viewport) {
	render_material_to(to_rendertarget, nullptr, camera, projection, layer_filter, clear, viewport);
}

///////////////////////////////////////////

void render_material_to(tex_t to_rendertarget, material_t override_material, const matrix& camera, const matrix& projection, render_layer_ layer_filter, render_clear_ clear, rect_t viewport) {
	render_pass_settings_t settings = {};
	settings.layer_filter      = layer_filter;
	settings.override_material = override_material;
	settings.surface_clear     = clear;
	settings.viewport          = viewport;
	settings.clear_color_linear= render_get_clear_color();

	render_list_draw_to(render_list_primary, to_rendertarget, &camera, &projection, 1, settings);
	if ((to_rendertarget->type & tex_type_rendertarget) == 0) {
		log_err("render_to texture must be a render target texture type!");
		return;
	}
}

///////////////////////////////////////////

void render_set_material(material_t material) {
	if (material == render_last_material)
		return;
	render_last_material = material;
	render_list_primary->stats.swaps_material++;

	// Update and bind the material parameter buffer
	if (material->args.buffer != nullptr) {
		if (material->args.buffer_dirty) {
			skg_buffer_set_contents(&material->args.buffer_gpu, material->args.buffer, (uint32_t)material->args.buffer_size);
			material->args.buffer_dirty = false;
		}
		skg_buffer_bind(&material->args.buffer_gpu, material->args.buffer_bind, 0);
	}

	// Bind the material textures
	for (int32_t i = 0; i < material->args.texture_count; i++) {
		if (render_global_textures[material->args.texture_binds[i].slot] == nullptr)
			skg_tex_bind(&material->args.textures[i]->tex, material->args.texture_binds[i]);
	}

	// And bind the pipeline
	skg_pipeline_bind(&material->pipeline);
}

///////////////////////////////////////////

skg_buffer_t *render_fill_inst_buffer(array_t<render_transform_buffer_t> &list, int32_t &offset, int32_t &out_count) {
	// Find a buffer that can contain this list! Or the biggest one
	int32_t size  = (int32_t)list.count - offset;
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
	skg_buffer_set_contents(&render_instance_buffer, &list[start], sizeof(render_transform_buffer_t) * out_count);
	return &render_instance_buffer;
}

///////////////////////////////////////////

vec3 render_unproject_pt(vec3 normalized_screen_pt) {
	XMMATRIX fast_proj, fast_view;
	math_matrix_to_fast(render_get_projection(), &fast_proj);
	math_matrix_to_fast(render_camera_root_final_inv,  &fast_view);
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

inline void _render_instanced_list_run(material_t material, const skg_mesh_t *mesh, int32_t mesh_inds, uint32_t view_count, render_stats_t *stats) {
	render_set_material(material);
	skg_mesh_bind      (mesh);
	stats->swaps_mesh++;

	// Collect and draw instances
	int32_t offsets = 0, inst_count = 0;
	do {
		skg_buffer_t *instances = render_fill_inst_buffer(render_instance_list, offsets, inst_count);
		skg_buffer_bind(instances, render_bind_instances, 0);

		skg_draw(0, 0, mesh_inds, inst_count * view_count);
		stats->draw_calls     += 1;
		stats->draw_instances += inst_count;

	} while (offsets != 0);
}

///////////////////////////////////////////

void render_instanced_list(const array_t<render_item_t> items, render_layer_ filter, uint32_t view_count, render_stats_t *stats) {
	render_item_t *run_start = nullptr;
	for (size_t i = 0; i < items.count; i++) {
		render_item_t *item = &items[i];
		
		// Skip this item if it's filtered out
		if ((item->layer & filter) == 0) continue;

		// If it's the first in the run, record the material/mesh
		if (run_start == nullptr) {
			run_start = item;
		}
		// If the material/mesh changed
		else if (run_start->material != item->material || run_start->mesh != item->mesh) {
			// Render the run that just ended
			_render_instanced_list_run(run_start->material, run_start->mesh, run_start->mesh_inds, view_count, stats);
			render_instance_list.clear();
			// Start the next run
			run_start = item;
		}

		// Add the current item to the run of instances
		XMMATRIX transpose = XMMatrixTranspose(item->transform);
		render_instance_list.add(render_transform_buffer_t{ transpose, item->color });
	}
	// Render the last remaining run, which won't be triggered by the loop's
	// conditions
	if (render_instance_list.count > 0) {
		_render_instanced_list_run(run_start->material, run_start->mesh, run_start->mesh_inds, view_count, stats);
		render_instance_list.clear();
	}
}

///////////////////////////////////////////

void render_instanced_list_material(const array_t<render_item_t> items, render_layer_ filter, uint32_t view_count, material_t override_material, render_stats_t *stats) {
	render_item_t *run_start = nullptr;
	for (size_t i = 0; i < items.count; i++) {
		render_item_t *item = &items[i];

		// Skip this item if it's filtered out
		if ((item->layer & filter) == 0) continue;

		// If it's the first in the run, record the material/mesh
		if (run_start == nullptr) {
			run_start = item;
		}
		// If the mesh changed
		else if (run_start->mesh != item->mesh) {
			// Render the run that just ended
			_render_instanced_list_run(override_material, run_start->mesh, run_start->mesh_inds, view_count, stats);
			render_instance_list.clear();
			// Start the next run
			run_start = item;
		}

		// Add the current item to the run of instances
		XMMATRIX transpose = XMMatrixTranspose(item->transform);
		render_instance_list.add(render_transform_buffer_t{ transpose, item->color });
	}
	// Render the last remaining run, which won't be triggered by the loop's
	// conditions
	if (render_instance_list.count > 0) {
		_render_instanced_list_run(override_material, run_start->mesh, run_start->mesh_inds, view_count, stats);
		render_instance_list.clear();
	}
}

} // namespace sk
