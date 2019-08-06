#include "stereokit.h"
#include "texture.h"

#include "d3d.h"

#pragma warning( disable : 26451 6011 6262 6308 6387 28182 )
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma warning( default : 26451 6011 6262 6308 6387 28182 )

tex2d_t tex2d_create(const char *id, tex_type_ type, tex_format_ format) {
	tex2d_t result = (tex2d_t)assets_allocate(asset_type_texture, id);
	result->type   = type;
	result->format = format;
	return result;
}
void tex2d_add_zbuffer(tex2d_t texture, tex_format_ format) {
	if (!(texture->type & tex_type_rendertarget)) {
		log_write(log_error, "Can't add a zbuffer to a non-rendertarget texture!");
		return;
	}

	char id[64];
	assets_unique_name("zbuffer/", id, sizeof(id));
	texture->depth_buffer = tex2d_create(id, tex_type_depth, format);
	if (texture->texture != nullptr) {
		tex2d_set_colors(texture->depth_buffer, texture->width, texture->height, nullptr);
	}
}
tex2d_t tex2d_find(const char *id) {
	tex2d_t result = (tex2d_t)assets_find(id);
	if (result != nullptr) {
		assets_addref(result->header);
		return result;
	}
	return nullptr;
}
tex2d_t tex2d_create_file(const char *file) {
	tex2d_t result = tex2d_find(file);
	if (result != nullptr)
		return result;

	int      channels = 0;
	int      width    = 0;
	int      height   = 0;
	uint8_t *data     = stbi_load(file, &width, &height, &channels, 4);

	if (data == nullptr) {
		return nullptr;
	}
	result = tex2d_create(file, tex_type_image);

	tex2d_set_colors(result, width, height, data);
	free(data);

	return result;
}
tex2d_t tex2d_create_mem(const char *id, void *data, size_t data_size) {
	tex2d_t result = tex2d_find(id);
	if (result != nullptr)
		return result;

	int      channels = 0;
	int      width    = 0;
	int      height   = 0;
	uint8_t *col_data =  stbi_load_from_memory((stbi_uc*)data, (int)data_size, &width, &height, &channels, 4);

	if (col_data == nullptr) {
		return nullptr;
	}
	result = tex2d_create(id);

	tex2d_set_colors(result, width, height, col_data);
	free(col_data);

	return result;
}

void tex2d_release(tex2d_t texture) {
	assets_releaseref(texture->header);
}
void tex2d_destroy(tex2d_t tex) {
	tex2d_releasesurface(tex);
	if (tex->sampler      != nullptr) tex->sampler->Release();
	if (tex->depth_buffer != nullptr) tex2d_release(tex->depth_buffer);
	
	*tex = {};
}
void tex2d_releasesurface(tex2d_t tex) {
	if (tex->resource    != nullptr) tex->resource   ->Release();
	if (tex->target_view != nullptr) tex->target_view->Release();
	if (tex->texture     != nullptr) tex->texture    ->Release();
	if (tex->depth_view  != nullptr) tex->depth_view ->Release();
	tex->resource    = nullptr;
	tex->target_view = nullptr;
	tex->texture     = nullptr;
	tex->depth_view  = nullptr;
}

void tex2d_set_colors(tex2d_t texture, int32_t width, int32_t height, void *data) {
	bool dynamic        = texture->type & tex_type_dynamic;
	bool different_size = texture->width != width || texture->height != height;
	if (texture->texture == nullptr || different_size) {
		tex2d_releasesurface(texture);
		
		texture->width  = width;
		texture->height = height;

		bool result = tex2d_create_surface(texture, data);
		if (result)
			result = tex2d_create_views  (texture);
		if (result && texture->depth_buffer != nullptr)
			tex2d_set_colors(texture->depth_buffer, width, height, nullptr);
	} else if (dynamic) {
		D3D11_MAPPED_SUBRESOURCE tex_mem = {};
		if (FAILED(d3d_context->Map(texture->texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &tex_mem))) {
			log_write(log_error, "Failed mapping a texture");
			return;
		}

		size_t   color_size = tex2d_format_size(texture->format);
		uint8_t *dest_line  = (uint8_t *)tex_mem.pData;
		uint8_t *src_line   = (uint8_t *)data;
		for (int i = 0; i < height; i++) {
			memcpy(dest_line, src_line, (size_t)width * color_size);
			dest_line += tex_mem.RowPitch;
			src_line  += width * color_size;
		}
		d3d_context->Unmap(texture->texture, 0);
	} else {
		log_write(log_warning, "Attempting additional writes to a non-dynamic texture!");
	}
}

