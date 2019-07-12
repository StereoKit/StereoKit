#pragma once

#include <d3d11.h>

extern ID3D11Device           *d3d_device;
extern ID3D11DeviceContext    *d3d_context;
extern IDXGISwapChain         *d3d_swapchain;
extern ID3D11RenderTargetView *d3d_rendertarget;

void d3d_init        (HWND hwnd);
void d3d_shutdown    ();
void d3d_render_begin();
void d3d_render_end  ();