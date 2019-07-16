#include "d3d.h"

#include <thread> // sleep_for

using namespace std;

ID3D11Device             *d3d_device        = nullptr;
ID3D11DeviceContext      *d3d_context       = nullptr;
IDXGISwapChain           *d3d_swapchain     = nullptr;
ID3D11RenderTargetView   *d3d_rendertarget  = nullptr;
ID3D11DepthStencilView   *d3d_depthtarget   = nullptr;
ID3D11DepthStencilState  *d3d_depthstate    = nullptr;
int                       d3d_screen_width  = 640;
int                       d3d_screen_height = 480;

float  d3d_timef = 0;
double d3d_time  = 0;
double d3d_time_start    = 0;
double d3d_time_elapsed  = 0;
float  d3d_time_elapsedf = 0;

void d3d_release_backbuffer();
void d3d_create_backbuffer();

void d3d_init(HWND hwnd) {
	DXGI_SWAP_CHAIN_DESC sd = { 0 };
	sd.BufferCount       = 1;
	sd.BufferDesc.Width  = d3d_screen_width;
	sd.BufferDesc.Height = d3d_screen_height;
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

	d3d_create_backbuffer();

	D3D11_DEPTH_STENCIL_DESC desc_depthstate = {};
	desc_depthstate.DepthEnable    = true;
	desc_depthstate.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	desc_depthstate.DepthFunc      = D3D11_COMPARISON_LESS;
	desc_depthstate.StencilEnable    = true;
	desc_depthstate.StencilReadMask  = 0xFF;
	desc_depthstate.StencilWriteMask = 0xFF;
	desc_depthstate.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	desc_depthstate.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	desc_depthstate.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
	desc_depthstate.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
	desc_depthstate.BackFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	desc_depthstate.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	desc_depthstate.BackFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
	desc_depthstate.BackFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;

	ID3D11DepthStencilState * pDSState;
	d3d_device->CreateDepthStencilState(&desc_depthstate, &d3d_depthstate);
}

///////////////////////////////////////////

void d3d_shutdown() {
	d3d_release_backbuffer();
	if (d3d_context) { d3d_context->Release(); d3d_context = nullptr; }
	if (d3d_device ) { d3d_device->Release();  d3d_device  = nullptr; }
}

void d3d_render_begin() {
	// Set up where on the render target we want to draw, the view has a 
	D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.f, 0.f, d3d_screen_width, d3d_screen_height);
	d3d_context->RSSetViewports(1, &viewport);

	// Wipe our swapchain color and depth target clean, and then set them up for rendering!
	float clear[] = { 0, 0, 0, 1 };
	d3d_context->ClearRenderTargetView (d3d_rendertarget, clear);
	d3d_context->ClearDepthStencilView (d3d_depthtarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	d3d_context->OMSetRenderTargets    (1, &d3d_rendertarget, d3d_depthtarget);
	d3d_context->OMSetDepthStencilState(d3d_depthstate, 1);
}
void d3d_render_end() {
	d3d_swapchain->Present( 1, 0 );
}

void d3d_frame_begin() {
	FILETIME time;
	GetSystemTimePreciseAsFileTime(&time);
	LONGLONG ll_now = (LONGLONG)time.dwLowDateTime + ((LONGLONG)(time.dwHighDateTime) << 32LL);
	double time_curr = ll_now / 10000000.0;

	if (d3d_time_start == 0)
		d3d_time_start = time_curr;
	double new_time = time_curr - d3d_time_start;
	d3d_time_elapsed  = new_time - d3d_time;
	d3d_time          = new_time;
	d3d_time_elapsedf = (float)d3d_time_elapsed;
	d3d_timef         = (float)d3d_time;
}
void d3d_frame_end() {
	this_thread::sleep_for(chrono::milliseconds(1));
}

void d3d_resize_screen(int width, int height) {
	if (width == d3d_screen_width || height == d3d_screen_height)
		return;

	d3d_screen_width  = width;
	d3d_screen_height = height;

	if (d3d_swapchain != nullptr) {
		d3d_release_backbuffer();
		d3d_swapchain->ResizeBuffers(0, (UINT)d3d_screen_width, (UINT)d3d_screen_height, DXGI_FORMAT_UNKNOWN, 0);
		d3d_create_backbuffer();
	}
}

void d3d_release_backbuffer() {
	if (d3d_rendertarget != nullptr) d3d_rendertarget->Release();
	if (d3d_depthtarget  != nullptr) d3d_depthtarget ->Release();
}

void d3d_create_backbuffer() {
	ID3D11Texture2D *back_buffer;
	d3d_swapchain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
	d3d_device   ->CreateRenderTargetView(back_buffer, nullptr, &d3d_rendertarget);
	back_buffer->Release();

	// Create depth buffer
	ID3D11Texture2D     *depth_stencil = nullptr;
	D3D11_TEXTURE2D_DESC desc_depth    = {};
	desc_depth.Width     = d3d_screen_width;
	desc_depth.Height    = d3d_screen_height;
	desc_depth.MipLevels = 1;
	desc_depth.ArraySize = 1;
	desc_depth.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc_depth.Usage     = D3D11_USAGE_DEFAULT;
	desc_depth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	desc_depth.SampleDesc.Count = 1;
	d3d_device->CreateTexture2D( &desc_depth, nullptr, &depth_stencil );

	D3D11_DEPTH_STENCIL_VIEW_DESC desc_depthview = {};
	desc_depthview.Format        = desc_depth.Format;
	desc_depthview.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	d3d_device->CreateDepthStencilView( depth_stencil, &desc_depthview, &d3d_depthtarget );
	depth_stencil->Release();
}