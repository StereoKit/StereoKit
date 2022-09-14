#pragma once

#include "../libraries/sk_gpu.h"

#include "../stereokit.h"
#include "assets.h"

namespace sk {

struct _tex_t {
	asset_header_t header;
	tex_t          fallback;
	bool32_t       owned;

	// Metadata fields
	int32_t        width;
	int32_t        height;
	tex_format_    format;
	uint64_t       meta_hash;

	tex_type_      type;
	tex_sample_    sample_mode;
	tex_address_   address_mode;
	int32_t        anisotropy;
	skg_tex_t      tex;
	tex_t          depth_buffer;
	spherical_harmonics_t *light_info;
};

void tex_destroy(tex_t texture);

} // namespace sk