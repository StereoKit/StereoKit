#include "d3d.h"

ID3D11Device             *d3d_device        = nullptr;
ID3D11DeviceContext      *d3d_context       = nullptr;
IDXGISwapChain           *d3d_swapchain     = nullptr;
ID3D11RenderTargetView   *d3d_rendertarget  = nullptr;

void d3d_init(HWND hwnd) {
	DXGI_SWAP_CHAIN_DESC sd = { 0 };
	sd.BufferCount       = 1;
	sd.BufferDesc.Width  = 640;
	sd.BufferDesc.Height = 480;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator   = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow     = hwnd;
	sd.SampleDesc.Count = 1;
	sd.Windowed         = true;

	UINT creation_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
	if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, creation_flags, featureLevels, _countof(featureLevels), D3D11_SDK_VERSION, &sd, &d3d_swapchain, &d3d_device, nullptr, &d3d_context)))
		MessageBox(0, "\tFailed to init d3d!\n", "Error", 0);

	ID3D11Texture2D *backbuffer;
	d3d_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)& backbuffer);
	d3d_device   ->CreateRenderTargetView( backbuffer, NULL, &d3d_rendertarget );
	backbuffer->Release();

}

///////////////////////////////////////////

void d3d_shutdown() {

	if (d3d_rendertarget) { d3d_rendertarget->Release(); d3d_rendertarget = nullptr; }
	if (d3d_context) { d3d_context->Release(); d3d_context = nullptr; }
	if (d3d_device ) { d3d_device->Release();  d3d_device  = nullptr; }
}

void d3d_render_begin() {
	// Set up where on the render target we want to draw, the view has a 
	D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.f, 0.f, 640.f, 480.f);
	d3d_context->RSSetViewports(1, &viewport);

	// Wipe our swapchain color and depth target clean, and then set them up for rendering!
	float clear[] = { 0, 0, 0, 1 };
	d3d_context->ClearRenderTargetView(d3d_rendertarget, clear);
	//d3d_context->ClearDepthStencilView(surface.depth_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	d3d_context->OMSetRenderTargets(1, &d3d_rendertarget, nullptr);
}
void d3d_render_end() {
	d3d_swapchain->Present( 1, 0 );
}