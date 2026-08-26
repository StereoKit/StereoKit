#include "render_pipeline.h"
#include "render.h"
#include "lighting.h"
#include "defaults.h"
#include "../asset_types/texture.h"
#include "../asset_types/material.h"
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
	int32_t                     array_count;
	int32_t                     multisample;
	int32_t                     width;
	int32_t                     height;
	color128                    clear_color;
	bool32_t                    clear_covered; // Draw fills every pixel, clear is skippable
	tex_t                       tex;       // Null draws into resolve_target
	tex_t                       depth_tex; // Used when tex is null
	matrix*                     view_matrices;
	matrix*                     proj_matrices;
	bool32_t                    enabled;
	skr_tex_t*                  resolve_target;
	tex_t                       present_tex;    // Stands in for resolve_target when sizes differ
	material_t                  present_mat;    // Stretches present_tex over resolve_target
	bool32_t                    present_active; // Is present_tex in use this frame?
};

struct render_pipeline_state_t {
	array_t<pipeline_surface_t> surfaces;
};
static render_pipeline_state_t local = {};

///////////////////////////////////////////

static void render_pipeline_begin() {
	lighting_check_pending();
	render_action_list_execute();
	render_check_screenshots();
}

///////////////////////////////////////////

void render_pipeline_draw() {
	profiler_zone();
	render_pipeline_begin();

	render_list_t list = render_get_primary_list();

	for (int32_t i = 0; i < local.surfaces.count; i++) {
		pipeline_surface_t* s = &local.surfaces[i];
		if (s->enabled == false) continue;

		skr_tex_t* resolve_dest = s->present_active ? &s->present_tex->gpu_tex : s->resolve_target;

		// Drawing direct also means there's nothing left to resolve.
		skr_tex_t* color_tex = s->tex ? &s->tex->gpu_tex : resolve_dest;
		if (color_tex == nullptr) continue;

		int32_t width  = (int32_t)fmaxf(1, (float)s->width  * s->viewport_scale);
		int32_t height = (int32_t)fmaxf(1, (float)s->height * s->viewport_scale);

		// Get depth buffer if available
		tex_t      depth_surface = s->tex ? s->tex->depth_buffer : s->depth_tex;
		skr_tex_t* depth_tex     = depth_surface ? &depth_surface->gpu_tex : nullptr;

		// Set up clear values and submit the pass - the pass system handles
		// multi-view fallback automatically via the view_desc.
		// A fully covered surface overwrites every pixel, so the clear is
		// wasted work. Passthrough covers none, and there the clear _is_ the
		// content.
		skr_vec4_t clear_color = { s->clear_color.r, s->clear_color.g, s->clear_color.b, s->clear_color.a };
		skr_clear_ clear_flags = (skr_clear_)((s->clear_covered ? skr_clear_color_discard : skr_clear_color) | skr_clear_depth | skr_clear_stencil);

		render_draw_queue(list, s->view_matrices, s->proj_matrices, 0, s->array_count, s->layer, 0, width, height);

		skr_pass_t pass = {};
		pass.color            = color_tex;
		pass.depth            = depth_tex;
		pass.resolve          = s->tex ? resolve_dest : nullptr;
		pass.clear            = clear_flags;
		pass.clear_color      = clear_color;
		pass.clear_depth      = 1.0f;
		pass.viewport         = { 0, 0, (float)width, (float)height };
		pass.scissor          = { 0, 0, width, height };
		pass.view_count       = s->array_count;
		pass.views_correlated = s->array_count > 1;
		render_pass_add_draw(&pass);
		render_pass_add_global_post_process(&pass);
		skr_pass_submit(&pass);

		// Only the viewport corner of present_tex was drawn, and it stretches
		// to fill the swapchain image.
		if (s->present_active && s->resolve_target) {
			// uv_clamp pins far-edge samples to the last drawn texel's center,
			// so linear filtering can't read the undrawn region past the
			// viewport. At full size it matches the sampler's own edge clamp,
			// so it needs no special case.
			skr_vec3i_t dst = skr_tex_get_size(s->resolve_target);
			material_set_vector2(s->present_mat, "uv_scale", vec2{ (float)width /  s->width, (float)height /  s->height });
			material_set_vector2(s->present_mat, "uv_clamp", vec2{ (width-0.5f) / s->width, (height-0.5f) / s->height });
			material_check_dirty(s->present_mat);
			skr_renderer_blit(&s->present_mat->gpu_mat, s->resolve_target, skr_recti_t{ 0, 0, dst.x, dst.y });
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
	profiler_zone();
	// End the frame without presenting to any swapchain surface.
	// Used by OpenXR which manages its own swapchains externally.
	skr_renderer_frame_end(nullptr, 0);
}

///////////////////////////////////////////

pipeline_surface_id render_pipeline_surface_create(tex_format_ color, tex_format_ depth, int32_t array_count) {
	pipeline_surface_t result = {};
	result.enabled        = false; // shouldn't be enabled until the tex is sized
	result.color          = color;
	result.depth          = depth;
	result.layer          = render_layer_all;
	result.viewport_scale = 1;
	result.array_count    = array_count;
	result.view_matrices  = sk_malloc_t(matrix, array_count);
	result.proj_matrices  = sk_malloc_t(matrix, array_count);
	return local.surfaces.add(result);
}

///////////////////////////////////////////

void render_pipeline_surface_destroy(pipeline_surface_id surface_id) {
	pipeline_surface_t* surface = &local.surfaces[surface_id];
	surface->enabled = false;
	sk_free(surface->view_matrices);
	sk_free(surface->proj_matrices);
	tex_release     (surface->tex);
	tex_release     (surface->depth_tex);
	tex_release     (surface->present_tex);
	material_release(surface->present_mat);
	*surface = {};
}

///////////////////////////////////////////

bool32_t render_pipeline_surface_resize(pipeline_surface_id surface_id, int32_t width, int32_t height, int32_t multisample) {
	pipeline_surface_t *surface = &local.surfaces[surface_id];

	// The zbuffer has to match the color tex's sample count, so a multisample
	// change rebuilds both from scratch rather than trying to retarget them.
	if (surface->tex != nullptr && surface->multisample != multisample) {
		tex_release(surface->tex);
		surface->tex = nullptr;
	}

	// At 1x there's no intermediate to allocate, just a depth buffer to pair
	// with the swapchain image.
	bool    needs_tex = multisample > 1;
	bool    has_tex   = needs_tex ? surface->tex != nullptr : surface->depth_tex != nullptr;
	char    name[64];

	// If the surface is the same, like the OS is sending multiple resize
	// commands at the same dimensions, we don't need to do anything more.
	if (has_tex && width == surface->width && height == surface->height && surface->multisample == multisample)
		return false;

	log_diagf("%s target surface: <~grn>%d<~clr>x<~grn>%d<~clr>x<~grn>%d<~clr>@<~grn>%d<~clr>msaa", has_tex ? "Resizing" : "Creating", width, height, surface->array_count, multisample);

	// Only the very first sizing enables the surface, backends own the flag
	// after that.
	if (surface->width == 0) surface->enabled = true;
	surface->width       = width;
	surface->height      = height;
	surface->multisample = multisample;

	if (needs_tex) {
		// Release the 1x depth first, it shares an id with the zbuffer below.
		tex_release(surface->depth_tex);
		surface->depth_tex = nullptr;

		bool fresh = surface->tex == nullptr;
		if (fresh) {
			// needs_tex is the "will be resolved" condition, so this tex is
			// only ever an MSAA intermediate. Nothing samples or copies it.
			surface->tex = tex_create(tex_type_image_nomips | tex_type_rendertarget | tex_type_transient_internal, surface->color);
			snprintf(name, sizeof(name), "sk/render/pipeline_surface_%d", surface_id);
			tex_set_id(surface->tex, name);
		}
		tex_set_color_arr(surface->tex, width, height, nullptr, surface->array_count, multisample);

		// An existing zbuffer already tracks the color tex, and already has
		// the id below. Naming it a second time would collide with itself.
		if (fresh) {
			tex_add_zbuffer(surface->tex, surface->depth);
			tex_t zbuffer = tex_get_zbuffer(surface->tex);
			snprintf(name, sizeof(name), "sk/render/pipeline_surface_%d_depth", surface_id);
			tex_set_id (zbuffer, name);
			tex_release(zbuffer);
		}
	} else {
		tex_release(surface->tex);
		surface->tex = nullptr;

		if (surface->depth_tex == nullptr) {
			surface->depth_tex = tex_create(tex_type_zbuffer, surface->depth);
			snprintf(name, sizeof(name), "sk/render/pipeline_surface_%d_depth", surface_id);
			tex_set_id(surface->depth_tex, name);
		}
		tex_set_color_arr(surface->depth_tex, width, height, nullptr, surface->array_count, 1);
	}

	render_update_projection();
	return true;
}

///////////////////////////////////////////

// Direct rendering into the swapchain image only works when the surface fills
// it exactly. Anything else needs an intermediate for the present blit. A
// surface that never needs one never allocates it, and one that does keeps it
// warm, since these settings tend to get swept back and forth across the
// boundary.
static void render_pipeline_surface_update_present(pipeline_surface_id surface_id, skr_tex_t* swapchain_tex) {
	pipeline_surface_t* surface = &local.surfaces[surface_id];

	skr_vec3i_t size = skr_tex_get_size(swapchain_tex);
	surface->present_active = size.x != surface->width || size.y != surface->height || surface->viewport_scale < 1;
	if (surface->present_active == false) return;

	if (surface->present_mat == nullptr)
		surface->present_mat = material_create(sk_default_shader_blit);

	bool fresh = surface->present_tex == nullptr;
	if (fresh) {
		char name[64];
		surface->present_tex = tex_create(tex_type_image_nomips | tex_type_rendertarget, surface->color);
		snprintf(name, sizeof(name), "sk/render/pipeline_present_%d", surface_id);
		tex_set_id     (surface->present_tex, name);
		tex_set_sample (surface->present_tex, tex_sample_linear);
		tex_set_address(surface->present_tex, tex_address_clamp);
	}
	// A resize swaps the GPU texture out from under the binding, but that
	// changes the tex's meta hash, so material_check_dirty rebinds it.
	if (surface->present_tex->width != surface->width || surface->present_tex->height != surface->height)
		tex_set_color_arr(surface->present_tex, surface->width, surface->height, nullptr, surface->array_count, 1);
	if (fresh)
		material_set_texture(surface->present_mat, "source", surface->present_tex);
}

///////////////////////////////////////////

skr_acquire_ render_pipeline_surface_acquire_swapchain(pipeline_surface_id surface_id, skr_surface_t* skr_surface, skr_vec2i_t size) {
	profiler_zone();
	pipeline_surface_t* surface = &local.surfaces[surface_id];

	// Acquire the next swapchain image
	skr_tex_t*   target         = nullptr;
	skr_acquire_ acquire_result = skr_surface_next_tex(skr_surface, size, &target);

	if (acquire_result == skr_acquire_success && target) {
		// Set the swapchain image as the MSAA resolve target
		// The render pass will automatically resolve to this during end_pass
		surface->resolve_target = target;
		render_pipeline_surface_update_present(surface_id, target);
	} else {
		surface->resolve_target = nullptr;
	}

	return acquire_result;
}

///////////////////////////////////////////

void render_pipeline_surface_present_swapchain(pipeline_surface_id surface_id, skr_surface_t* skr_surface) {
	profiler_zone();
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

	// A multisampled surface renders into a transient MSAA intermediate that
	// can't be sampled or copied, only the direct-render tex is readable.
	if (surface->tex == nullptr || (surface->tex->type & tex_type_transient_internal)) {
		log_err("render_pipeline_surface_to_tex: this surface has no readable texture, read its resolve target instead");
		return;
	}

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

	*out_array_idx   = view_idx;
	out_xywh_rect[0] = 0;
	out_xywh_rect[1] = 0;
	out_xywh_rect[2] = surface->width  > 0 ? (int32_t)fmaxf(1, (float)surface->width  * surface->viewport_scale) : 0;
	out_xywh_rect[3] = surface->height > 0 ? (int32_t)fmaxf(1, (float)surface->height * surface->viewport_scale) : 0;
}

///////////////////////////////////////////

void render_pipeline_surface_set_tex(pipeline_surface_id surface_id, tex_t tex) {
	pipeline_surface_t* surface = &local.surfaces[surface_id];
	if (tex)          tex_addref (tex);
	if (surface->tex) tex_release(surface->tex);
	surface->tex = tex;

	// An externally provided tex brings its own size, sample count, and depth
	// along with it. Callers that use this don't also call resize.
	if (tex) {
		surface->width       = tex->width;
		surface->height      = tex->height;
		surface->multisample = skr_tex_get_multisample(&tex->gpu_tex);
		tex_release(surface->depth_tex);
		surface->depth_tex = nullptr;
	}
}

///////////////////////////////////////////

tex_t render_pipeline_surface_get_tex(pipeline_surface_id surface_id) {
	pipeline_surface_t* surface = &local.surfaces[surface_id];

	// A multisampled surface's tex is a transient that can't be sampled or
	// copied, so handing it out is never useful. Same as _to_tex below.
	if (surface->tex && (surface->tex->type & tex_type_transient_internal)) {
		log_err("render_pipeline_surface_get_tex: this surface has no readable texture, read its resolve target instead");
		return nullptr;
	}

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

void render_pipeline_surface_set_clear(pipeline_surface_id surface, color128 color, bool32_t fully_covered) {
	local.surfaces[surface].clear_color   = color;
	local.surfaces[surface].clear_covered = fully_covered;
}

///////////////////////////////////////////

void render_pipeline_surface_set_perspective(pipeline_surface_id surface_id, matrix* view_matrices, matrix* proj_matrices, int32_t count) {
	pipeline_surface_t* surface = &local.surfaces[surface_id];
	if (count != surface->array_count) log_err("Surface count mismatch.");

	memcpy(surface->view_matrices, view_matrices, sizeof(matrix) * count);
	memcpy(surface->proj_matrices, proj_matrices, sizeof(matrix) * count);
}

///////////////////////////////////////////

void render_pipeline_surface_set_resolve_target(pipeline_surface_id surface_id, skr_tex_t* resolve_target) {
	local.surfaces[surface_id].resolve_target = resolve_target;
}

}