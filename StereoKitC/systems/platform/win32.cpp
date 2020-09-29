#if defined(_WIN32) && !defined(WINDOWS_UWP)

#include "win32.h"
#include "flatscreen_input.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../../stereokit.h"
#include "../../_stereokit.h"
#include "../../asset_types/texture.h"
#include "../render.h"
#include "../input.h"
#include "../hand/input_hand.h"

namespace sk {

///////////////////////////////////////////

HWND            win32_window    = nullptr;
skr_swapchain_t win32_swapchain = {};
float           win32_scroll    = 0;

// For managing window resizing
bool win32_check_resize = true;
bool win32_resize_drag  = false;
UINT win32_resize_x     = 0;
UINT win32_resize_y     = 0;

///////////////////////////////////////////

void win32_resize(int width, int height) {
	if (width == sk_info.display_width && height == sk_info.display_height)
		return;
	sk_info.display_width  = width;
	sk_info.display_height = height;
	log_diagf("Resized to: %d<~BLK>x<~clr>%d", width, height);
	
	skr_swapchain_resize(&win32_swapchain, sk_info.display_width, sk_info.display_height);
	render_update_projection();
}

///////////////////////////////////////////

bool win32_init(void *from_window) {
	sk_info.display_width  = sk_settings.flatscreen_width;
	sk_info.display_height = sk_settings.flatscreen_height;
	sk_info.display_type   = display_opaque;

	WNDCLASS wc = {0}; 
	wc.lpfnWndProc = [](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch(message) {
		case WM_CLOSE:     sk_run     = false; PostQuitMessage(0); break;
		case WM_SETFOCUS:  sk_focused = true;  break;
		case WM_KILLFOCUS: sk_focused = false; break;
		case WM_MOUSEWHEEL:win32_scroll += (short)HIWORD(wParam); break;
		case WM_SYSCOMMAND: {
			// Has the user pressed the restore/'un-maximize' button?
			// WM_SIZE happens -after- this event, and contains the new size.
			if (wParam == SC_RESTORE) {
				win32_check_resize = true;
			}
			// Disable alt menu
			if ((wParam & 0xfff0) == SC_KEYMENU) 
				return (LRESULT)0; 
		} return DefWindowProc(hWnd, message, wParam, lParam); 
		case WM_SIZE: {
			win32_resize_x = (UINT)LOWORD(lParam);
			win32_resize_y = (UINT)HIWORD(lParam);

			// Don't check every time the size changes, this can lead to ugly memory alloc.
			// If a restore event, a maximize, or something else says we should resize, check it!
			if (win32_check_resize || wParam == SIZE_MAXIMIZED) { 
				win32_check_resize = false; 
				win32_resize(win32_resize_x, win32_resize_y); 
			}
		} return DefWindowProc(hWnd, message, wParam, lParam);
		case WM_EXITSIZEMOVE: {
			// If the user was dragging the window around, WM_SIZE is called -before- this 
			// event, so we can go ahead and resize now!
			if (win32_resize_drag) {
				win32_resize_drag = false;
				win32_resize(win32_resize_x, win32_resize_y);
			}
		} return DefWindowProc(hWnd, message, wParam, lParam);
		case WM_ENTERSIZEMOVE: win32_resize_drag = true; return DefWindowProc(hWnd, message, wParam, lParam);
		default: return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return (LRESULT)0;
	};
	wc.hInstance     = GetModuleHandle(NULL);
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = sk_app_name;
	if( !RegisterClass(&wc) ) return false;

	RECT r;
	r.left   = sk_settings.flatscreen_pos_x;
	r.right  = sk_settings.flatscreen_pos_x + sk_info.display_width;
	r.top    = sk_settings.flatscreen_pos_y;
	r.bottom = sk_settings.flatscreen_pos_y + sk_info.display_height;
	AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW | WS_VISIBLE, false);
	win32_window = CreateWindow(
		wc.lpszClassName, 
		sk_app_name, 
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
		max(0,r.left), 
		max(0,r.top), 
		r.right  - r.left, 
		r.bottom - r.top, 
		0, 0, 
		wc.hInstance, 
		nullptr);
	if( !win32_window ) return false;

	skr_callback_log([](skr_log_ level, const char *text) {
		switch (level) {
		case skr_log_info:     log_diagf("sk_gpu: %s", text); break;
		case skr_log_warning:  log_warnf("sk_gpu: %s", text); break;
		case skr_log_critical: log_errf ("sk_gpu: %s", text); break;
		}
	});
	if (!skr_init(sk_app_name, win32_window, nullptr))
		return false;
	win32_swapchain = skr_swapchain_create(skr_tex_fmt_rgba32_linear, skr_tex_fmt_depth16, sk_info.display_width, sk_info.display_height);

	flatscreen_input_init();

	return true;
}

///////////////////////////////////////////

void win32_shutdown() {
	flatscreen_input_shutdown();
	skr_swapchain_destroy(&win32_swapchain);
	skr_shutdown();
}

///////////////////////////////////////////

void win32_step_begin() {
	MSG msg = {0};
	if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage (&msg);
	}
	flatscreen_input_update();
}

///////////////////////////////////////////

void win32_step_end() {
	skr_draw_begin();

	color128   col    = render_get_clear_color();
	skr_tex_t *target = skr_swapchain_get_next(&win32_swapchain);
	skr_tex_target_bind(target, true, &col.r);

	input_update_predicted();

	matrix view = render_get_cam_root  ();
	matrix proj = render_get_projection();
	matrix_inverse(view, view);
	render_draw_matrix(&view, &proj, 1);
	render_clear();
}

///////////////////////////////////////////

void win32_vsync() {
	skr_swapchain_present(&win32_swapchain);
}

///////////////////////////////////////////

void *win32_hwnd() {
	return win32_window;
}

} // namespace sk

#endif // defined(_WIN32) && !defined(WINDOWS_UWP)