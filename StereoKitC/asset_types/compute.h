#pragma once

#include <sk_renderer.h>
#include "../stereokit.h"
#include "assets.h"

namespace sk {

struct _compute_t {
	asset_header_t    header;
	shader_t          shader;
	skr_compute_t     gpu_compute;

	tex_t*            textures;
	int32_t           texture_count;
	compute_buffer_t* buffers;
	int32_t           buffer_count;

	// Name-keyed [[vk::constant_id]] overrides, re-fed through every pipeline
	// rebuild. Names point into the shader meta, which outlives the compute.
	skr_spec_constant_t spec_overrides[SKR_MAX_SPEC_CONSTANTS];
	int32_t             spec_override_count;
};

void compute_destroy(compute_t compute);

} // namespace sk
