#include "../stereokit.h"
#include "../shaders_builtin/shader_builtin.h"
#include "../systems/d3d.h"
#include "../stref.h"
#include "texture.h"

#pragma warning( disable : 26451 6011 6262 6308 6387 28182 )
#define STB_IMAGE_IMPLEMENTATION
#include "../libraries/stb_image.h"
#pragma warning( default : 26451 6011 6262 6308 6387 28182 )

///////////////////////////////////////////

tex2d_t tex2d_create(const char *id, tex_type_ type, tex_format_ format) {
	tex2d_t result = (tex2d_t)assets_allocate(asset_type_texture, id);
	result->type   = type;
	result->format = format;
	return result;
}

///////////////////////////////////////////

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

///////////////////////////////////////////

tex2d_t tex2d_find(const char *id) {
	tex2d_t result = (tex2d_t)assets_find(id);
	if (result != nullptr) {
		assets_addref(result->header);
		return result;
	}
	return nullptr;
}

///////////////////////////////////////////

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

	DX11ResName(result->resource, "tex2d_view", file);
	DX11ResName(result->texture,  "tex2d_src", file);
	return result;
}

///////////////////////////////////////////

tex2d_t tex2d_create_cubemap_file(const char *equirectangular_file) {
	tex2d_t result = tex2d_find(equirectangular_file);
	if (result != nullptr)
		return result;

	const vec3 up   [6] = { -vec3_up, -vec3_up, vec3_forward, -vec3_forward, -vec3_up, -vec3_up };
	const vec3 fwd  [6] = { {1,0,0}, {-1,0,0}, {0,-1,0}, {0,1,0}, {0,0,1}, {0,0,-1} };
	const vec3 right[6] = { {0,0,-1}, {0,0,1}, {1,0,0}, {1,0,0}, {1,0,0}, {-1,0,0} };

	tex2d_t    equirect         = tex2d_create_file(equirectangular_file);
	equirect->header.id = string_hash("temp/equirectid");
	shader_t   convert_shader   = shader_create  ("default/equirect_shader", sk_shader_builtin_equirect);
	material_t convert_material = material_create("default/equirect_convert", convert_shader);
	material_set_texture( convert_material, "source", equirect );

	tex2d_t  face    = tex2d_create("temp/equirect_face", tex_type_rendertarget, equirect->format);
	color32 *data[6] = {};
	int      width   = equirect->height / 2;
	int      height  = width;
	size_t   size    = (size_t)width*(size_t)height*tex2d_format_size(equirect->format);
	tex2d_set_colors(face, width, height, nullptr);
	for (size_t i = 0; i < 6; i++) {
		material_set_vector(convert_material, "up",      { up[i].x, up[i].y, up[i].z, 0 });
		material_set_vector(convert_material, "right",   { right[i].x, right[i].y, right[i].z, 0 });
		material_set_vector(convert_material, "forward", { fwd[i].x, fwd[i].y, fwd[i].z, 0 });

		render_blit   (face, convert_material);
		data[i] = (color32*)malloc(size);
		tex2d_get_data(face, data[i], size);
	}

	result = tex2d_create(equirectangular_file, tex_type_image | tex_type_cubemap, equirect->format);
	tex2d_set_colors(result, width, height, (void**)&data, 6);

	shader_release(convert_shader);
	material_release(convert_material);
	tex2d_release(equirect);
	tex2d_release(face);

	for (size_t i = 0; i < 6; i++) {
		free(data[i]);
	}

	DX11ResName(result->resource, "cubemap_view", equirectangular_file);
	DX11ResName(result->texture,  "cubemap_src", equirectangular_file);
	return result;
}

///////////////////////////////////////////

