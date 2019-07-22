#pragma once

#include <d3d11.h>

#include "stereokit.h"
#include "assets.h"

struct _tex2d_t {
	asset_header_t header;
	int width;
	int height;
	ID3D11ShaderResourceView *resource;
	ID3D11Texture2D          *texture;
};

void tex2d_set_active(tex2d_t tex, int slot);
void tex2d_destroy   (tex2d_t tex);