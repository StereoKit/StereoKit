#pragma once

#include "../stereokit.h"
#include "../libraries/sk_gpu.h"

namespace sk {

enum render_pipeline_surface_type_ {
	render_pipeline_surface_type_swapchain,
	render_pipeline_surface_type_texture,
};
struct render_pipeline_surface_t {
	render_pipeline_surface_type_ type;
	union {
		skg_tex_t*       target;
		skg_swapchain_t* swapchain;
	};
};

typedef int32_t pipeline_surface_id;

pipeline_surface_id render_pipeline_surface_create         (tex_format_ color, tex_format_ depth, int32_t surface_count);
void                render_pipeline_surface_destroy        (pipeline_surface_id surface);
bool32_t            render_pipeline_surface_resize         (pipeline_surface_id surface, int32_t width, int32_t height, int32_t multisample);
void                render_pipeline_surface_to_swapchain   (pipeline_surface_id surface, skg_swapchain_t* swapchain);
void                render_pipeline_surface_set_tex        (pipeline_surface_id surface, tex_t tex);
tex_t               render_pipeline_surface_get_tex        (pipeline_surface_id surface);
void                render_pipeline_surface_set_enabled    (pipeline_surface_id surface, bool32_t enabled);
void                render_pipeline_surface_set_layer      (pipeline_surface_id surface, render_layer_ layer);
void                render_pipeline_surface_set_clear      (pipeline_surface_id surface, color128 color);
void                render_pipeline_surface_set_perspective(pipeline_surface_id surface, matrix* view_matrices, matrix* proj_matrices, int32_t count);

void render_pipeline_shutdown();

void render_pipeline_begin();
void render_pipeline_draw ();
void render_pipeline_end  ();

} // namespace sk