#pragma once

#include "../stereokit.h"

namespace sk {

struct render_stats_t {
	int swaps_mesh;
	int swaps_shader;
	int swaps_texture;
	int swaps_material;
	int draw_calls;
};

matrix render_get_projection();
color32 render_get_clear_color();
vec2 render_get_clip();
void render_draw_matrix (const matrix *views, const matrix *projs, int32_t view_count);
void render_clear       ();
vec3 render_unproject_pt(vec3 normalized_screen_pt);
void render_update_projection();

bool render_initialize();
void render_update();
void render_shutdown();

void render_set_material(material_t material);
void render_set_shader  (shader_t   shader);
void render_set_mesh    (mesh_t     mesh);
void render_draw_item   (int count);

} // namespace sk