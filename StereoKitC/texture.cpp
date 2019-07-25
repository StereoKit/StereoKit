#include "stereokit.h"
#include "texture.h"

#include "d3d.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

tex2d_t tex2d_create(const char *name) {
	return (tex2d_t)assets_allocate(asset_type_texture, name);
}
tex2d_t tex2d_create_file(const char *file) {
	tex2d_t result = (tex2d_t)assets_find(file);
	if (result != nullptr) {
		assets_addref(result->header);
		return result;
	}

	int      channels = 0;
	int      width    = 0;
	int      height   = 0;
	uint8_t *data     = stbi_load(file, &width, &height, &channels, 4);

	if (data == nullptr) {
		return nullptr;
	}
	result = tex2d_create(file);

	tex2d_set_colors(result, width, height, data);
	return result;
}
tex2d_t tex2d_create_mem(const char *id, void *data, size_t data_size) {
	tex2d_t result = (tex2d_t)assets_find(id);
	if (result != nullptr) {
		assets_addref(result->header);
		return result;
	}

	int      channels = 0;
	int      width    = 0;
	int      height   = 0;
	uint8_t *col_data =  stbi_load_from_memory((stbi_uc*)data, data_size, &width, &height, &channels, 4);

	if (col_data == nullptr) {
		return nullptr;
	}
	result = tex2d_create(id);

	tex2d_set_colors(result, width, height, col_data);
	return result;
}

void tex2d_release(tex2d_t tex) {
	assets_releaseref(tex->header);
}
void tex2d_destroy(tex2d_t tex) {
	if (tex->resource != nullptr) tex->resource->Release();
	if (tex->texture  != nullptr) tex->texture ->Release();
	*tex = {};
}

void tex2d_set_colors(tex2d_t tex, int width, int height, uint8_t *data_rgba32) {
	if (tex->width != width || tex->height != height) {
		if (tex->resource != nullptr)
			tex->resource->Release();

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width            = width;
		desc.Height           = height;
		desc.MipLevels        = 1;
		desc.ArraySize        = 1;
		desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage            = D3D11_USAGE_DYNAMIC;
		desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags   = D3D11_CPU_ACCESS_WRITE;

		if (FAILED(d3d_device->CreateTexture2D(&desc, nullptr, &tex->texture))) {
			printf("Create texture error!\n");
			return;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC res_desc = {};
		res_desc.Format              = desc.Format;
		res_desc.Texture2D.MipLevels = desc.MipLevels;
		res_desc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
		d3d_device->CreateShaderResourceView(tex->texture, &res_desc, &tex->resource);
	}
	tex->width  = width;
	tex->height = height;

	D3D11_MAPPED_SUBRESOURCE data = {};
	if (FAILED(d3d_context->Map(tex->texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &data))) {
		printf("Failed mapping a texture\n");
		return;
	}

	uint8_t *dest_line = (uint8_t*)data.pData;
	uint8_t *src_line  = data_rgba32;
	for (int i = 0; i < height; i++) {
		memcpy(dest_line, src_line, (size_t)width*sizeof(uint8_t)*4);
		dest_line += data.RowPitch;
		src_line  += width * sizeof(uint8_t) * 4;
	}
	d3d_context->Unmap(tex->texture, 0);
}

void tex2d_set_active(tex2d_t tex, int slot) {
	d3d_context->PSSetShaderResources(slot, 1, &tex->resource);
}