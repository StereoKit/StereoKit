#pragma once

#include "../stereokit.h"
#include "assets.h"
#include "shader.h"

struct _material_t {
	asset_header_t    header;
	shader_t          shader;
	shaderargs_data_t args;
	material_alpha_   mode;
	material_cull_    cull;
	int32_t           queue_offset;
	ID3D11BlendState      *blend_state;
	ID3D11RasterizerState *rasterizer_state;
};

void material_destroy(material_t material);