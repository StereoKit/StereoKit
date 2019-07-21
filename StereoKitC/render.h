#pragma once

#include "stereokit.h"

void render_draw       ();
void render_draw_from  (camera_t &cam, transform_t &cam_transform);
void render_draw_matrix(const float *cam_matrix, transform_t &cam_transform);
void render_clear();

void render_initialize();
void render_shutdown();