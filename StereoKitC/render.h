#pragma once

#include "stereokit.h"

struct render_stats_t {
	int swaps_mesh;
	int swaps_shader;
	int swaps_texture;
	int swaps_material;
	int draw_calls;
};

void render_draw       ();
void render_draw_from  (camera_t &cam, transform_t &cam_transform);
void render_draw_matrix(const float *cam_matrix, transform_t &cam_transform);
void render_clear();

void render_initialize();
void render_shutdown();

// don't like this function much
void render_get_cam(camera_t **cam, transform_t **cam_transform);

void render_set_material(material_t material);
void render_set_shader  (shader_t   shader);
void render_set_texture (tex2d_t    texture, int slot);
void render_set_mesh    (mesh_t     mesh);
void render_draw_item   (int count);