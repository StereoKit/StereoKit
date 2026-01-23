#pragma once

#include <sk_renderer.h>

#include "../stereokit.h"
#include "assets.h"

namespace sk {

struct _tex_t {
	asset_header_t   header;
	tex_t            fallback;
	bool32_t         owned;

	// Metadata fields - kept for quick CPU access
	// TODO: can get rid of some maybe?
	int32_t          width;
	int32_t          height;
	tex_format_      format;
	uint64_t         meta_hash;

	tex_type_        type;
	tex_sample_      sample_mode;
	tex_sample_comp_ sample_comp;
	tex_address_     address_mode;
	int32_t          anisotropy;
	skr_tex_t        gpu_tex;
	tex_t            depth_buffer;
	spherical_harmonics_t *light_info;
};

void tex_destroy(tex_t texture);

} // namespace sk