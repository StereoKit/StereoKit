#pragma once

#include "stereokit.h"
#include "assets.h"

#include "shader.h"

struct _material_t {
	asset_header_t    header;
	shader_t          shader;
	shaderargs_data_t args;
	material_alpha_   mode;
	ID3D11BlendState *blend_state;
};

void material_destroy(material_t material);