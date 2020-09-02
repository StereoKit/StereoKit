#pragma once

#include "../stereokit.h"
#include "../libraries/sk_gpu_inc.h"
#include "assets.h"
#include "shader.h"

namespace sk {

struct _material_t {
	asset_header_t    header;
	shader_t          shader;
	shaderargs_data_t args;
	transparency_     alpha_mode;
	cull_             cull;
	bool32_t          wireframe;
	int32_t           queue_offset;
	skr_pipeline_t    pipeline;
};

void   material_destroy   (material_t material);
size_t material_param_size(material_param_ type);

} // namespace sk