void tex2d_set_options(tex2d_t texture, tex_sample_ sample, tex_address_ address_mode, int32_t anisotropy_level) {
	if (texture->sampler != nullptr)
		texture->sampler->Release();

	D3D11_TEXTURE_ADDRESS_MODE mode;
	switch (address_mode) {
	case tex_address_clamp:  mode = D3D11_TEXTURE_ADDRESS_CLAMP; break;
	case tex_address_wrap:   mode = D3D11_TEXTURE_ADDRESS_WRAP; break;
	case tex_address_mirror: mode = D3D11_TEXTURE_ADDRESS_MIRROR; break;
	}

	D3D11_FILTER filter;
	switch (sample) {
	case tex_sample_linear:     filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT; break;
	case tex_sample_point:      filter = D3D11_FILTER_MIN_MAG_MIP_POINT; break;
	case tex_sample_anisotropic:filter = D3D11_FILTER_ANISOTROPIC; break;
	}

	D3D11_SAMPLER_DESC desc_sampler = {};
	desc_sampler.AddressU = mode;
	desc_sampler.AddressV = mode;
	desc_sampler.AddressW = mode;
	desc_sampler.Filter   = filter;
	desc_sampler.MaxAnisotropy = anisotropy_level;
	desc_sampler.MaxLOD   = D3D11_FLOAT32_MAX;

	// D3D will already return the same sampler when provided the same settings, so we
	// can just lean on that to prevent sampler duplicates :)
	d3d_device->CreateSamplerState(&desc_sampler, &texture->sampler);
}

void tex2d_set_active(tex2d_t texture, int slot) {
	if (texture != nullptr) {
		d3d_context->PSSetSamplers       (slot, 1, &texture->sampler);
		d3d_context->PSSetShaderResources(slot, 1, &texture->resource);
	} else {
		d3d_context->PSSetShaderResources(slot, 0, nullptr);
	}
}

bool tex2d_create_surface(tex2d_t texture, void *data) {
	bool dynamic = texture->type & tex_type_dynamic;
	bool depth   = texture->type & tex_type_depth;
	bool rtarget = texture->type & tex_type_rendertarget;

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width            = texture->width;
	desc.Height           = texture->height;
	desc.MipLevels        = 1;
	desc.ArraySize        = 1;
	desc.SampleDesc.Count = 1;
	desc.Format           = tex2d_get_native_format(texture->format);
	desc.BindFlags        = depth   ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_SHADER_RESOURCE;
	desc.Usage            = dynamic ? D3D11_USAGE_DYNAMIC      : D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags   = dynamic ? D3D11_CPU_ACCESS_WRITE   : 0;
	if (rtarget)
		desc.BindFlags |= D3D11_BIND_RENDER_TARGET;

	D3D11_SUBRESOURCE_DATA tex_mem;
	tex_mem.pSysMem     = data;
	tex_mem.SysMemPitch = (UINT)(tex2d_format_size(texture->format) * texture->width);

	if (FAILED(d3d_device->CreateTexture2D(&desc, data == nullptr ? nullptr : &tex_mem, &texture->texture))) {
		log_write(log_error, "Create texture error!");
		return false;
	}
	return true;
}

