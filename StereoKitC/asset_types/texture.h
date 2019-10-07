#pragma once

#include <d3d11.h>

#include "../stereokit.h"
#include "assets.h"

namespace sk {

struct _tex2d_t {
	asset_header_t header;
	tex_format_    format;
	tex_type_      type;
	int width;
	int height;
	ID3D11SamplerState       *sampler;
	ID3D11ShaderResourceView *resource;
	ID3D11RenderTargetView   *target_view;
	ID3D11DepthStencilView   *depth_view;
	ID3D11Texture2D          *texture;
	tex2d_t                   depth_buffer;
};

tex2d_t     tex2d_create_mem(void *data, size_t data_size);
void        tex2d_set_active(tex2d_t texture, int slot);
void        tex2d_destroy   (tex2d_t texture);
DXGI_FORMAT tex2d_get_native_format(tex_format_ format);
size_t      tex2d_format_size      (tex_format_ format);
void        tex2d_set_colors(tex2d_t texture, int32_t width, int32_t height, void **data, int32_t data_count);

void tex2d_releasesurface(tex2d_t texture);
void tex2d_setsurface    (tex2d_t texture, ID3D11Texture2D *source);
bool tex2d_create_surface(tex2d_t texture, void **data, int32_t data_count);
bool tex2d_create_views  (tex2d_t texture);

bool tex2d_downsample(color32 *data, int32_t width, int32_t height, color32 **out_data, int32_t *out_width, int32_t *out_height);

} // namespace sk