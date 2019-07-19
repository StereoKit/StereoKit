#pragma once

#include "d3d.h"

struct shader_t {
	ID3D11VertexShader *vshader;
	ID3D11PixelShader  *pshader;
	ID3D11InputLayout  *vert_layout;
};
struct shaderargs_t {
	ID3D11Buffer *const_buffer;
	int buffer_slot;
	int buffer_size;
};

void shader_create     (shader_t &shader, const char *hlsl);
bool shader_create_file(shader_t &shader, const char *filename);
void shader_destroy    (shader_t &shader);
void shader_set_active (shader_t &shader);

void shaderargs_create    (shaderargs_t &args, size_t buffer_size, int buffer_slot);
void shaderargs_destroy   (shaderargs_t &args);
void shaderargs_set_data  (shaderargs_t &args, void *data);
void shaderargs_set_active(shaderargs_t &args);