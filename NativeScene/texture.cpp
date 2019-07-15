#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void tex2d_create(tex2d_t &tex) {
	tex = {};
}
bool tex2d_create_file(tex2d_t &tex, const char *file) {
	tex2d_create(tex);

	int      channels = 0;
	int      width    = 0;
	int      height   = 0;
	uint8_t *data     = stbi_load(file, &width, &height, &channels, 4);

	if (data == nullptr)
		return false;

	tex2d_set_colors(tex, width, height, data);
	return true;
}

void tex2d_set_colors(tex2d_t &tex, int width, int height, uint8_t *data_rgba32) {
	if (tex.width != width || tex.height != height) {
		if (tex.resource != nullptr)
			tex.resource->Release();

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

		if (FAILED(d3d_device->CreateTexture2D(&desc, nullptr, &tex.texture))) {
			printf("Create texture error!\n");
			return;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC res_desc = {};
		res_desc.Format              = desc.Format;
		res_desc.Texture2D.MipLevels = desc.MipLevels;
		res_desc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
		d3d_device->CreateShaderResourceView(tex.texture, &res_desc, &tex.resource);
	}
	tex.width  = width;
	tex.height = height;

	D3D11_MAPPED_SUBRESOURCE data = {};
	if (FAILED(d3d_context->Map(tex.texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &data))) {
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
	d3d_context->Unmap(tex.texture, 0);
}

void tex2d_set_active(tex2d_t &tex, int slot) {
	d3d_context->PSSetShaderResources(slot, 1, &tex.resource);
}