tex2d_t tex2d_create_cubemap_files(const char **cube_face_file_xxyyzz) {
	tex2d_t result = tex2d_find(cube_face_file_xxyyzz[0]);
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
		data[i] = stbi_load(cube_face_file_xxyyzz[i], &width, &height, &channels, 4);

		// Check if there were issues, or one of the images is the wrong size!
		if (data[i] == nullptr || 
			(final_width  != 0 && final_width  != width ) ||
			(final_height != 0 && final_height != height)) {
			loaded = false;
			log_writef(log_error, "Issue loading cubemap image '%s', file not found, invalid image format, or faces of different sizes?", cube_face_file_xxyyzz[i]);
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
	result = tex2d_create(cube_face_file_xxyyzz[0], tex_type_image | tex_type_cubemap);
	tex2d_set_colors(result, final_width, final_height, (void**)&data, 6);
	for (size_t i = 0; i < 6; i++) {
		free(data[i]);
	}

	DX11ResName(result->resource, "cubemap_view", cube_face_file_xxyyzz[0]);
	DX11ResName(result->texture,  "cubemap_src", cube_face_file_xxyyzz[0]);
	return result;
}

///////////////////////////////////////////

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

	DX11ResName(result->resource, "tex2d_view", id);
	DX11ResName(result->texture,  "tex2d_src",  id);
	return result;
}

///////////////////////////////////////////

void tex2d_release(tex2d_t texture) {
	assets_releaseref(texture->header);
}

///////////////////////////////////////////

void tex2d_destroy(tex2d_t tex) {
	tex2d_releasesurface(tex);
	if (tex->sampler      != nullptr) tex->sampler->Release();
	if (tex->depth_buffer != nullptr) tex2d_release(tex->depth_buffer);
	
	*tex = {};
}

///////////////////////////////////////////

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

///////////////////////////////////////////

