#include "render.h"
#include "render_sort.h"
#include "../libraries/sk_gpu.h"
#include "../libraries/stref.h"
#include "../sk_math.h"
#include "../spherical_harmonics.h"
#include "../stereokit.h"
#include "../hierarchy.h"
#include "../asset_types/mesh.h"
#include "../asset_types/texture.h"
#include "../asset_types/shader.h"
#include "../asset_types/material.h"
#include "../asset_types/model.h"
#include "../systems/input.h"
#include "../systems/platform/platform_utils.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../libraries/stb_image_write.h"

#include <DirectXMath.h> // Matrix math functions and objects
using namespace DirectX;

namespace sk {

///////////////////////////////////////////

struct render_transform_buffer_t {
	XMMATRIX world;
	color128 color;
	uint32_t view_id;
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
	float    time;
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
	char *filename;
	vec3  from;
	vec3  at;
	int width;
	int height;
};

///////////////////////////////////////////

array_t<render_transform_buffer_t> render_instance_list = {};
render_inst_buffer                 render_instance_buffers[] = { { 1 }, { 5 }, { 10 }, { 20 }, { 50 }, { 100 }, { 250 }, { 500 }, { 682 } };

skg_buffer_t           render_shader_globals;
skg_buffer_t           render_shader_blit;
matrix                 render_camera_root     = matrix_identity;
matrix                 render_camera_root_inv = matrix_identity;
matrix                 render_default_camera_proj;
vec2                   render_clip_planes = {0.01f, 50};
float                  render_fov         = 90;
render_global_buffer_t render_global_buffer;
mesh_t                 render_blit_quad;
tex_t                  render_default_tex;
vec4                   render_lighting[9] = {};
color128               render_clear_color = {0,0,0,1};
render_list_t          render_list_primary = -1;

array_t<render_screenshot_t>  render_screenshot_list = {};

mesh_t     render_sky_mesh    = nullptr;
material_t render_sky_mat     = nullptr;
tex_t      render_sky_cubemap = nullptr;
bool32_t   render_sky_show    = false;

material_t render_last_material;
shader_t   render_last_shader;
mesh_t     render_last_mesh;

array_t< render_list_t> render_list_stack  = {};
array_t<_render_list_t> render_lists       = {};
render_list_t           render_list_active = -1;
skg_bind_t              render_list_global_bind = { 1,  skg_stage_vertex | skg_stage_pixel };
skg_bind_t              render_list_inst_bind   = { 2,  skg_stage_vertex | skg_stage_pixel };
skg_bind_t              render_list_blit_bind   = { 2,  skg_stage_vertex | skg_stage_pixel };
skg_bind_t              render_list_sky_bind    = { 11, skg_stage_pixel };

///////////////////////////////////////////

void render_set_material(material_t material);

///////////////////////////////////////////

skg_buffer_t *render_fill_inst_buffer(array_t<render_transform_buffer_t> &list, size_t &offset, size_t &out_count);
void          render_check_screenshots();

///////////////////////////////////////////

inline uint64_t render_queue_id(material_t material, mesh_t mesh) {
	return ((uint64_t)(material->alpha_mode*1000 + material->queue_offset) << 32) | (material->header.index << 16) | mesh->header.index;
}

///////////////////////////////////////////

void render_set_clip(float near_plane, float far_plane) {
	// near_plane will throw divide by zero errors if it's zero! So we'll clamp it :)
	near_plane = fmaxf(0.0001f, near_plane);
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
	case tex_format_rgba64:        return "rgba64";
	case tex_format_rgba128:       return "rgba128";
	case tex_format_r8:            return "r8";
	case tex_format_r16:           return "r16";
	case tex_format_r32:           return "r32";
	case tex_format_depthstencil:  return "depth24_stencil8";
	case tex_format_depth32:       return "depth32";
	case tex_format_depth16:       return "depth16";
	default:                       return "Unknown";
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

void render_set_cam_root(const matrix &cam_root) {
	render_camera_root = cam_root;
	matrix_inverse(render_camera_root, render_camera_root_inv);
}

///////////////////////////////////////////

void render_set_skytex(tex_t sky_texture) {
	if (sky_texture != nullptr && !(sky_texture->type & tex_type_cubemap)) {
		log_err("render_set_skytex: Attempting to set the skybox texture to a texture that's not a cubemap! Sorry, but cubemaps only here please!");
		return;
	}

	if (render_sky_cubemap != nullptr)
		tex_release(render_sky_cubemap);

	render_sky_cubemap = sky_texture;
	if (render_sky_cubemap == nullptr)
		return;

	assets_addref(render_sky_cubemap->header);
}

///////////////////////////////////////////

tex_t render_get_skytex() {
	return render_sky_cubemap;
}

///////////////////////////////////////////

void render_set_skylight(const spherical_harmonics_t &light_info) {
	sh_to_fast(light_info, render_lighting);
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

void render_set_clear_color(color128 color) {
	render_clear_color = color;
}

//////////////////////////////////////////

color128 render_get_clear_color() {
	return render_clear_color;
}

///////////////////////////////////////////

void render_add_mesh(mesh_t mesh, material_t material, const matrix &transform, color128 color) {
	render_item_t item;
	item.mesh     = &mesh->gpu_mesh;
	item.mesh_inds= mesh->ind_draw;
	item.material = material;
	item.color    = color;
	item.sort_id  = render_queue_id(material, mesh);
	if (hierarchy_enabled) {
		matrix_mul(transform, hierarchy_stack.last().transform, item.transform);
	} else {
		math_matrix_to_fast(transform, &item.transform);
	}
	render_list_add(&item);
}

///////////////////////////////////////////

void render_add_model(model_t model, const matrix &transform, color128 color) {
	XMMATRIX root;
	if (hierarchy_enabled) {
		matrix_mul(transform, hierarchy_stack.last().transform, root);
	} else {
		math_matrix_to_fast(transform, &root);
	}

	for (int i = 0; i < model->subset_count; i++) {
		render_item_t item;
		item.mesh     = &model->subsets[i].mesh->gpu_mesh;
		item.mesh_inds= model->subsets[i].mesh->ind_count;
		item.material = model->subsets[i].material;
		item.color    = color;
		item.sort_id  = render_queue_id(item.material, model->subsets[i].mesh);
		matrix_mul(model->subsets[i].offset, root, item.transform);
		render_list_add(&item);
	}
}

///////////////////////////////////////////

void render_draw_queue(const matrix *views, const matrix *projections, int32_t view_count) {
	// Copy camera information into the global buffer
	for (int32_t i = 0; i < view_count; i++) {
		XMMATRIX view_f, projection_f;
		math_matrix_to_fast(views      [i], &view_f      );
		math_matrix_to_fast(projections[i], &projection_f);

		XMMATRIX view_inv = XMMatrixInverse(nullptr, view_f      );
		XMMATRIX proj_inv = XMMatrixInverse(nullptr, projection_f);

		XMVECTOR cam_pos = XMVector3Transform(DirectX::g_XMIdentityR3, view_inv);
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
	render_global_buffer.time = time_getf();
	vec3 tip = input_hand(handed_right)->tracked_state & button_state_active ? input_hand(handed_right)->fingers[1][4].position : vec3{0,-1000,0};
	render_global_buffer.fingertip[0] = { tip.x, tip.y, tip.z, 0 };
	tip = input_hand(handed_left)->tracked_state & button_state_active ? input_hand(handed_left)->fingers[1][4].position : vec3{0,-1000,0};
	render_global_buffer.fingertip[1] = { tip.x, tip.y, tip.z, 0 };

	// Upload shader globals and set them active!
	skg_buffer_set_contents(&render_shader_globals, &render_global_buffer, sizeof(render_global_buffer_t));
	skg_buffer_bind        (&render_shader_globals, render_list_global_bind, 0);

	// Sky cubemap is global, and used for reflections with PBR materials
	if (render_sky_cubemap != nullptr) {
		skg_tex_bind(&render_sky_cubemap->tex, render_list_sky_bind);
	}

	render_list_execute(render_list_primary, view_count);
}

///////////////////////////////////////////

void render_draw_matrix(const matrix* views, const matrix* projections, int32_t count) {
	render_draw_queue(views, projections, count);
	render_check_screenshots();
}

///////////////////////////////////////////

void render_check_screenshots() {
	skg_tex_t *old_target = skg_tex_target_get();
	for (size_t i = 0; i < render_screenshot_list.count; i++) {
		int32_t  w = render_screenshot_list[i].width;
		int32_t  h = render_screenshot_list[i].height;

		matrix view = matrix_trs(
			render_screenshot_list[i].from,
			quat_lookat(render_screenshot_list[i].from, render_screenshot_list[i].at));
		matrix_inverse(view, view);

		matrix proj;
		math_fast_to_matrix(XMMatrixPerspectiveFovRH(
			render_fov * deg2rad, 
			(float)w/h, 
			render_clip_planes.x, 
			render_clip_planes.y), &proj);

		// Create the screenshot surface
		
		size_t   size   = sizeof(color32) * w * h;
		color32 *buffer = (color32*)malloc(size);
		tex_t    render_capture_surface = tex_create(tex_type_image_nomips | tex_type_rendertarget);
		tex_set_colors (render_capture_surface, w, h, buffer);
		tex_add_zbuffer(render_capture_surface);

		// Setup to render the screenshot
		float color[4] = {
			render_clear_color.r / 255.f,
			render_clear_color.g / 255.f,
			render_clear_color.b / 255.f,
			render_clear_color.a / 255.f };
		skg_tex_target_bind(&render_capture_surface->tex, true, color);
		
		// Render!
		render_draw_queue(&view, &proj, 1);
		skg_tex_target_bind(nullptr, false, nullptr);
		
		// And save the screenshot to file
		tex_get_data(render_capture_surface, buffer, size);
		stbi_write_jpg(render_screenshot_list[i].filename, w, h, 4, buffer, 85);
		free(buffer);
		free(render_screenshot_list[i].filename);
	}
	render_screenshot_list.clear();
	skg_tex_target_bind(old_target, false, nullptr);
}

///////////////////////////////////////////

void render_clear() {
	//log_infof("draws: %d, instances: %d, material: %d, shader: %d, texture %d, mesh %d", render_stats.draw_calls, render_stats.draw_instances, render_stats.swaps_material, render_stats.swaps_shader, render_stats.swaps_texture, render_stats.swaps_mesh);
	render_list_clear(render_list_active);

	render_last_material = nullptr;
	render_last_shader   = nullptr;
	render_last_mesh     = nullptr;
}

///////////////////////////////////////////

bool render_init() {
	render_shader_globals = skg_buffer_create(nullptr, 1, sizeof(render_global_buffer_t), skg_buffer_type_constant, skg_use_dynamic);
	render_shader_blit    = skg_buffer_create(nullptr, 1, sizeof(render_blit_data_t),     skg_buffer_type_constant, skg_use_dynamic);;

	for (size_t i = 0; i < _countof(render_instance_buffers); i++) {
		render_instance_buffers[i].buffer = skg_buffer_create(nullptr, render_instance_buffers[i].max, sizeof(render_transform_buffer_t), skg_buffer_type_constant, skg_use_dynamic);
	}

	// Setup a default camera
	render_set_clip(render_clip_planes.x, render_clip_planes.y);

	// Set up resources for doing blit operations
	render_blit_quad = mesh_find(default_id_mesh_quad);
	assets_addref(render_blit_quad->header);

	// Create a default skybox
	render_sky_mesh = mesh_create();
	vind_t inds [] = {2,1,0, 3,2,0};
	vert_t verts[] = {
		vert_t{ {-1, 1,1}, {0,0,1}, {0,0}, {255,255,255,255} },
		vert_t{ { 1, 1,1}, {0,0,1}, {0,0}, {255,255,255,255} },
		vert_t{ { 1,-1,1}, {0,0,1}, {0,0}, {255,255,255,255} },
		vert_t{ {-1,-1,1}, {0,0,1}, {0,0}, {255,255,255,255} }, };
	mesh_set_inds (render_sky_mesh, inds,  _countof(inds));
	mesh_set_verts(render_sky_mesh, verts, _countof(verts));
	mesh_set_id   (render_sky_mesh, "render/skybox_mesh");

	render_sky_mat  = material_create(shader_find(default_id_shader_sky));
	material_set_id          (render_sky_mat, "render/skybox_material");
	material_set_queue_offset(render_sky_mat, 100);

	render_default_tex = tex_find(default_id_tex);
	render_list_primary = render_list_create();
	render_list_push(render_list_primary);

	return true;
}

///////////////////////////////////////////

void render_update() {
	if (hierarchy_stack.count > 0)
		log_err("Render transform stack doesn't have matching begin/end calls!");

	if (render_sky_show && sk_system_info().display_type == display_opaque) {
		render_add_mesh(render_sky_mesh, render_sky_mat, matrix_identity);
	}
}

///////////////////////////////////////////

void render_shutdown() {
	for (int32_t i = 0; i < render_lists.count; i++) {
		render_list_release(i);
	}
	render_lists.free();
	render_list_stack.free();
	render_screenshot_list.free();
	render_instance_list.free();

	material_release(render_sky_mat);
	mesh_release    (render_sky_mesh);
	tex_release     (render_sky_cubemap);
	tex_release     (render_default_tex);

	for (size_t i = 0; i < _countof(render_instance_buffers); i++) {
		skg_buffer_destroy(&render_instance_buffers[i].buffer);
	}

	skg_buffer_destroy(&render_shader_blit);
	skg_buffer_destroy(&render_shader_globals);
	mesh_release(render_blit_quad);
}

///////////////////////////////////////////

void render_blit(tex_t to, material_t material) {
	// Wipe our swapchain color and depth target clean, and then set them up for rendering!
	skg_tex_t *old_target = skg_tex_target_get();
	float      color[4]   = { 0,0,0,0 };
	skg_tex_target_bind(&to->tex, true, color);

	// Setup shader args for the blit operation
	render_blit_data_t data = {};
	data.width  = (float)to->tex.width;
	data.height = (float)to->tex.height;
	data.pixel_width  = 1.0f / to->tex.width;
	data.pixel_height = 1.0f / to->tex.height;

	// Setup render states for blitting
	skg_buffer_set_contents(&render_shader_blit, &data, sizeof(render_blit_data_t));
	skg_buffer_bind        (&render_shader_blit, render_list_blit_bind, 0);
	render_set_material(material);
	skg_mesh_bind(&render_blit_quad->gpu_mesh);
	
	// And draw to it!
	skg_draw(0, 0, render_blit_quad->ind_count, 1);

	skg_tex_target_bind(old_target, false, nullptr);

	render_last_material = nullptr;
	render_last_mesh     = nullptr;
	render_last_shader   = nullptr;
}

///////////////////////////////////////////

void render_screenshot(vec3 from_viewpt, vec3 at, int width, int height, const char *file) {
	char *file_copy = string_copy(file);
	render_screenshot_list.add( render_screenshot_t{ file_copy, from_viewpt, at, width, height });
}

///////////////////////////////////////////

void render_set_material(material_t material) {
	if (material == render_last_material)
		return;
	render_last_material = material;
	render_lists[render_list_active].stats.swaps_material++;

	// Update and bind the material parameter buffer
	if (material->args.buffer != nullptr) {
		if (material->args.buffer_dirty) {
			skg_buffer_set_contents(&material->args.buffer_gpu, material->args.buffer, (uint32_t)material->args.buffer_size);
			material->args.buffer_dirty = false;
		}
		skg_buffer_bind(&material->args.buffer_gpu, material->args.buffer_bind, 0);
	}

	// Bind the material textures
	for (size_t i = 0; i < material->args.texture_count; i++) {
		if (material->args.texture_binds[i].slot != render_list_sky_bind.slot)
			skg_tex_bind(&material->args.textures[i]->tex, material->args.texture_binds[i]);
	}

	// And bind the pipeline
	skg_pipeline_bind(&material->pipeline);
}

///////////////////////////////////////////

skg_buffer_t *render_fill_inst_buffer(array_t<render_transform_buffer_t> &list, int32_t &offset, int32_t &out_count) {
	// Find a buffer that can contain this list! Or the biggest one
	int32_t size  = (int32_t)list.count - offset;
	int32_t index = 0;
	for (int32_t i = 0; i < _countof(render_instance_buffers); i++) {
		index = i;
		if (render_instance_buffers[i].max >= size)
			break;
	}
	int32_t start = offset;

	// Check if it fits, if it doesn't, then set up data so we only fill what we have!
	if (size > render_instance_buffers[index].max) {
		offset   += render_instance_buffers[index].max;
		out_count = render_instance_buffers[index].max;
	} else {
		// this means we've gotten through the whole list :)
		offset    = 0;
		out_count = size;
	}

	// Copy data into the buffer, and return it!
	skg_buffer_set_contents(&render_instance_buffers[index].buffer, &list[start], sizeof(render_transform_buffer_t) * out_count);
	return &render_instance_buffers[index].buffer;
}

///////////////////////////////////////////

vec3 render_unproject_pt(vec3 normalized_screen_pt) {
	XMMATRIX fast_proj, fast_view;
	math_matrix_to_fast(render_get_projection(), &fast_proj);
	math_matrix_to_fast(render_camera_root_inv,  &fast_view);
	XMVECTOR result = XMVector3Unproject(math_vec3_to_fast(normalized_screen_pt),
		0, 0, sk_system_info().display_width, sk_system_info().display_height,
		0, 1,
		fast_proj, fast_view, XMMatrixIdentity());
		
	return math_fast_to_vec3(result);
}

///////////////////////////////////////////

void render_get_device(void **device, void **context) {
	log_warn("render_get_device not implemented for sk_gpu!");
	*device  = nullptr; //d3d_device;
	*context = nullptr; //d3d_context;
}

///////////////////////////////////////////
// Render List                           //
///////////////////////////////////////////

render_list_t render_list_create() {
	int64_t id = render_lists.index_where(&_render_list_t::state, render_list_state_destroyed);
	if (id == -1)
		id = render_lists.add({});
	return id;
}

///////////////////////////////////////////

void render_list_release(render_list_t list) {
	render_lists[list].queue.free();
	render_lists[list] = {};
	render_lists[list].state = render_list_state_destroyed;
}

///////////////////////////////////////////

void render_list_push(render_list_t list) {
	render_list_active = render_list_stack.add(list);
	render_lists[list].state = render_list_state_used;
}

///////////////////////////////////////////

void render_list_pop() {
	render_list_stack.pop();
	render_list_active = render_list_stack.count - 1;
}

///////////////////////////////////////////

void render_list_add(const render_item_t *item) {
	render_lists[render_list_active].queue.add(*item);
}

///////////////////////////////////////////

void render_list_add_to(render_list_t list, const render_item_t *item) {
	render_lists[list].queue.add(*item);
}

///////////////////////////////////////////

void render_list_execute(render_list_t list_id, uint32_t surface_count) {
	_render_list_t *list = &render_lists[list_id];
	list->state = render_list_state_rendering;

	size_t count = list->queue.count;
	if (count == 0) {
		list->state = render_list_state_rendered;
		return;
	}
	if (!list->sorted) {
		radix_sort7(&list->queue[0], count);
		list->sorted = true;
	}

	render_item_t *item          = &list->queue[0];
	material_t     last_material = item->material;
	skg_mesh_t    *last_mesh     = item->mesh;

	for (size_t i = 0; i < count; i++) {
		XMMATRIX transpose = XMMatrixTranspose(item->transform);

		// Add a render instance for each display surface
		for (uint32_t s = 0; s < surface_count; s++) {
			render_instance_list.add(render_transform_buffer_t { transpose, item->color, s });
		}

		// If the next item is not the same as the current run of render 
		// items, we'll collect the instances, and submit them to draw!
		render_item_t *next = i+1>=count ? nullptr : &list->queue[i+1];
		if (next == nullptr || last_material != next->material || last_mesh != next->mesh) {
			render_set_material(item->material);
			skg_mesh_bind      (item->mesh);
			list->stats.swaps_mesh++;

			// Collect and draw instances
			int32_t offsets = 0, count = 0;
			do {
				skg_buffer_t *instances = render_fill_inst_buffer(render_instance_list, offsets, count);
				skg_buffer_bind(instances, render_list_inst_bind, 0);

				skg_draw(0, 0, item->mesh_inds, count);
				list->stats.draw_calls     += 1;
				list->stats.draw_instances += count;

			} while (offsets != 0);
			render_instance_list.clear();

			// Update the material/mesh info, so we know when to draw next
			if (next != nullptr) {
				last_material = next->material;
				last_mesh     = next->mesh;
			}
		}
		item = next;
	}

	list->state = render_list_state_rendered;
}

///////////////////////////////////////////

void render_list_clear(render_list_t list) {
	render_lists[list].queue.clear();
	render_lists[list].stats  = {};
	render_lists[list].sorted = false;
	render_lists[list].state  = render_list_state_empty;
}

} // namespace sk