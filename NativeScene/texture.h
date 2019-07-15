#pragma once

#include "d3d.h"

#include <stdint.h>

struct tex2d_t {
	int width;
	int height;
	ID3D11ShaderResourceView *resource;
	ID3D11Texture2D          *texture;
};

void tex2d_create     (tex2d_t &tex);
bool tex2d_create_file(tex2d_t &tex, const char *file);
void tex2d_set_colors (tex2d_t &tex, int width, int height, uint8_t *data_rgba32);
void tex2d_set_active (tex2d_t &tex, int slot);