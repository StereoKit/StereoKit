#include "win32.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dxgi1_2.h>

#include "stereokit.h"

#include "texture.h"
#include "render.h"
#include "d3d.h"
#include "input.h"
#include "win32_input.h"

HWND             win32_window    = nullptr;
tex2d_t          win32_target    = {};
IDXGISwapChain1 *win32_swapchain = {};
float            win32_scroll    = 0;

void win32_resize(int width, int height) {
	if (width == d3d_screen_width && height == d3d_screen_height)
		return;
	d3d_screen_width  = width;
	d3d_screen_height = height;
	log_write(log_info, "Resize!");

	if (win32_swapchain != nullptr) {
		tex2d_releasesurface(win32_target);
		win32_swapchain->ResizeBuffers(0, (UINT)d3d_screen_width, (UINT)d3d_screen_height, DXGI_FORMAT_UNKNOWN, 0);
		ID3D11Texture2D *back_buffer;
		win32_swapchain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
		tex2d_setsurface(win32_target, back_buffer);
	}
}

bool win32_init(const char *app_name) {
	MSG      msg     = {0};
	WNDCLASS wc      = {0}; 
	wc.lpfnWndProc = [](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch(message) {
		case WM_CLOSE:     sk_run     = false; PostQuitMessage(0); break;
		case WM_SETFOCUS:  sk_focused = true;  break;
		case WM_KILLFOCUS: sk_focused = false; break;
		case WM_MOUSEWHEEL:win32_scroll += (short)HIWORD(wParam); break;
		case WM_SIZE:       if (wParam != SIZE_MINIMIZED) win32_resize((UINT)LOWORD(lParam), (UINT)HIWORD(lParam)); break;
		case WM_SYSCOMMAND: if ((wParam & 0xfff0) == SC_KEYMENU) return (LRESULT)0; // Disable alt menu
		default: return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return (LRESULT)0;
	};
	wc.hInstance     = GetModuleHandle(NULL);
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = app_name;
	if( !RegisterClass(&wc) ) return false;
	win32_window = CreateWindow(wc.lpszClassName, app_name, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 20, 20, 640, 480, 0, 0, wc.hInstance, nullptr);
	if( !win32_window ) return false;

	// Create a swapchain for the window
	DXGI_SWAP_CHAIN_DESC1 sd = { };
	sd.BufferCount = 2;
	sd.Width       = d3d_screen_width;
	sd.Height      = d3d_screen_height;
	sd.Format      = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
	sd.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.SampleDesc.Count = 1;
	
	IDXGIDevice2  *dxgi_device;  d3d_device  ->QueryInterface(__uuidof(IDXGIDevice2),  (void **)&dxgi_device);
	IDXGIAdapter  *dxgi_adapter; dxgi_device ->GetParent     (__uuidof(IDXGIAdapter),  (void **)&dxgi_adapter);
	IDXGIFactory2 *dxgi_factory; dxgi_adapter->GetParent     (__uuidof(IDXGIFactory2), (void **)&dxgi_factory);

	dxgi_factory->CreateSwapChainForHwnd(d3d_device, win32_window, &sd, nullptr, nullptr, &win32_swapchain);
	ID3D11Texture2D *back_buffer;
	win32_swapchain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));

	win32_target = tex2d_create("stereokit/system/rendertarget", tex_type_rendertarget);
	tex2d_setsurface (win32_target, back_buffer);
	tex2d_add_zbuffer(win32_target);

	dxgi_factory->Release();
	dxgi_adapter->Release();
	dxgi_device ->Release();

	win32_input_init();

	return true;
}
void win32_shutdown() {
	win32_input_shutdown();
	tex2d_release(win32_target);
	win32_swapchain->Release();
}

void win32_step_begin() {
	MSG msg = {0};
	if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage (&msg);
	}
	win32_input_update();
}
void win32_step_end() {
	// Set up where on the render target we want to draw, the view has a 
	D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.f, 0.f, (float)d3d_screen_width, (float)d3d_screen_height);
	d3d_context->RSSetViewports(1, &viewport);

	// Wipe our swapchain color and depth target clean, and then set them up for rendering!
	tex2d_rtarget_clear(win32_target, {0,0,0,255});
	tex2d_rtarget_set_active(win32_target);

	render_draw();

	win32_swapchain->Present(1, 0);

	render_clear();
}