void tex2d_setsurface(tex2d_t texture, ID3D11Texture2D *source) {
	tex2d_releasesurface(texture);
	texture->texture = source;

	int old_width  = texture->width;
	int old_height = texture->height;
	D3D11_TEXTURE2D_DESC color_desc;
	texture->texture->GetDesc(&color_desc);
	texture->width  = color_desc.Width;
	texture->height = color_desc.Height;

	bool created_views      = tex2d_create_views(texture);
	bool resolution_changed = (old_width != texture->width || old_height != texture->height);
	if (created_views && resolution_changed && texture->depth_buffer != nullptr) {
		tex2d_set_colors(texture->depth_buffer, texture->width, texture->height, nullptr);
	}
}

bool tex2d_create_views(tex2d_t texture) {
	DXGI_FORMAT format = tex2d_get_native_format(texture->format);

	if (!(texture->type & tex_type_depth)) {
		D3D11_SHADER_RESOURCE_VIEW_DESC res_desc = {};
		res_desc.Format              = format;
		res_desc.Texture2D.MipLevels = 1;
		res_desc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
		if (FAILED(d3d_device->CreateShaderResourceView(texture->texture, &res_desc, &texture->resource))) {
			log_write(log_error, "Create Shader Resource View error!");
			return false;
		}
	}

	if (texture->type & tex_type_rendertarget) {
		D3D11_RENDER_TARGET_VIEW_DESC target_desc = {};
		target_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		target_desc.Format        = format;
		if (FAILED(d3d_device->CreateRenderTargetView(texture->texture, &target_desc, &texture->target_view))) {
			log_write(log_error, "Create Render Target View error!");
			return false;
		}
	}

	if (texture->type & tex_type_depth) {
		D3D11_DEPTH_STENCIL_VIEW_DESC stencil_desc = {};
		stencil_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		stencil_desc.Format        = format;
		if (FAILED(d3d_device->CreateDepthStencilView(texture->texture, &stencil_desc, &texture->depth_view))) {
			log_write(log_error, "Create Depth Stencil View error!");
			return false;
		}
	}

	if (texture->sampler == nullptr)
		tex2d_set_options(texture);

	return true;
}

DXGI_FORMAT tex2d_get_native_format(tex_format_ format) {
	switch (format) {
	case tex_format_rgba32:  return DXGI_FORMAT_R8G8B8A8_UNORM;
	case tex_format_rgba64:  return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case tex_format_rgba128: return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case tex_format_depth32: return DXGI_FORMAT_D32_FLOAT;
	case tex_format_depth16: return DXGI_FORMAT_D16_UNORM;
	case tex_format_depthstencil: return DXGI_FORMAT_D24_UNORM_S8_UINT;
	default: return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
}

size_t tex2d_format_size(tex_format_ format) {
	switch (format) {
	case tex_format_depth32:
	case tex_format_depthstencil:
	case tex_format_rgba32:  return sizeof(color32);
	case tex_format_rgba64:  return sizeof(uint16_t)*4;
	case tex_format_rgba128: return sizeof(color128);
	case tex_format_depth16: return sizeof(uint16_t);
	default: return sizeof(color32);
	}
}

void tex2d_rtarget_clear(tex2d_t render_target, color32 color) {
	assert(render_target->type & tex_type_rendertarget);

	if (render_target->target_view != nullptr) {
		float colorF[4] = {
			(uint8_t)color.r / 255.0f,
			(uint8_t)color.g / 255.0f,
			(uint8_t)color.b / 255.0f,
			(uint8_t)color.a / 255.0f, };
		d3d_context->ClearRenderTargetView(render_target->target_view, colorF);
	}

	if (render_target->depth_buffer != nullptr && render_target->depth_buffer->depth_view != nullptr)
		d3d_context->ClearDepthStencilView (render_target->depth_buffer->depth_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}
void tex2d_rtarget_set_active(tex2d_t render_target) {
	if (render_target == nullptr) {
		d3d_context->OMSetRenderTargets(0, nullptr, nullptr);
		return;
	}

	assert(render_target->type & tex_type_rendertarget);

	bool                    has_depth  = render_target->depth_buffer != nullptr && render_target->depth_buffer->depth_view != nullptr;
	ID3D11DepthStencilView *depth_view = has_depth ? render_target->depth_buffer->depth_view : nullptr;

	d3d_context->OMSetRenderTargets(1, &render_target->target_view, depth_view);
}