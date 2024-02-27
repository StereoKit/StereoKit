#include "render_pipeline.h"
#include "render.h"
#include "../asset_types/texture.h"
#include "../libraries/sk_gpu.h"

#include <stdio.h>

namespace sk {

///////////////////////////////////////////

struct pipeline_surface_t {
	tex_format_   color;
	tex_format_   depth;
	render_layer_ layer;
	int32_t       surface_count;
	int32_t       multisample;
	color128      clear_color;
	tex_t         tex;
	matrix       *view_matrices;
	matrix       *proj_matrices;
	bool32_t      enabled;
};

struct render_pipeline_state_t {
	array_t<pipeline_surface_t> surfaces;
	bool32_t                    begin_called;
};
static render_pipeline_state_t local = {};

///////////////////////////////////////////

void render_pipeline_begin() {
	skg_event_begin("Setup");
	{
		skg_draw_begin();
	}
	skg_event_end();
	skg_event_begin("Offscreen");
	{
		render_check_viewpoints();
		render_check_screenshots();
	}
	skg_event_end();
	local.begin_called = true;
}

///////////////////////////////////////////

void render_pipeline_draw() {
	if (!local.begin_called)
		render_pipeline_begin();

	for (int32_t i = 0; i < local.surfaces.count; i++) {
		pipeline_surface_t* s = &local.surfaces[i];
		if (s->enabled == false) continue;

		skg_event_begin("Draw Surface");
		{
			skg_tex_target_bind(&s->tex->tex);
			skg_target_clear   (true, &s->clear_color.r);
			render_draw_matrix (s->view_matrices, s->proj_matrices, i, s->surface_count, s->layer);
		}
		skg_event_end();
	}

	render_clear();
	local.begin_called = false;
}

///////////////////////////////////////////

void render_pipeline_shutdown() {
	for (int32_t i = 0; i < local.surfaces.count; i++)
		render_pipeline_surface_destroy(i);
	local.surfaces.free();
	local = {};
}

///////////////////////////////////////////

pipeline_surface_id render_pipeline_surface_create(tex_format_ color, tex_format_ depth, int32_t surface_count) {
	pipeline_surface_t result = {};
	result.enabled       = false; // shouldn't be enabled until the tex is sized
	result.color         = color;
	result.depth         = depth;
	result.layer         = render_layer_all;
	result.surface_count = surface_count;
	result.view_matrices = sk_malloc_t(matrix, surface_count);
	result.proj_matrices = sk_malloc_t(matrix, surface_count);
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
		log_diagf("Creating target surface: <~grn>%d<~clr>x<~grn>%d<~clr>x<~grn>%d<~clr>@<~grn>%d<~clr>msaa", width, height, surface->surface_count, multisample);

		surface->tex         = tex_create(tex_type_image_nomips | tex_type_rendertarget, surface->color);
		surface->multisample = multisample;
		surface->enabled     = true;
		tex_set_color_arr(surface->tex, width, height, nullptr, surface->surface_count, nullptr, multisample);

		char name[64];
		snprintf(name, sizeof(name), "sk/pipeline_surface_%d", surface_id);
		tex_set_id(surface->tex, name);

		tex_add_zbuffer(surface->tex, surface->depth);
		tex_t zbuffer = tex_get_zbuffer(surface->tex);
		snprintf(name, sizeof(name), "sk/pipeline_surface_%d_depth", surface_id);
		tex_set_id(zbuffer, name);
		tex_release(zbuffer);
		return true;
	}

	// If the surface is the same size, like the OS is sending multiple resize
	// commands at the same dimensions, we don't need to do anything more.
	if (width == tex_get_width(surface->tex) && height == tex_get_height(surface->tex) && surface->multisample == multisample)
		return false;

	log_diagf("Resizing target surface: <~grn>%d<~clr>x<~grn>%d<~clr>x<~grn>%d<~clr>@<~grn>%d<~clr>msaa", width, height, surface->surface_count, multisample);
	surface->multisample = multisample;
	tex_set_color_arr(surface->tex, width, height, nullptr, surface->surface_count, nullptr, multisample);

	return true;
}

///////////////////////////////////////////

void render_pipeline_surface_to_swapchain(pipeline_surface_id surface_id, skg_swapchain_t* swapchain) {
	pipeline_surface_t* surface = &local.surfaces[surface_id];

	skg_event_begin("Present");
	{
		// This copies the color data over to the swapchain, and resolves any
		// multisampling on the primary target texture.
		skg_tex_copy_to_swapchain(&surface->tex->tex, swapchain);

		// Present to the screen
		skg_swapchain_present(swapchain);
	}
	skg_event_end();
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

void render_pipeline_surface_set_layer(pipeline_surface_id surface, render_layer_ layer) {
	local.surfaces[surface].layer = layer;
}

///////////////////////////////////////////

void render_pipeline_surface_set_clear(pipeline_surface_id surface, color128 color) {
	local.surfaces[surface].clear_color = color;
}

///////////////////////////////////////////

void render_pipeline_surface_set_perspective(pipeline_surface_id surface_id, matrix* view_matrices, matrix* proj_matrices, int32_t count) {
	pipeline_surface_t* surface = &local.surfaces[surface_id];
	if (count != surface->surface_count) log_err("Surface count mismatch.");

	memcpy(surface->view_matrices, view_matrices, sizeof(matrix) * count);
	memcpy(surface->proj_matrices, proj_matrices, sizeof(matrix) * count);
}

}