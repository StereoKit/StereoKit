#include "stereokit.h"
#include "texture.h"

#include "d3d.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

tex2d_t tex2d_create(const char *id) {
	return (tex2d_t)assets_allocate(asset_type_texture, id);
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
	result = tex2d_create(file);

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
	uint8_t *col_data =  stbi_load_from_memory((stbi_uc*)data, data_size, &width, &height, &channels, 4);

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
	if (tex->resource != nullptr) tex->resource->Release();
	if (tex->texture  != nullptr) tex->texture ->Release();
	*tex = {};
}

void tex2d_set_colors(tex2d_t texture, int32_t width, int32_t height, uint8_t *data_rgba32) {
	if (texture->width != width || texture->height != height || (texture->resource != nullptr && texture->can_write == false)) {
		if (texture->resource != nullptr) {
			texture->resource->Release();
			texture->can_write = true;
		}

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width            = width;
		desc.Height           = height;
		desc.MipLevels        = 1;
		desc.ArraySize        = 1;
		desc.SampleDesc.Count = 1;
		desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;
		desc.Usage            = texture->can_write ? D3D11_USAGE_DYNAMIC    : D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags   = texture->can_write ? D3D11_CPU_ACCESS_WRITE : 0;

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem     = data_rgba32;
		data.SysMemPitch = sizeof(uint8_t) * 4 * width;

		if (FAILED(d3d_device->CreateTexture2D(&desc, &data, &texture->texture))) {
			log_write(log_error, "Create texture error!");
			return;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC res_desc = {};
		res_desc.Format              = desc.Format;
		res_desc.Texture2D.MipLevels = desc.MipLevels;
		res_desc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
		d3d_device->CreateShaderResourceView(texture->texture, &res_desc, &texture->resource);

		texture->width  = width;
		texture->height = height;
		return;
	}
	texture->width  = width;
	texture->height = height;

	D3D11_MAPPED_SUBRESOURCE data = {};
	if (FAILED(d3d_context->Map(texture->texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &data))) {
		log_write(log_error, "Failed mapping a texture");
		return;
	}

	uint8_t *dest_line = (uint8_t*)data.pData;
	uint8_t *src_line  = data_rgba32;
	for (int i = 0; i < height; i++) {
		memcpy(dest_line, src_line, (size_t)width*sizeof(uint8_t)*4);
		dest_line += data.RowPitch;
		src_line  += width * sizeof(uint8_t) * 4;
	}
	d3d_context->Unmap(texture->texture, 0);
}

void tex2d_set_active(tex2d_t texture, int slot) {
	if (texture != nullptr)
		d3d_context->PSSetShaderResources(slot, 1, &texture->resource);
	else
		d3d_context->PSSetShaderResources(slot, 0, nullptr);
}