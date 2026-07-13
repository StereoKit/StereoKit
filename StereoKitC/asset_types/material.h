#pragma once

#include "../stereokit.h"
#include <sk_renderer.h>
#include "assets.h"

namespace sk {

struct _material_t {
	asset_header_t    header;
	shader_t          shader;
	skr_material_t    gpu_mat;

	// Cached state - most pipeline state now in gpu_mat.key (single source of truth)
	transparency_     alpha_mode;  // Higher-level abstraction mapping to blend_state + alpha_to_coverage

	// Name-keyed [[vk::constant_id]] overrides, re-fed through every pipeline
	// rebuild. Names point into the shader meta, which outlives the material.
	skr_spec_constant_t spec_overrides[SKR_MAX_SPEC_CONSTANTS];
	int32_t             spec_override_count;

	// Per-resource tracking — single allocation, pointers index into it.
	// Layout: [tex_t * N][uint64_t * N][compute_buffer_t * N]
	tex_t*            textures;            // owns the allocation
	uint64_t*         texture_meta_hashes;
	compute_buffer_t* buffers;
	int32_t           resource_count;

	material_t        chain;
	material_t        variants[3];
};

struct _material_buffer_t {
	asset_header_t header;
	int32_t        size;
	skr_buffer_t   buffer;
};

void   material_buffer_destroy (material_buffer_t buffer);
void   material_destroy        (material_t material);
void   material_check_dirty    (material_t material);
tex_t  material_get_default_tex(material_t material, const char *name);
size_t material_param_size     (material_param_ type);

} // namespace sk