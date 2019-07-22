#pragma once

#include "stereokit.h"
#include "assets.h"

#include "shader.h"
#include "texture.h"

struct _material_t {
	asset_header_t header;
	shader_t       shader;
	tex2d_t        texture;
};

void material_destroy(material_t material);