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

void render_draw        ();
void render_draw_matrix (const matrix &view, const matrix &proj);
void render_clear       ();
vec3 render_unproject_pt(vec3 normalized_screen_pt);

bool render_initialize();
void render_update();
void render_shutdown();

void render_set_material(material_t material);
void render_set_shader  (shader_t   shader);
void render_set_texture (tex2d_t    texture, int slot);
void render_set_mesh    (mesh_t     mesh);
void render_draw_item   (int count);

} // namespace sk