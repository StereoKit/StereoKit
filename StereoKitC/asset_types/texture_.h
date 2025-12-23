#pragma once

#include "../stereokit.h"

namespace sk {

void        tex_set_zbuffer      (tex_t texture, tex_t depth_texture);
void        tex_set_options      (tex_t texture, tex_sample_ sample = tex_sample_linear, tex_address_ address_mode = tex_address_wrap, tex_sample_comp_ compare = tex_sample_comp_none, int32_t anisotropy_level = 4);
size_t      tex_format_size      (tex_format_ format, int32_t width, int32_t height);
tex_format_ tex_get_tex_format   (int64_t native_fmt);
int64_t     tex_fmt_to_native    (tex_format_ format);
void        tex_set_meta         (tex_t texture, int32_t width, int32_t height, tex_format_ format);
uint64_t    tex_meta_hash        (tex_t texture);

uint8_t* unzip_malloc(const uint8_t* buffer, int32_t len, int32_t* out_len);

} // namespace sk