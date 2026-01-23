#include "render_pipeline.h"
#include "render.h"
#include "../asset_types/texture.h"
#include "../libraries/profiler.h"

#include <sk_renderer.h>
#include <stdio.h>

namespace sk {

///////////////////////////////////////////

struct pipeline_surface_t {
	tex_format_                 color;
	tex_format_                 depth;
	render_layer_               layer;
	float                       viewport_scale;
	pipeline_render_strategy_   strategy;
	int32_t                     array_count;
	int32_t                     quilt_width;
	int32_t                     quilt_height;
	int32_t                     multisample;
	color128                    clear_color;
	tex_t                       tex;
	matrix*                     view_matrices;
	matrix*                     proj_matrices;
	bool32_t                    enabled;
	skr_tex_t*                  resolve_target;
};

struct render_pipeline_state_t {
	array_t<pipeline_surface_t> surfaces;
};
static render_pipeline_state_t local = {};

///////////////////////////////////////////

static void render_pipeline_begin() {
	render_check_pending_skytex();
	render_action_list_execute();
	render_check_screenshots();
}

///////////////////////////////////////////

void render_pipeline_draw() {
	render_pipeline_begin();

	render_list_t list = render_get_primary_list();

	for (int32_t i = 0; i < local.surfaces.count; i++) {
		pipeline_surface_t* s = &local.surfaces[i];
		if (s->enabled == false) continue;

		int32_t width  = (int32_t)fmaxf(1, (float)(s->tex->width  / s->quilt_width ) * s->viewport_scale);
		int32_t height = (int32_t)fmaxf(1, (float)(s->tex->height / s->quilt_height) * s->viewport_scale);

		// Get depth buffer if available
		tex_t      depth_surface = s->tex->depth_buffer;
		skr_tex_t* depth_tex     = depth_surface ? &depth_surface->gpu_tex : nullptr;

		// Set up clear values
		skr_vec4_t clear_color = { s->clear_color.r, s->clear_color.g, s->clear_color.b, s->clear_color.a };
		skr_clear_ clear_flags = (skr_clear_)(skr_clear_color | skr_clear_depth | skr_clear_stencil);

		if (s->strategy == pipeline_render_strategy_sequential) {
			for (int32_t layer = 0; layer < s->array_count; layer++) {
				// Begin render pass for this layer
				// TODO: sk_renderer may need array layer support in begin_pass
				skr_renderer_begin_pass(&s->tex->gpu_tex, depth_tex, s->resolve_target, clear_flags, clear_color, 1.0f, 0);

				for (int32_t quilt_y = 0; quilt_y < s->quilt_height; quilt_y += 1) {
					for (int32_t quilt_x = 0; quilt_x < s->quilt_width; quilt_x += 1) {
						skr_rect_t  viewport = { (float)(quilt_x * width), (float)(quilt_y * height), (float)width, (float)height };
						skr_recti_t scissor  = { quilt_x * width, quilt_y * height, width, height };
						skr_renderer_set_viewport(viewport);
						skr_renderer_set_scissor (scissor);

						int32_t idx = quilt_x + quilt_y * s->quilt_width + layer * s->quilt_width * s->quilt_height;
						render_draw_queue(list, s->view_matrices, s->proj_matrices, idx, 1, 1, s->layer, 0);
					}
				}

				skr_renderer_end_pass();
			}
		} else if (
			s->strategy == pipeline_render_strategy_simultaneous ||
			s->strategy == pipeline_render_strategy_multiview) {

			// Begin render pass for all layers
			skr_renderer_begin_pass(&s->tex->gpu_tex, depth_tex, s->resolve_target, clear_flags, clear_color, 1.0f, 0);

			// Regular simultaneous array textures draw one inst per
			// `view`, and multiview draws one inst `view` times.
			int32_t inst_multiplier = s->strategy == pipeline_render_strategy_simultaneous
				? s->array_count
				: 1;

			for (int32_t quilt_y = 0; quilt_y < s->quilt_height; quilt_y += 1) {
			for (int32_t quilt_x = 0; quilt_x < s->quilt_width;  quilt_x += 1) {
				skr_rect_t  viewport = { (float)(quilt_x * width), (float)(quilt_y * height), (float)width, (float)height };
				skr_recti_t scissor  = { quilt_x * width, quilt_y * height, width, height };
				skr_renderer_set_viewport(viewport);
				skr_renderer_set_scissor (scissor);

				int32_t idx = quilt_x + quilt_y * s->quilt_width;
				render_draw_queue(list, s->view_matrices, s->proj_matrices, idx, s->array_count, inst_multiplier, s->layer, 0);
			} }

			skr_renderer_end_pass();
		}
	}

	render_list_clear  (list);
	render_list_release(list);
}

///////////////////////////////////////////

void render_pipeline_shutdown() {
	for (int32_t i = 0; i < local.surfaces.count; i++)
		render_pipeline_surface_destroy(i);
	local.surfaces.free();
	local = {};
}

///////////////////////////////////////////

void render_pipeline_begin_frame() {
	skr_renderer_frame_begin();
}

///////////////////////////////////////////

void render_pipeline_skip_present() {
	// End the frame without presenting to any swapchain surface.
	// Used by OpenXR which manages its own swapchains externally.
	skr_renderer_frame_end(nullptr, 0);
}

///////////////////////////////////////////

pipeline_surface_id render_pipeline_surface_create(pipeline_render_strategy_ strategy, tex_format_ color, tex_format_ depth, int32_t array_count, int32_t quilt_width, int32_t quilt_height) {
	pipeline_surface_t result = {};
	result.enabled        = false; // shouldn't be enabled until the tex is sized
	result.color          = color;
	result.depth          = depth;
	result.layer          = render_layer_all;
	result.viewport_scale = 1;
	result.array_count    = array_count;
	result.quilt_width    = quilt_width;
	result.quilt_height   = quilt_height;
	result.strategy       = strategy;
	result.view_matrices  = sk_malloc_t(matrix, array_count * quilt_width * quilt_height);
	result.proj_matrices  = sk_malloc_t(matrix, array_count * quilt_width * quilt_height);
	return local.surfaces.add(result);
}

///////////////////////////////////////////

void render_pipeline_surface_destroy(pipeline_surface_id surface_id) {
	pipeline_surface_t* surface = &local.surfaces[surface_id];
	surface->enabled = false;
	sk_free(surface->view_matrices);
	sk_free(surface->proj_matrices);
	tex_release(surface->tex);
	*surface = {};
}

///////////////////////////////////////////

bool32_t render_pipeline_surface_resize(pipeline_surface_id surface_id, int32_t width, int32_t height, int32_t multisample) {
	pipeline_surface_t *surface = &local.surfaces[surface_id];

	// If this is the first time getting called, the texture will be null, and
	// we'll need to create a fresh new one.
	if (surface->tex == nullptr) {
		log_diagf("Creating target surface: <~grn>%d<~clr>x<~grn>%d<~clr>x<~grn>%d<~clr>@<~grn>%d<~clr>msaa", width, height, surface->array_count, multisample);

		surface->tex         = tex_create(tex_type_image_nomips | tex_type_rendertarget, surface->color);
		surface->multisample = multisample;
		surface->enabled     = true;
		tex_set_color_arr(surface->tex, width, height, nullptr, surface->array_count, multisample, nullptr);

		char name[64];
		snprintf(name, sizeof(name), "sk/render/pipeline_surface_%d", surface_id);
		tex_set_id(surface->tex, name);

		tex_add_zbuffer(surface->tex, surface->depth);
		tex_t zbuffer = tex_get_zbuffer(surface->tex);
		snprintf(name, sizeof(name), "sk/render/pipeline_surface_%d_depth", surface_id);
		tex_set_id (zbuffer, name);
		tex_release(zbuffer);
		return true;
	}

	// If the surface is the same size, like the OS is sending multiple resize
	// commands at the same dimensions, we don't need to do anything more.
	if (width == tex_get_width(surface->tex) && height == tex_get_height(surface->tex) && surface->multisample == multisample)
		return false;

	log_diagf("Resizing target surface: <~grn>%d<~clr>x<~grn>%d<~clr>x<~grn>%d<~clr>@<~grn>%d<~clr>msaa", width, height, surface->array_count, multisample);
	surface->multisample = multisample;
	tex_set_color_arr(surface->tex, width, height, nullptr, surface->array_count, multisample, nullptr);

	return true;
}

///////////////////////////////////////////

skr_acquire_ render_pipeline_surface_acquire_swapchain(pipeline_surface_id surface_id, skr_surface_t* skr_surface) {
	pipeline_surface_t* surface = &local.surfaces[surface_id];

	// Acquire the next swapchain image
	skr_tex_t*   target         = nullptr;
	skr_acquire_ acquire_result = skr_surface_next_tex(skr_surface, &target);

	if (acquire_result == skr_acquire_success && target) {
		// Set the swapchain image as the MSAA resolve target
		// The render pass will automatically resolve to this during end_pass
		surface->resolve_target = target;
	} else {
		surface->resolve_target = nullptr;

		if (acquire_result == skr_acquire_needs_resize) {
			skr_surface_resize(skr_surface);
		}
	}

	return acquire_result;
}

///////////////////////////////////////////

void render_pipeline_surface_present_swapchain(pipeline_surface_id surface_id, skr_surface_t* skr_surface) {
	pipeline_surface_t* surface = &local.surfaces[surface_id];

	if (surface->resolve_target) {
		// End frame with surface synchronization
		skr_surface_t* surfaces[] = { skr_surface };
		skr_renderer_frame_end(surfaces, 1);

		// Present to the screen
		{
			profiler_zone_name("VSync");
			skr_surface_present(skr_surface);
		}
	} else {
		// Failed to acquire earlier - still need to end frame
		render_pipeline_skip_present();
	}

	// Clear resolve target for next frame
	surface->resolve_target = nullptr;
}

///////////////////////////////////////////

void render_pipeline_surface_to_tex(pipeline_surface_id surface_id, tex_t destination, material_t mat) {
	pipeline_surface_t* surface = &local.surfaces[surface_id];

	if (mat) {
		material_set_texture(mat, "source", surface->tex);
		render_blit(destination, mat);
	} else {
		// Copy all array layers
		uint32_t src_layers = surface->tex->gpu_tex.layer_count;
		uint32_t dst_layers = destination->gpu_tex.layer_count;
		uint32_t layer_count = src_layers < dst_layers ? src_layers : dst_layers;
		skr_tex_copy(&surface->tex->gpu_tex, &destination->gpu_tex, 0, 0, 0, 0, layer_count);
	}
}

///////////////////////////////////////////

void render_pipeline_surface_get_surface_info(pipeline_surface_id surface_id, int32_t view_idx, int32_t *out_array_idx, int32_t *out_xywh_rect) {
	pipeline_surface_t* surface = &local.surfaces[surface_id];

	int32_t array_step      = surface->quilt_width * surface->quilt_height;
	int32_t arr             = view_idx / array_step;
	int32_t array_based_idx = (view_idx - (arr * array_step));

	int32_t y = array_based_idx / surface->quilt_width;
	int32_t x = array_based_idx % surface->quilt_width;

	*out_array_idx = arr;
	out_xywh_rect[2] = surface->tex ? (int32_t)fmaxf(1, (float)(surface->tex->width  / surface->quilt_width ) * surface->viewport_scale) : 0;
	out_xywh_rect[3] = surface->tex ? (int32_t)fmaxf(1, (float)(surface->tex->height / surface->quilt_height) * surface->viewport_scale) : 0;
	out_xywh_rect[0] = x * out_xywh_rect[2];
	out_xywh_rect[1] = y * out_xywh_rect[3];
}

///////////////////////////////////////////

void render_pipeline_surface_set_tex(pipeline_surface_id surface_id, tex_t tex) {
	pipeline_surface_t* surface = &local.surfaces[surface_id];
	if (tex)          tex_addref (tex);
	if (surface->tex) tex_release(surface->tex);
	surface->tex = tex;
}

///////////////////////////////////////////

tex_t render_pipeline_surface_get_tex(pipeline_surface_id surface_id) {
	pipeline_surface_t* surface = &local.surfaces[surface_id];
	if (surface->tex) tex_addref(surface->tex);
	return surface->tex;
}

///////////////////////////////////////////

void render_pipeline_surface_set_enabled(pipeline_surface_id surface, bool32_t enabled) {
	local.surfaces[surface].enabled = enabled;
}

///////////////////////////////////////////

bool32_t render_pipeline_surface_get_enabled(pipeline_surface_id surface) {
	return local.surfaces[surface].enabled;
}

///////////////////////////////////////////

void render_pipeline_surface_set_layer(pipeline_surface_id surface, render_layer_ layer) {
	local.surfaces[surface].layer = layer;
}

///////////////////////////////////////////

void render_pipeline_surface_set_viewport_scale(pipeline_surface_id surface, float viewport_rect_scale) {
	local.surfaces[surface].viewport_scale = viewport_rect_scale;
}

///////////////////////////////////////////

void render_pipeline_surface_set_clear(pipeline_surface_id surface, color128 color) {
	local.surfaces[surface].clear_color = color;
}

///////////////////////////////////////////

void render_pipeline_surface_set_perspective(pipeline_surface_id surface_id, matrix* view_matrices, matrix* proj_matrices, int32_t count) {
	pipeline_surface_t* surface = &local.surfaces[surface_id];
	if (count != surface->array_count*surface->quilt_width*surface->quilt_height) log_err("Surface count mismatch.");

	memcpy(surface->view_matrices, view_matrices, sizeof(matrix) * count);
	memcpy(surface->proj_matrices, proj_matrices, sizeof(matrix) * count);
}

///////////////////////////////////////////

void render_pipeline_surface_set_resolve_target(pipeline_surface_id surface_id, skr_tex_t* resolve_target) {
	local.surfaces[surface_id].resolve_target = resolve_target;
}

}