#include "d3d.h"

ID3D11Device             *d3d_device        = nullptr;
ID3D11DeviceContext      *d3d_context       = nullptr;
IDXGISwapChain           *d3d_swapchain     = nullptr;
ID3D11RenderTargetView   *d3d_rendertarget  = nullptr;