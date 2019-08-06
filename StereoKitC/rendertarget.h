#pragma once

#include <d3d11.h>

#include "stereokit.h"

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
void rendertarget_set_surface     (rendertarget_t &target, ID3D11Texture2D *surface, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN);
void rendertarget_clear           (rendertarget_t &target, const float *color);
void rendertarget_set_active      (rendertarget_t &target);
void rendertarget_make_depthbuffer(rendertarget_t &target);