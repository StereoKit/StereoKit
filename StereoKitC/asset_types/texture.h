#pragma once

#include <d3d11.h>

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
	int array_size;
	int width;
	int height;
	ID3D11SamplerState       *sampler;
	ID3D11ShaderResourceView *resource;
	ID3D11RenderTargetView   *target_view;
	ID3D11DepthStencilView   *depth_view;
	ID3D11Texture2D          *texture;
	tex_t                     depth_buffer;
};

void        tex_set_active       (tex_t texture, int slot);
void        tex_destroy          (tex_t texture);
DXGI_FORMAT tex_get_native_format(tex_format_ format);
tex_format_ tex_get_tex_format   (DXGI_FORMAT format);
size_t      tex_format_size      (tex_format_ format);

void tex_releasesurface(tex_t texture);
void tex_setsurface    (tex_t texture, ID3D11Texture2D *source, DXGI_FORMAT source_format);
void tex_set_zbuffer   (tex_t texture, tex_t depth_texture);
bool tex_create_surface(tex_t texture, void **data, int32_t data_count, spherical_harmonics_t *sh_lighting_info);
bool tex_create_views  (tex_t texture, DXGI_FORMAT source_format, bool create_shader_view);
void tex_set_options   (tex_t texture, tex_sample_ sample = tex_sample_linear, tex_address_ address_mode = tex_address_wrap, int32_t anisotropy_level = 4);

bool tex_downsample    (color32  *data, int32_t width, int32_t height, color32  **out_data, int32_t *out_width, int32_t *out_height);
bool tex_downsample_128(color128 *data, int32_t width, int32_t height, color128 **out_data, int32_t *out_width, int32_t *out_height);

} // namespace sk