#pragma once

#include "../libraries/sk_gpu_inc.h"

#include "../stereokit.h"
#include "assets.h"

namespace sk {

struct _tex_t {
	asset_header_t header;
	tex_format_    format;
	tex_type_      type;
	tex_sample_    sample_mode;
	tex_address_   address_mode;
	int32_t        anisotropy;
	skr_tex_t      tex;
	tex_t          depth_buffer;
};

void        tex_destroy          (tex_t texture);
size_t      tex_format_size      (tex_format_ format);

void tex_set_zbuffer(tex_t texture, tex_t depth_texture);
void tex_set_options(tex_t texture, tex_sample_ sample = tex_sample_linear, tex_address_ address_mode = tex_address_wrap, int32_t anisotropy_level = 4);

} // namespace sk