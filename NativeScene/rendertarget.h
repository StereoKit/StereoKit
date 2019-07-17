#pragma once

#include <d3d11.h>

struct rendertarget_t {
	int         width;
	int         height;
	DXGI_FORMAT format;
	ID3D11Texture2D          *texture;
	ID3D11RenderTargetView   *target_view;
	ID3D11DepthStencilView   *depth_view;
	ID3D11ShaderResourceView *shader_view;
};

void rendertarget_release         (rendertarget_t &target);
void rendertarget_resize          (rendertarget_t &target, int width, int height);
void rendertarget_make_views      (rendertarget_t &target);
void rendertarget_set_surface     (rendertarget_t &target, ID3D11Texture2D *surface);
void rendertarget_make_depthbuffer(rendertarget_t &target);