void tex2d_set_colors(tex2d_t texture, int32_t width, int32_t height, void **data, int32_t data_count) {
	bool dynamic        = texture->type & tex_type_dynamic;
	bool different_size = texture->width != width || texture->height != height;
	if (texture->texture == nullptr || different_size) {
		tex2d_releasesurface(texture);
		
		texture->width  = width;
		texture->height = height;

		bool result = tex2d_create_surface(texture, data, data_count);
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
		uint8_t *src_line   = (uint8_t *)data[0];
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

///////////////////////////////////////////

void tex2d_set_colors(tex2d_t texture, int32_t width, int32_t height, void *data) {
	void *data_arr[1] = { data };
	tex2d_set_colors(texture, width, height, data_arr, 1);
}

///////////////////////////////////////////

void tex2d_set_options(tex2d_t texture, tex_sample_ sample, tex_address_ address_mode, int32_t anisotropy_level) {
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
	d3d_device->CreateSamplerState(&desc_sampler, &texture->sampler);
}

///////////////////////////////////////////

void tex2d_set_active(tex2d_t texture, int slot) {
	if (texture != nullptr) {
		d3d_context->PSSetSamplers       (slot, 1, &texture->sampler);
		d3d_context->PSSetShaderResources(slot, 1, &texture->resource);
	} else {
		d3d_context->PSSetShaderResources(slot, 0, nullptr);
	}
}

///////////////////////////////////////////

bool tex2d_create_surface(tex2d_t texture, void **data, int32_t data_count) {
	bool mips    = texture->type & tex_type_mips && texture->format == tex_format_rgba32 && texture->width == texture->height;
	bool dynamic = texture->type & tex_type_dynamic;
	bool depth   = texture->type & tex_type_depth;
	bool rtarget = texture->type & tex_type_rendertarget;

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width            = texture->width;
	desc.Height           = texture->height;
	desc.MipLevels        = mips ? log2(texture->width) + 1 : 1;
	desc.ArraySize        = data_count;
	desc.SampleDesc.Count = 1;
	desc.Format           = tex2d_get_native_format(texture->format);
	desc.BindFlags        = depth   ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_SHADER_RESOURCE;
	desc.Usage            = dynamic ? D3D11_USAGE_DYNAMIC      : D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags   = dynamic ? D3D11_CPU_ACCESS_WRITE   : 0;
	if (rtarget)
		desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	if (texture->type & tex_type_cubemap)
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	D3D11_SUBRESOURCE_DATA *tex_mem = (D3D11_SUBRESOURCE_DATA *)malloc(data_count * desc.MipLevels * sizeof(D3D11_SUBRESOURCE_DATA));
	for (size_t i = 0; i < data_count; i++) {
		void *curr_data = data[i];
		
		tex_mem[i*desc.MipLevels].pSysMem     = curr_data;
		tex_mem[i*desc.MipLevels].SysMemPitch = (UINT)(tex2d_format_size(texture->format) * texture->width);

		if (mips) {
			color32 *mip_data      = (color32*)curr_data;
			size_t   mip_data_size = 0;

			int32_t  mip_width  = texture->width;
			int32_t  mip_height = texture->height;
			for (int32_t m = 1; m < desc.MipLevels; m++) {
				int index = i*desc.MipLevels + m;
				tex2d_downsample(mip_data, mip_width, mip_height, (color32**)&tex_mem[index].pSysMem, &mip_width, &mip_height);
				mip_data = (color32*)tex_mem[index].pSysMem;
				tex_mem[index].SysMemPitch = (UINT)(tex2d_format_size(texture->format) * mip_width);
			}
		}
	}
	
	bool result = true;
	if (FAILED(d3d_device->CreateTexture2D(&desc, data == nullptr || data[0] == nullptr ? nullptr : tex_mem, &texture->texture))) {
		log_write(log_error, "Create texture error!");
		result = false;
	}

	// Free any mip-maps we've generated!
	for (size_t i = 0; i < data_count; i++) {
		for (size_t m = 1; m < desc.MipLevels; m++) {
			int index = i*desc.MipLevels + m;
			free((void*)tex_mem[index].pSysMem);
		} 
	}
	free(tex_mem);

	return result;
}

///////////////////////////////////////////

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

///////////////////////////////////////////

bool tex2d_create_views(tex2d_t texture) {
	DXGI_FORMAT format = tex2d_get_native_format(texture->format);
	bool        mips   = texture->type & tex_type_mips && texture->format == tex_format_rgba32 && texture->width == texture->height;

	if (!(texture->type & tex_type_depth)) {
		D3D11_SHADER_RESOURCE_VIEW_DESC res_desc = {};
		res_desc.Format              = format;
		res_desc.Texture2D.MipLevels = mips ? log2(texture->width) + 1 : 1;
		res_desc.ViewDimension       = texture->type & tex_type_cubemap ? D3D11_SRV_DIMENSION_TEXTURECUBE : D3D11_SRV_DIMENSION_TEXTURE2D;
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

///////////////////////////////////////////

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

///////////////////////////////////////////

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

///////////////////////////////////////////

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

///////////////////////////////////////////

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

///////////////////////////////////////////

void tex2d_get_data(tex2d_t texture, void *out_data, size_t out_data_size) {
	// Make sure we've been provided enough memory to hold this texture
	int32_t format_size = tex2d_format_size(texture->format);
	assert(out_data_size == (size_t)texture->width * (size_t)texture->height * (size_t)format_size);

	D3D11_TEXTURE2D_DESC desc             = {};
	ID3D11Texture2D     *copy_tex         = nullptr;
	bool                 copy_tex_release = true;
	texture->texture->GetDesc(&desc);

	// Make sure copy_tex is a texture that we can read from!
	if (desc.SampleDesc.Count > 1) {
		// Not gonna bother with MSAA stuff
		log_write(log_warning, "tex2d_get_data AA surfaces not implemented");
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
			log_write(log_error, "CreateTexture2D failed!");
			return;
		}
		d3d_context->CopyResource(copy_tex, texture->texture);
	}

	// Load the data into CPU RAM
	D3D11_MAPPED_SUBRESOURCE data;
	if (FAILED(d3d_context->Map(copy_tex, 0, D3D11_MAP_READ, 0, &data))) {
		log_write(log_error, "Texture Map failed!");
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

bool tex2d_downsample(color32 *data, int32_t width, int32_t height, color32 **out_data, int32_t *out_width, int32_t *out_height) {
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