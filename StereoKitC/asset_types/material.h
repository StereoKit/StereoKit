#pragma once

#include "../stereokit.h"
#include <sk_renderer.h>
#include "assets.h"

namespace sk {

struct _material_t {
	asset_header_t    header;
	shader_t          shader;
	skr_material_t    gpu_mat;

	// Cached state for API compatibility and recreating gpu_mat on state change
	transparency_     alpha_mode;
	cull_             cull;
	depth_test_       depth_test;
	bool32_t          depth_write;
	bool32_t          depth_clip;
	int32_t           queue_offset;

	// Texture references for proper lifetime management
	// sk_renderer manages the GPU bindings, but we need to track SK texture refs
	tex_t*            textures;
	uint64_t*         texture_meta_hashes; // Cached hashes to detect texture changes
	int32_t           texture_count;

	material_t        chain;
	material_t        variants[3];
};

struct _material_buffer_t {
	int32_t      refs;
	int32_t      size;
	skr_buffer_t buffer;
};

void   material_destroy    (material_t material);
void   material_check_dirty(material_t material);
size_t material_param_size (material_param_ type);

} // namespace sk