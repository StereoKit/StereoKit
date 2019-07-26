#pragma once

#include "stereokit.h"
#include "assets.h"

#include "shader.h"
#include "texture.h"

struct _material_t {
	asset_header_t    header;
	shader_t          shader;
	shaderargs_data_t args;
};

void material_destroy   (material_t material);
void material_set_active(material_t material);