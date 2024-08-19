#include "render_pipeline.h"
#include "render.h"
#include "../asset_types/texture.h"

#include <sk_gpu.h>
#include <stdio.h>

namespace sk {

///////////////////////////////////////////

struct pipeline_surface_t {
	tex_format_   color;
	tex_format_   depth;
	render_layer_ layer;
	float         viewport_scale;
	int32_t       array_count;
	int32_t       quilt_width;
	int32_t       quilt_height;
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

	render_list_t list = render_get_primary_list();

	for (int32_t i = 0; i < local.surfaces.count; i++) {
		pipeline_surface_t* s = &local.surfaces[i];
		if (s->enabled == false) continue;

		int32_t width  = (int32_t)fmaxf(1, (float)(s->tex->width  / s->quilt_width ) * s->viewport_scale);
		int32_t height = (int32_t)fmaxf(1, (float)(s->tex->height / s->quilt_height) * s->viewport_scale);

		skg_event_begin("Draw Surface");
		{
#if defined(SKG_DIRECT3D11)
			skg_tex_target_bind(&s->tex->tex, -1, 0);
			skg_target_clear   (true, &s->clear_color.r);
				
			for (int32_t quilt_y = 0; quilt_y < s->quilt_height; quilt_y += 1) {
			for (int32_t quilt_x = 0; quilt_x < s->quilt_width;  quilt_x += 1) {
				int32_t viewport[4] = {quilt_x*width, quilt_y*height, width, height };
				skg_viewport(viewport);

				int32_t idx = quilt_x + quilt_y * s->quilt_width;
				render_draw_queue(list, s->view_matrices, s->proj_matrices, idx, s->array_count, s->layer);
			} }
#elif defined (SKG_OPENGL)
			for (int32_t layer = 0; layer < s->array_count; layer++) {
				skg_tex_target_bind(&s->tex->tex, layer, 0);
				skg_target_clear(true, &s->clear_color.r);

				for (int32_t quilt_y = 0; quilt_y < s->quilt_height; quilt_y += 1) {
				for (int32_t quilt_x = 0; quilt_x < s->quilt_width;  quilt_x += 1) {
					int32_t viewport[4] = { quilt_x * width, quilt_y * height, width, height };
					skg_viewport(viewport);

					int32_t idx = quilt_x + quilt_y * s->quilt_width + layer * s->quilt_width * s->quilt_height;
					render_draw_queue(list, s->view_matrices, s->proj_matrices, idx, 1, s->layer);
				} }
			}
#else
#pragma error
#endif
		}
		skg_event_end();
	}

	render_list_clear  (list);
	render_list_release(list);

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

pipeline_surface_id render_pipeline_surface_create(tex_format_ color, tex_format_ depth, int32_t array_count, int32_t quilt_width, int32_t quilt_height) {
	pipeline_surface_t result = {};
	result.enabled        = false; // shouldn't be enabled until the tex is sized
	result.color          = color;
	result.depth          = depth;
	result.layer          = render_layer_all;
	result.viewport_scale = 1;
	result.array_count    = array_count;
	result.quilt_width    = quilt_width;
	result.quilt_height   = quilt_height;
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
		tex_set_color_arr(surface->tex, width, height, nullptr, surface->array_count, nullptr, multisample);

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
	tex_set_color_arr(surface->tex, width, height, nullptr, surface->array_count, nullptr, multisample);

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

void render_pipeline_surface_to_tex(pipeline_surface_id surface_id, tex_t destination, material_t mat) {
	pipeline_surface_t* surface = &local.surfaces[surface_id];

	skg_event_begin("Present to Tex");
	{
		if (mat) {
			material_set_texture(mat, "source", surface->tex);
			render_blit(destination, mat);
		} else {
			skg_tex_copy_to(&surface->tex->tex, -1, &destination->tex, -1);
		}
	}
	skg_event_end();
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

}