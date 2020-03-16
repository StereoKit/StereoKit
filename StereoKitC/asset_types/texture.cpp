#include "../stereokit.h"
#include "../shaders_builtin/shader_builtin.h"
#include "../systems/d3d.h"
#include "../systems/platform/platform_utils.h"
#include "../libraries/stref.h"
#include "../math.h"
#include "../spherical_harmonics.h"
#include "texture.h"

#pragma warning( disable : 26451 6011 6262 6308 6387 28182 )
#define STB_IMAGE_IMPLEMENTATION
#include "../libraries/stb_image.h"
#pragma warning( default : 26451 6011 6262 6308 6387 28182 )

namespace sk {

///////////////////////////////////////////

tex_t tex_create(tex_type_ type, tex_format_ format) {
	tex_t result = (tex_t)assets_allocate(asset_type_texture);
	result->type   = type;
	result->format = format;
	result->address_mode = tex_address_wrap;
	result->sample_mode  = tex_sample_linear;
	result->anisotropy   = 4;
	return result;
}

///////////////////////////////////////////

tex_t tex_add_zbuffer(tex_t texture, tex_format_ format) {
	if (!(texture->type & tex_type_rendertarget)) {
		log_err("tex_add_zbuffer can't add a zbuffer to a non-rendertarget texture!");
		return nullptr;
	}

	char id[64];
	assets_unique_name(asset_type_texture, "zbuffer/", id, sizeof(id));
	texture->depth_buffer = tex_create(tex_type_depth, format);
	tex_set_id(texture->depth_buffer, id);
	if (texture->texture != nullptr) {
		tex_set_color_arr(texture->depth_buffer, texture->width, texture->height, nullptr, texture->array_size);
	}
	return texture->depth_buffer;
}

///////////////////////////////////////////

void tex_set_zbuffer(tex_t texture, tex_t depth_texture) {
	if (!(texture->type & tex_type_rendertarget)) {
		log_err("tex_set_zbuffer can't add a zbuffer to a non-rendertarget texture!");
		return;
	}
	if (!(depth_texture->type & tex_type_depth)) {
		log_err("tex_set_zbuffer can't add a non-depth texture as a zbuffer!");
		return;
	}
	texture->depth_buffer = depth_texture;
}

///////////////////////////////////////////

tex_t tex_find(const char *id) {
	tex_t result = (tex_t)assets_find(id, asset_type_texture);
	if (result != nullptr) {
		assets_addref(result->header);
		return result;
	}
	return nullptr;
}

///////////////////////////////////////////

void tex_set_id(tex_t tex, const char *id) {
	assets_set_id(tex->header, id);

	if (tex->resource)
		DX11ResName(tex->resource,    "tex_view", id);
	if (tex->texture)
		DX11ResName(tex->texture,     "tex_src", id);
	if (tex->target_view)
		DX11ResName(tex->target_view, "tex_target", id);
}

///////////////////////////////////////////

tex_t tex_create_mem(void *data, size_t data_size, bool32_t srgb_data) {
	bool     is_hdr   = stbi_is_hdr_from_memory((stbi_uc*)data, data_size);
	int      channels = 0;
	int      width    = 0;
	int      height   = 0;
	uint8_t *col_data =  is_hdr ? 
		(uint8_t *)stbi_loadf_from_memory((stbi_uc*)data, data_size, &width, &height, &channels, 4):
		(uint8_t *)stbi_load_from_memory ((stbi_uc*)data, data_size, &width, &height, &channels, 4);

	if (col_data == nullptr) {
		log_warn("Couldn't parse image data!");
		return nullptr;
	}
	tex_format_ format = srgb_data ? tex_format_rgba32 : tex_format_rgba32_linear;
	if (is_hdr) format = tex_format_rgba128;

	tex_t result = tex_create(tex_type_image, format);

	tex_set_colors(result, width, height, col_data);
	free(col_data);

	DX11ResType(result->resource, "tex_view");
	DX11ResType(result->texture,  "tex_src" );
	return result;
}

///////////////////////////////////////////

tex_t tex_create_file(const char *file, bool32_t srgb_data) {
	tex_t result = tex_find(file);
	if (result != nullptr)
		return result;

	void  *file_data;
	size_t file_size;
	if (!platform_read_file(assets_file(file), file_data, file_size))
		return nullptr;

	result = tex_create_mem(file_data, file_size, srgb_data);
	free(file_data);

	if (result == nullptr) {
		log_warnf("Issue loading file [%s]", file);
		return nullptr;
	}
	tex_set_id(result, file);
	
	return result;
}

///////////////////////////////////////////

tex_t tex_create_cubemap_file(const char *equirectangular_file, bool32_t srgb_data, spherical_harmonics_t *sh_lighting_info) {
	tex_t result = tex_find(equirectangular_file);
	if (result != nullptr)
		return result;

	const vec3 up   [6] = { -vec3_up, -vec3_up, vec3_forward, -vec3_forward, -vec3_up, -vec3_up };
	const vec3 fwd  [6] = { {1,0,0}, {-1,0,0}, {0,-1,0}, {0,1,0}, {0,0,1}, {0,0,-1} };
	const vec3 right[6] = { {0,0,-1}, {0,0,1}, {1,0,0}, {1,0,0}, {1,0,0}, {-1,0,0} };

	tex_t equirect = tex_create_file(equirectangular_file, srgb_data ? tex_format_rgba32 : tex_format_rgba32_linear);
	if (equirect == nullptr)
		return nullptr;
	equirect->header.id = string_hash("temp/equirectid");
	material_t convert_material = material_find("default/equirect_convert");

	material_set_texture( convert_material, "source", equirect );

	tex_t    face    = tex_create(tex_type_rendertarget, equirect->format);
	void    *data[6] = {};
	int      width   = equirect->height / 2;
	int      height  = width;
	size_t   size    = (size_t)width*(size_t)height*tex_format_size(equirect->format);
	tex_set_colors(face, width, height, nullptr);
	for (size_t i = 0; i < 6; i++) {
		material_set_vector(convert_material, "up",      { up   [i].x, up   [i].y, up   [i].z, 0 });
		material_set_vector(convert_material, "right",   { right[i].x, right[i].y, right[i].z, 0 });
		material_set_vector(convert_material, "forward", { fwd  [i].x, fwd  [i].y, fwd  [i].z, 0 });

		render_blit (face, convert_material);
		data[i] = malloc(size);
		tex_get_data(face, data[i], size);
	}

	result = tex_create(tex_type_image | tex_type_cubemap, equirect->format);
	tex_set_color_arr(result, width, height, (void**)&data, 6, sh_lighting_info);
	tex_set_id       (result, equirectangular_file);

	material_release(convert_material);
	tex_release(equirect);
	tex_release(face);

	for (size_t i = 0; i < 6; i++) {
		free(data[i]);
	}

	return result;
}

///////////////////////////////////////////

tex_t tex_create_cubemap_files(const char **cube_face_file_xxyyzz, bool32_t srgb_data, spherical_harmonics_t *sh_lighting_info) {
	tex_t result = tex_find(cube_face_file_xxyyzz[0]);
	if (result != nullptr)
		return result;

	// Load all 6 faces
	uint8_t *data[6] = {};
	int  final_width  = 0;
	int  final_height = 0;
	bool loaded       = true;
	for (size_t i = 0; i < 6; i++) {
		int channels = 0;
		int width    = 0;
		int height   = 0;
		data[i] = stbi_load(assets_file(cube_face_file_xxyyzz[i]), &width, &height, &channels, 4);

		// Check if there were issues, or one of the images is the wrong size!
		if (data[i] == nullptr || 
			(final_width  != 0 && final_width  != width ) ||
			(final_height != 0 && final_height != height)) {
			loaded = false;
			log_errf("Issue loading cubemap image '%s', file not found, invalid image format, or faces of different sizes?", cube_face_file_xxyyzz[i]);
			break;
		}
		final_width  = width;
		final_height = height;
	}

	// free memory if we failed
	if (!loaded) {
		for (size_t i = 0; i < 6; i++) {
			free(data[i]);
		}
		return nullptr;
	}

	// Create with the data we have
	result = tex_create(tex_type_image | tex_type_cubemap, srgb_data ? tex_format_rgba32 : tex_format_rgba32_linear);
	tex_set_color_arr(result, final_width, final_height, (void**)&data, 6, sh_lighting_info);
	tex_set_id       (result, cube_face_file_xxyyzz[0]);
	for (size_t i = 0; i < 6; i++) {
		free(data[i]);
	}

	return result;
}

///////////////////////////////////////////

void tex_release(tex_t texture) {
	if (texture == nullptr)
		return;
	assets_releaseref(texture->header);
}

///////////////////////////////////////////

void tex_destroy(tex_t tex) {
	tex_releasesurface(tex);
	if (tex->sampler      != nullptr) tex->sampler->Release();
	if (tex->depth_buffer != nullptr) tex_release(tex->depth_buffer);
	
	*tex = {};
}

///////////////////////////////////////////

void tex_releasesurface(tex_t tex) {
	if (tex->resource    != nullptr) tex->resource   ->Release();
	if (tex->target_view != nullptr) tex->target_view->Release();
	if (tex->texture     != nullptr) tex->texture    ->Release();
	if (tex->depth_view  != nullptr) tex->depth_view ->Release();
	tex->resource    = nullptr;
	tex->target_view = nullptr;
	tex->texture     = nullptr;
	tex->depth_view  = nullptr;
}

///////////////////////////////////////////

void tex_set_color_arr(tex_t texture, int32_t width, int32_t height, void **data, int32_t data_count, spherical_harmonics_t *sh_lighting_info) {
	bool dynamic        = texture->type & tex_type_dynamic;
	bool different_size = texture->width != width || texture->height != height || texture->array_size != data_count;
	if (!different_size && (data == nullptr || *data == nullptr))
		return;
	if (texture->texture == nullptr || different_size || (!different_size && !dynamic)) {
		tex_releasesurface(texture);
		
		texture->width  = width;
		texture->height = height;
		texture->array_size = data_count;

		if (!different_size && !dynamic)
			texture->type &= tex_type_dynamic;

		bool result = tex_create_surface(texture, data, data_count, sh_lighting_info);
		if (result)
			result = tex_create_views  (texture, DXGI_FORMAT_UNKNOWN, true);
		if (result && texture->depth_buffer != nullptr)
			tex_set_colors(texture->depth_buffer, width, height, nullptr);
	} else if (dynamic) {
		D3D11_MAPPED_SUBRESOURCE tex_mem = {};
		if (FAILED(d3d_context->Map(texture->texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &tex_mem))) {
			log_err("Failed mapping a texture");
			return;
		}

		size_t   color_size = tex_format_size(texture->format);
		uint8_t *dest_line  = (uint8_t *)tex_mem.pData;
		uint8_t *src_line   = (uint8_t *)data[0];
		for (int i = 0; i < height; i++) {
			memcpy(dest_line, src_line, (size_t)width * color_size);
			dest_line += tex_mem.RowPitch;
			src_line  += width * color_size;
		}
		d3d_context->Unmap(texture->texture, 0);
	} else {
		log_warn("Attempting additional writes to a non-dynamic texture!");
	}
}

///////////////////////////////////////////

void tex_set_colors(tex_t texture, int32_t width, int32_t height, void *data) {
	void *data_arr[1] = { data };
	tex_set_color_arr(texture, width, height, data_arr, 1);
}

///////////////////////////////////////////

void tex_set_options(tex_t texture, tex_sample_ sample, tex_address_ address_mode, int32_t anisotropy_level) {
	texture->address_mode = address_mode;
	texture->anisotropy   = anisotropy_level;
	texture->sample_mode  = sample;
	if (texture->sampler != nullptr)
		texture->sampler->Release();

	D3D11_TEXTURE_ADDRESS_MODE mode;
	switch (address_mode) {
	case tex_address_clamp:  mode = D3D11_TEXTURE_ADDRESS_CLAMP;  break;
	case tex_address_wrap:   mode = D3D11_TEXTURE_ADDRESS_WRAP;   break;
	case tex_address_mirror: mode = D3D11_TEXTURE_ADDRESS_MIRROR; break;
	default: mode = D3D11_TEXTURE_ADDRESS_WRAP;
	}

	D3D11_FILTER filter;
	switch (sample) {
	case tex_sample_linear:     filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; break; // Technically trilinear
	case tex_sample_point:      filter = D3D11_FILTER_MIN_MAG_MIP_POINT;  break;
	case tex_sample_anisotropic:filter = D3D11_FILTER_ANISOTROPIC;        break;
	default: filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	}

	D3D11_SAMPLER_DESC desc_sampler = {};
	desc_sampler.AddressU = mode;
	desc_sampler.AddressV = mode;
	desc_sampler.AddressW = mode;
	desc_sampler.Filter   = filter;
	desc_sampler.MaxAnisotropy  = anisotropy_level;
	desc_sampler.MaxLOD         = D3D11_FLOAT32_MAX;
	desc_sampler.ComparisonFunc = D3D11_COMPARISON_ALWAYS;

	// D3D will already return the same sampler when provided the same settings, so we
	// can just lean on that to prevent sampler duplicates :)
	if (FAILED(d3d_device->CreateSamplerState(&desc_sampler, &texture->sampler)))
		log_warnf("tex_set_options: failed to create sampler state!");
}

///////////////////////////////////////////

tex_format_ tex_get_format(tex_t texture) {
	return texture->format;
}

///////////////////////////////////////////

int32_t tex_get_width(tex_t texture) {
	return texture->width;
}

///////////////////////////////////////////

int32_t tex_get_height(tex_t texture) {
	return texture->height;
}

///////////////////////////////////////////

void tex_set_sample(tex_t texture, tex_sample_ sample) {
	texture->sample_mode = sample;
	if (texture->sampler != nullptr)
		tex_set_options(texture, texture->sample_mode, texture->address_mode, texture->anisotropy);
}

///////////////////////////////////////////

tex_sample_ tex_get_sample(tex_t texture) {
	return texture->sample_mode;
}

///////////////////////////////////////////

void tex_set_address(tex_t texture, tex_address_ address_mode) {
	texture->address_mode = address_mode;
	if (texture->sampler != nullptr)
		tex_set_options(texture, texture->sample_mode, texture->address_mode, texture->anisotropy);
}

///////////////////////////////////////////

tex_address_ tex_get_address(tex_t texture) {
	return texture->address_mode;
}

///////////////////////////////////////////

void tex_set_anisotropy(tex_t texture, int32_t anisotropy_level) {
	texture->anisotropy = anisotropy_level;
	if (texture->sampler != nullptr)
		tex_set_options(texture, texture->sample_mode, texture->address_mode, texture->anisotropy);
}

///////////////////////////////////////////

int32_t tex_get_anisotropy(tex_t texture) {
	return texture->anisotropy;
}

///////////////////////////////////////////

void tex_set_active(tex_t texture, int slot) {
	if (texture != nullptr) {
		d3d_context->PSSetSamplers       (slot, 1, &texture->sampler);
		d3d_context->PSSetShaderResources(slot, 1, &texture->resource);
	} else {
		d3d_context->PSSetShaderResources(slot, 1, nullptr);
	}
}

///////////////////////////////////////////

bool tex_create_surface(tex_t texture, void **data, int32_t data_count, spherical_harmonics_t *sh_lighting_info) {
	if (sh_lighting_info != nullptr) *sh_lighting_info = {};

	bool mips    = (texture->width & (texture->width - 1)) == 0 && texture->type & tex_type_mips && texture->width == texture->height && (texture->format == tex_format_rgba32 || texture->format == tex_format_rgba32_linear || texture->format == tex_format_rgba128);
	bool dynamic = texture->type & tex_type_dynamic;
	bool depth   = texture->type & tex_type_depth;
	bool rtarget = texture->type & tex_type_rendertarget;

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width            = texture->width;
	desc.Height           = texture->height;
	desc.MipLevels        = (UINT)(mips ? log2(texture->width) + 1 : 1);
	desc.ArraySize        = data_count;
	desc.SampleDesc.Count = 1;
	desc.Format           = tex_get_native_format(texture->format);
	desc.BindFlags        = depth   ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_SHADER_RESOURCE;
	desc.Usage            = dynamic ? D3D11_USAGE_DYNAMIC      : D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags   = dynamic ? D3D11_CPU_ACCESS_WRITE   : 0;
	if (rtarget)
		desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	if (texture->type & tex_type_cubemap)
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	// Generate mip-maps for the texture
	D3D11_SUBRESOURCE_DATA *tex_mem = nullptr;
	if (data != nullptr && data[0] != nullptr) {
		tex_mem = (D3D11_SUBRESOURCE_DATA *)malloc(data_count * desc.MipLevels * sizeof(D3D11_SUBRESOURCE_DATA));
		for (int32_t i = 0; i < data_count; i++) {
			void *curr_data = data[i];
		
			tex_mem[i*desc.MipLevels].pSysMem     = curr_data;
			tex_mem[i*desc.MipLevels].SysMemPitch = (UINT)(tex_format_size(texture->format) * texture->width);

			if (mips) {
				void    *mip_data     = curr_data;
				int32_t  mip_width    = texture->width;
				int32_t  mip_height   = texture->height;
				for (uint32_t m = 1; m < desc.MipLevels; m++) {
					uint32_t index = i*desc.MipLevels + m;
					if (texture->format == tex_format_rgba128)
						tex_downsample_128((color128*)mip_data, mip_width, mip_height, (color128**)&tex_mem[index].pSysMem, &mip_width, &mip_height);
					else
						tex_downsample    ((color32* )mip_data, mip_width, mip_height, (color32** )&tex_mem[index].pSysMem, &mip_width, &mip_height);
					mip_data = (void*)tex_mem[index].pSysMem;
					tex_mem[index].SysMemPitch = (UINT)(tex_format_size(texture->format) * mip_width);
					
				}
			}
		}
		if (sh_lighting_info != nullptr) {
			int32_t lighting_mip = maxi(0, (int32_t)desc.MipLevels-6);
			void *mip_level[6] = { 
				(void*)tex_mem[0 * desc.MipLevels + lighting_mip].pSysMem,
				(void*)tex_mem[1 * desc.MipLevels + lighting_mip].pSysMem,
				(void*)tex_mem[2 * desc.MipLevels + lighting_mip].pSysMem,
				(void*)tex_mem[3 * desc.MipLevels + lighting_mip].pSysMem,
				(void*)tex_mem[4 * desc.MipLevels + lighting_mip].pSysMem,
				(void*)tex_mem[5 * desc.MipLevels + lighting_mip].pSysMem,};
			*sh_lighting_info = sh_calculate(mip_level, texture->format, (int32_t)powf(2, (float)desc.MipLevels - (lighting_mip+1)));
		}
	}
	
	bool result = true;
	if (FAILED(d3d_device->CreateTexture2D(&desc, tex_mem, &texture->texture))) {
		log_err("Create texture error!");
		result = false;
	}

	// Free any mip-maps we've generated!
	if (tex_mem != nullptr) {
		for (int32_t i = 0; i < data_count; i++) {
			for (uint32_t m = 1; m < desc.MipLevels; m++) {
				uint32_t index = i*desc.MipLevels + m;
				free((void*)tex_mem[index].pSysMem);
			} 
		}
		free(tex_mem);
	}

	return result;
}

///////////////////////////////////////////

void tex_setsurface(tex_t texture, ID3D11Texture2D *source, DXGI_FORMAT source_format) {
	tex_releasesurface(texture);
	texture->texture = source;
	source->AddRef();

	int old_width  = texture->width;
	int old_height = texture->height;
	D3D11_TEXTURE2D_DESC color_desc;
	texture->texture->GetDesc(&color_desc);
	texture->width      = color_desc.Width;
	texture->height     = color_desc.Height;
	texture->array_size = color_desc.ArraySize;
	if (source_format == DXGI_FORMAT_UNKNOWN)
		source_format = color_desc.Format;
	bool create_view = color_desc.BindFlags & D3D11_BIND_SHADER_RESOURCE;

	bool created_views      = tex_create_views(texture, source_format, create_view);
	bool resolution_changed = (old_width != texture->width || old_height != texture->height);
	if (created_views && resolution_changed && texture->depth_buffer != nullptr) {
		tex_set_colors(texture->depth_buffer, texture->width, texture->height, nullptr);
	}
}

///////////////////////////////////////////

bool tex_create_views(tex_t texture, DXGI_FORMAT source_format, bool create_shader_view) {
	DXGI_FORMAT format    = source_format == DXGI_FORMAT_UNKNOWN ? tex_get_native_format(texture->format) : source_format;
	bool        mips      = (texture->width & (texture->width - 1)) == 0 && texture->type & tex_type_mips && texture->width == texture->height && (texture->format == tex_format_rgba32 || texture->format == tex_format_rgba32_linear || texture->format == tex_format_rgba128) ;
	uint32_t    mip_count = (uint32_t)(mips ? log2(texture->width) + 1 : 1);

	if (!(texture->type & tex_type_depth) && create_shader_view) {
		D3D11_SHADER_RESOURCE_VIEW_DESC res_desc = {};
		res_desc.Format              = format;
		if (texture->type & tex_type_cubemap) {
			res_desc.TextureCube.MipLevels = mip_count;
			res_desc.ViewDimension         = D3D11_SRV_DIMENSION_TEXTURECUBE;
		} else if (texture->array_size > 1) {
			res_desc.Texture2DArray.MipLevels = mip_count;
			res_desc.Texture2DArray.ArraySize = texture->array_size;
			res_desc.ViewDimension            = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		} else {
			res_desc.Texture2D.MipLevels = mip_count;
			res_desc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
		}
		
		if (FAILED(d3d_device->CreateShaderResourceView(texture->texture, &res_desc, &texture->resource))) {
			log_err("Create Shader Resource View error!");
			return false;
		}
	}

	if (texture->type & tex_type_rendertarget) {
		D3D11_RENDER_TARGET_VIEW_DESC target_desc = {};
		target_desc.Format = format;
		if (texture->type & tex_type_cubemap || texture->array_size > 1) {
			target_desc.Texture2DArray.ArraySize = texture->array_size;
			target_desc.ViewDimension            = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		} else {
			target_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		}

		if (FAILED(d3d_device->CreateRenderTargetView(texture->texture, &target_desc, &texture->target_view))) {
			log_err("Create Render Target View error!");
			return false;
		}
	}

	if (texture->type & tex_type_depth) {
		D3D11_DEPTH_STENCIL_VIEW_DESC stencil_desc = {};
		stencil_desc.Format = format;
		if (texture->type & tex_type_cubemap || texture->array_size > 1) {
			stencil_desc.Texture2DArray.ArraySize = texture->array_size;
			stencil_desc.ViewDimension            = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		} else {
			stencil_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		}
		if (FAILED(d3d_device->CreateDepthStencilView(texture->texture, &stencil_desc, &texture->depth_view))) {
			log_err("Create Depth Stencil View error!");
			return false;
		}
	}

	if (texture->sampler == nullptr)
		tex_set_options(texture, texture->sample_mode, texture->address_mode, texture->anisotropy);

	return true;
}

///////////////////////////////////////////

DXGI_FORMAT tex_get_native_format(tex_format_ format) {
	switch (format) {
	case tex_format_rgba32:        return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case tex_format_rgba32_linear: return DXGI_FORMAT_R8G8B8A8_UNORM;
	case tex_format_rgba64:        return DXGI_FORMAT_R16G16B16A16_UNORM;
	case tex_format_rgba128:       return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case tex_format_depth32:       return DXGI_FORMAT_D32_FLOAT;
	case tex_format_depth16:       return DXGI_FORMAT_D16_UNORM;
	case tex_format_depthstencil:  return DXGI_FORMAT_D24_UNORM_S8_UINT;
	default: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	}
}

///////////////////////////////////////////

tex_format_ tex_get_tex_format(DXGI_FORMAT format) {
	switch (format) {
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return tex_format_rgba32;
	case DXGI_FORMAT_R8G8B8A8_UNORM:      return tex_format_rgba32_linear;
	case DXGI_FORMAT_R16G16B16A16_UNORM:  return tex_format_rgba64;
	case DXGI_FORMAT_R32G32B32A32_FLOAT:  return tex_format_rgba128;
	case DXGI_FORMAT_D32_FLOAT:           return tex_format_depth32;
	case DXGI_FORMAT_D16_UNORM:           return tex_format_depth16;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:   return tex_format_depthstencil;
	default: log_errf("tex_get_tex_format received an unrecognized format %d", format); return (tex_format_)0;
	}
}

///////////////////////////////////////////

size_t tex_format_size(tex_format_ format) {
	switch (format) {
	case tex_format_depth32:
	case tex_format_depthstencil:
	case tex_format_rgba32:
	case tex_format_rgba32_linear: return sizeof(color32);
	case tex_format_rgba64:        return sizeof(uint16_t)*4;
	case tex_format_rgba128:       return sizeof(color128);
	case tex_format_depth16:       return sizeof(uint16_t);
	default: return sizeof(color32);
	}
}

///////////////////////////////////////////

void tex_rtarget_clear(tex_t render_target, color32 color) {
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

///////////////////////////////////////////

void tex_rtarget_set_active(tex_t render_target) {
	if (render_target == nullptr) {
		ID3D11RenderTargetView* null_rtv = nullptr;
		d3d_context->OMSetRenderTargets(1, &null_rtv, nullptr);
		return;
	}

	assert(render_target->type & tex_type_rendertarget);

	bool                    has_depth  = render_target->depth_buffer != nullptr && render_target->depth_buffer->depth_view != nullptr;
	ID3D11DepthStencilView *depth_view = has_depth ? render_target->depth_buffer->depth_view : nullptr;

	d3d_context->OMSetRenderTargets(1, &render_target->target_view, depth_view);
}

///////////////////////////////////////////

void tex_get_data(tex_t texture, void *out_data, size_t out_data_size) {
	// Make sure we've been provided enough memory to hold this texture
	size_t format_size = tex_format_size(texture->format);
	assert(out_data_size == (size_t)texture->width * (size_t)texture->height * format_size);

	D3D11_TEXTURE2D_DESC desc             = {};
	ID3D11Texture2D     *copy_tex         = nullptr;
	bool                 copy_tex_release = true;
	texture->texture->GetDesc(&desc);

	// Make sure copy_tex is a texture that we can read from!
	if (desc.SampleDesc.Count > 1) {
		// Not gonna bother with MSAA stuff
		log_warn("tex_get_data AA surfaces not implemented");
		return;
	} else if ((desc.Usage == D3D11_USAGE_STAGING) && (desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ)) {
		// Handle case where the source is already a staging texture we can use directly
		copy_tex         = texture->texture;
		copy_tex_release = false;
	} else {
		// Otherwise, create a staging texture from the non-MSAA source
		desc.BindFlags      = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.Usage          = D3D11_USAGE_STAGING;

		if (FAILED(d3d_device->CreateTexture2D(&desc, nullptr, &copy_tex))) {
			log_err("CreateTexture2D failed!");
			return;
		}
		d3d_context->CopyResource(copy_tex, texture->texture);
	}

	// Load the data into CPU RAM
	D3D11_MAPPED_SUBRESOURCE data;
	if (FAILED(d3d_context->Map(copy_tex, 0, D3D11_MAP_READ, 0, &data))) {
		log_err("Texture Map failed!");
		return;
	}

	// Copy it into our waiting buffer
	uint8_t *srcPtr  = (uint8_t*)data.pData;
	uint8_t *destPtr = (uint8_t*)out_data;
	size_t   msize   = texture->width*format_size;
	for (size_t h = 0; h < desc.Height; ++h) {
		memcpy(destPtr, srcPtr, msize);
		srcPtr  += data.RowPitch;
		destPtr += msize;
	}

	// And cleanup
	d3d_context->Unmap(copy_tex, 0);
	if (copy_tex_release)
		copy_tex->Release();
}

///////////////////////////////////////////

void *tex_get_resource(tex_t texture) {
	return texture->resource;
}

///////////////////////////////////////////

void tex_set_resource(tex_t texture, void *surface) {
	tex_setsurface(texture, (ID3D11Texture2D*)surface, DXGI_FORMAT_UNKNOWN);
}

///////////////////////////////////////////

tex_t tex_gen_cubemap(const gradient_t gradient_bot_to_top, vec3 gradient_dir, int32_t resolution, spherical_harmonics_t* sh_lighting_info) {
	tex_t result = tex_create(tex_type_image | tex_type_cubemap, tex_format_rgba128);
	if (result == nullptr) {
		return nullptr;
	}
	gradient_dir = vec3_normalize(gradient_dir);

	int32_t size  = resolution;
	// make size a power of two
	int32_t power = (int32_t)logf((float)size);
	if (pow(2, power) < size)
		power += 1;
	size = (int32_t)pow(2, power);

	float    half_px = 0.5f / size;
	int32_t  size2 = size * size;
	color128*data[6];
	for (int32_t i = 0; i < 6; i++) {
		data[i] = (color128 *)malloc(size2 * sizeof(color128));
		vec3 p1 = math_cubemap_corner(i * 4);
		vec3 p2 = math_cubemap_corner(i * 4+1);
		vec3 p3 = math_cubemap_corner(i * 4+2);
		vec3 p4 = math_cubemap_corner(i * 4+3); 

		for (int32_t y = 0; y < size; y++) {
			float py = 1 - (y / (float)size + half_px);

			// Top face is flipped on both axes
			if (i == 2) {
				py = 1 - py;
			}
		for (int32_t x = 0; x < size; x++) {
			float px = x / (float)size + half_px;

			// Top face is flipped on both axes
			if (i == 2) {
				px = 1 - px;
			}

			vec3 pl = vec3_lerp(p1, p4, py);
			vec3 pr = vec3_lerp(p2, p3, py);
			vec3 pt = vec3_lerp(pl, pr, px);
			pt = vec3_normalize(pt);

			float pct = (vec3_dot(pt, gradient_dir)+1)*0.5f;
			data[i][x + y * size] = gradient_get(gradient_bot_to_top, pct);
		}
		}
	}

	tex_set_color_arr(result, (int32_t)size, (int32_t)size, (void**)data, 6, sh_lighting_info);
	for (int32_t i = 0; i < 6; i++) {
		free(data[i]);
	}

	return result;
}

///////////////////////////////////////////

tex_t tex_gen_cubemap_sh(const spherical_harmonics_t& lookup, int32_t face_size) {
	tex_t result = tex_create(tex_type_image | tex_type_cubemap, tex_format_rgba128);
	if (result == nullptr) {
		return nullptr;
	}

	int32_t size  = face_size;
	// make size a power of two
	int32_t power = (int32_t)logf((float)size);
	if (pow(2, power) < size)
		power += 1;
	size = (int32_t)pow(2, power);

	float    half_px = 0.5f / size;
	int32_t  size2 = size * size;
	color128 *data[6];
	for (int32_t i = 0; i < 6; i++) {
		data[i] = (color128 *)malloc(size2 * sizeof(color128));
		vec3 p1 = math_cubemap_corner(i * 4);
		vec3 p2 = math_cubemap_corner(i * 4+1);
		vec3 p3 = math_cubemap_corner(i * 4+2);
		vec3 p4 = math_cubemap_corner(i * 4+3); 

		for (int32_t y = 0; y < size; y++) {
			float py = 1 - (y / (float)size + half_px);

			// Top face is flipped on both axes
			if (i == 2) {
				py = 1 - py;
			}
			for (int32_t x = 0; x < size; x++) {
				float px = x / (float)size + half_px;

				// Top face is flipped on both axes
				if (i == 2) {
					px = 1 - px;
				}

				vec3 pl = vec3_lerp(p1, p4, py);
				vec3 pr = vec3_lerp(p2, p3, py);
				vec3 pt = vec3_lerp(pl, pr, px);
				pt = vec3_normalize(pt);

				data[i][x + y * size] = sh_lookup(lookup, pt);
			}
		}
	}

	tex_set_color_arr(result, (int32_t)size, (int32_t)size, (void**)data, 6);
	for (int32_t i = 0; i < 6; i++) {
		free(data[i]);
	}

	return result;
}

///////////////////////////////////////////

bool tex_downsample(color32 *data, int32_t width, int32_t height, color32 **out_data, int32_t *out_width, int32_t *out_height) {
	int w = (int32_t)log2(width);
	int h = (int32_t)log2(height);
	w = (1 << w)  >> 1;
	h = (1 << h) >> 1;

	*out_data = (color32*)malloc(w * h * sizeof(color32));
	memset(*out_data, 0, w * h * sizeof(color32));
	color32 *result = *out_data;

	for (int32_t y = 0; y < height; y++) {
		int32_t src_row_start  = y * width;
		int32_t dest_row_start = (y / 2) * w;
	for (int32_t x = 0; x < width;  x++) {
		int src  = x + src_row_start;
		int dest = (x / 2) + dest_row_start;
		color32 cD = result[dest];
		color32 cS = data  [src];

		cD.r += cS.r / 4;
		cD.g += cS.g / 4;
		cD.b += cS.b / 4;
		cD.a += cS.a / 4;

		result[dest] = cD;
	}
	}

	*out_width   = w;
	*out_height  = h;
	return w > 1 && h > 1;
}

///////////////////////////////////////////

bool tex_downsample_128(color128 *data, int32_t width, int32_t height, color128 **out_data, int32_t *out_width, int32_t *out_height) {
	int w = (int32_t)log2(width);
	int h = (int32_t)log2(height);
	w = (1 << w)  >> 1;
	h = (1 << h) >> 1;

	*out_data = (color128*)malloc(w * h * sizeof(color128));
	memset(*out_data, 0, w * h * sizeof(color128));
	color128 *result = *out_data;

	for (int32_t y = 0; y < height; y++) {
		int32_t src_row_start  = y * width;
		int32_t dest_row_start = (y / 2) * w;
		for (int32_t x = 0; x < width;  x++) {
			int src  = x + src_row_start;
			int dest = (x / 2) + dest_row_start;
			color128 cD = result[dest];
			color128 cS = data  [src];

			cD.r += cS.r / 4;
			cD.g += cS.g / 4;
			cD.b += cS.b / 4;
			cD.a += cS.a / 4;

			result[dest] = cD;
		}
	}

	*out_width   = w;
	*out_height  = h;
	return w > 1 && h > 1;
}

} // namespace sk