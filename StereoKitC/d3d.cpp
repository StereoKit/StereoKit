#pragma comment(lib,"D3D11.lib")
#pragma comment(lib,"Dxgi.lib") // CreateSwapChainForHwnd
#pragma comment(lib,"D3dcompiler.lib") // for shader compile

#include "d3d.h"

ID3D11Device             *d3d_device        = nullptr;
ID3D11DeviceContext      *d3d_context       = nullptr;
ID3D11DepthStencilState  *d3d_depthstate    = nullptr;
ID3D11SamplerState       *d3d_samplerstate  = nullptr;
ID3D11RasterizerState    *d3d_rasterstate   = nullptr;
int                       d3d_screen_width  = 640;
int                       d3d_screen_height = 480;

bool d3d_init() {
	UINT creation_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
	if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, creation_flags, featureLevels, _countof(featureLevels), D3D11_SDK_VERSION, &d3d_device, nullptr, &d3d_context)))
		return false;

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
	d3d_device->CreateDepthStencilState(&desc_depthstate, &d3d_depthstate);
	d3d_context->OMSetDepthStencilState(d3d_depthstate, 1);

	D3D11_SAMPLER_DESC desc_sampler = {};
	desc_sampler.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc_sampler.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc_sampler.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc_sampler.Filter   = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	d3d_device->CreateSamplerState(&desc_sampler, &d3d_samplerstate);
	for (size_t i = 0; i < 4; i++) {
		d3d_context->PSSetSamplers(i, 1, &d3d_samplerstate);
	}
	
	D3D11_RASTERIZER_DESC desc_rasterizer = {};
	desc_rasterizer.FillMode = D3D11_FILL_SOLID;
	desc_rasterizer.CullMode = D3D11_CULL_BACK;
	desc_rasterizer.FrontCounterClockwise = true;
	d3d_device->CreateRasterizerState(&desc_rasterizer, &d3d_rasterstate);
	d3d_context->RSSetState(d3d_rasterstate);

	return true;
}

///////////////////////////////////////////

void d3d_shutdown() {
	if (d3d_context) { d3d_context->Release(); d3d_context = nullptr; }
	if (d3d_device ) { d3d_device->Release();  d3d_device  = nullptr; }
}

void d3d_render_begin() {
	// Set up where on the render target we want to draw, the view has a 
	/*D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.f, 0.f, d3d_screen_width, d3d_screen_height);
	d3d_context->RSSetViewports(1, &viewport);

	// Wipe our swapchain color and depth target clean, and then set them up for rendering!
	float clear[] = { 0, 0, 0, 1 };
	d3d_context->ClearRenderTargetView (d3d_rendertarget, clear);
	d3d_context->ClearDepthStencilView (d3d_depthtarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	d3d_context->OMSetRenderTargets    (1, &d3d_rendertarget, d3d_depthtarget);
	d3d_context->OMSetDepthStencilState(d3d_depthstate, 1);*/
	d3d_context->OMSetDepthStencilState(d3d_depthstate, 1);
	d3d_context->PSSetSamplers(0, 1, &d3d_samplerstate);
}
void d3d_render_end() {
	//d3d_swapchain->Present( 1, 0 );
}