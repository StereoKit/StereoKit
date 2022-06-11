#pragma once

#include "../stereokit.h"

namespace sk {

void        tex_set_zbuffer      (tex_t texture, tex_t depth_texture);
void        tex_set_options      (tex_t texture, tex_sample_ sample = tex_sample_linear, tex_address_ address_mode = tex_address_wrap, int32_t anisotropy_level = 4);
void        tex_set_surface      (tex_t texture, void *native_surface, tex_type_ type, int64_t native_fmt, int32_t width, int32_t height, int32_t surface_count);
void        tex_set_surface_layer(tex_t texture, void *native_surface, tex_type_ type, int64_t native_fmt, int32_t width, int32_t height, int32_t surface_index);
size_t      tex_format_size      (tex_format_ format);
tex_format_ tex_get_tex_format   (int64_t native_fmt);
void        tex_set_meta         (tex_t texture, int32_t width, int32_t height, tex_format_ format);
uint64_t    tex_meta_hash        (tex_t texture);

} // namespace sk