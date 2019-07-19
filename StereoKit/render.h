#pragma once

#include "material.h"
#include "mesh.h"
#include "transform.h"
#include "camera.h"

void render_set_camera(camera_t &cam, transform_t &cam_transform);
void render_add(mesh_t &mesh, material_t &material, transform_t &transform);
void render_draw();
void render_draw_from(camera_t &cam, transform_t &cam_transform);
void render_draw_matrix(const float *cam_matrix, transform_t &cam_transform);
void render_clear();

void render_initialize();
void render_shutdown();