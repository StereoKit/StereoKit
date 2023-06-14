#include "win32.h"

#if defined(SK_OS_WINDOWS)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include "../stereokit.h"
#include "../_stereokit.h"
#include "../device.h"
#include "../sk_math.h"
#include "../asset_types/texture.h"
#include "../libraries/sokol_time.h"
#include "../libraries/stref.h"
#include "../systems/system.h"
#include "../systems/render.h"
#include "../systems/input_keyboard.h"
#include "../hands/input_hand.h"

namespace sk {

///////////////////////////////////////////

HWND            win32_window              = nullptr;
HINSTANCE       win32_hinst               = nullptr;
HICON           win32_icon                = nullptr;
skg_swapchain_t win32_swapchain           = {};
bool            win32_swapchain_initialized = false;
tex_t           win32_target              = {};
float           win32_scroll              = 0;
LONG_PTR        win32_openxr_base_winproc = 0;
system_t       *win32_render_sys          = nullptr;

// For managing window resizing
bool win32_check_resize = true;
UINT win32_resize_x     = 0;
UINT win32_resize_y     = 0;

#if defined(SKG_OPENGL)
const int32_t win32_multisample = 1;
#else
const int32_t win32_multisample = 8;
#endif

// Constants for the registry key and value names
const wchar_t* REG_KEY_NAME   = L"Software\\StereoKit Simulator";
const wchar_t* REG_VALUE_NAME = L"WindowLocation";

///////////////////////////////////////////

void win32_resize(int width, int height) {
	if (win32_swapchain_initialized == false || (width == device_data.display_width && height == device_data.display_height))
		return;
	sk_info.display_width  = width;
	sk_info.display_height = height;
	device_data.display_width  = width;
	device_data.display_height = height;
	log_diagf("Resized to: %d<~BLK>x<~clr>%d", width, height);
	
	skg_swapchain_resize(&win32_swapchain, device_data.display_width, device_data.display_height);
	tex_set_color_arr   (win32_target,     device_data.display_width, device_data.display_height, nullptr, 1, nullptr, win32_multisample);
	render_update_projection();
}

///////////////////////////////////////////

void win32_physical_key_interact() {
	// On desktop, we want to hide soft keyboards on physical presses
	input_last_physical_keypress = time_totalf_unscaled();
	platform_keyboard_show(false, text_context_text);
}

///////////////////////////////////////////

bool win32_window_message_common(UINT message, WPARAM wParam, LPARAM lParam) {
	switch(message) {
	case WM_LBUTTONDOWN: if (sk_focus == app_focus_active) input_key_inject_press  (key_mouse_left);   return true;
	case WM_LBUTTONUP:   if (sk_focus == app_focus_active) input_key_inject_release(key_mouse_left);   return true;
	case WM_RBUTTONDOWN: if (sk_focus == app_focus_active) input_key_inject_press  (key_mouse_right);  return true;
	case WM_RBUTTONUP:   if (sk_focus == app_focus_active) input_key_inject_release(key_mouse_right);  return true;
	case WM_MBUTTONDOWN: if (sk_focus == app_focus_active) input_key_inject_press  (key_mouse_center); return true;
	case WM_MBUTTONUP:   if (sk_focus == app_focus_active) input_key_inject_release(key_mouse_center); return true;
	case WM_XBUTTONDOWN: input_key_inject_press  (GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? key_mouse_back : key_mouse_forward); return true;
	case WM_XBUTTONUP:   input_key_inject_release(GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? key_mouse_back : key_mouse_forward); return true;
	case WM_KEYDOWN:     input_key_inject_press  ((key_)wParam); win32_physical_key_interact(); if ((key_)wParam == key_del) input_text_inject_char(0x7f); return true;
	case WM_KEYUP:       input_key_inject_release((key_)wParam); win32_physical_key_interact(); return true;
	case WM_SYSKEYDOWN:  input_key_inject_press  ((key_)wParam); win32_physical_key_interact(); return true;
	case WM_SYSKEYUP:    input_key_inject_release((key_)wParam); win32_physical_key_interact(); return true;
	case WM_CHAR:        input_text_inject_char   ((uint32_t)wParam); return true;
	case WM_MOUSEWHEEL:  if (sk_focus == app_focus_active) win32_scroll += (short)HIWORD(wParam); return true;
	default: return false;
	}
}

///////////////////////////////////////////

LRESULT win32_openxr_winproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (!win32_window_message_common(message, wParam, lParam)) {
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

///////////////////////////////////////////

bool win32_start_pre_xr() {
	return true;
}

///////////////////////////////////////////

bool win32_start_post_xr() {
	wchar_t *app_name_w = platform_to_wchar(sk_app_name);

	// Create a window just to grab input
	WNDCLASSW wc = {0}; 
	wc.lpfnWndProc   = [](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		if (!win32_window_message_common(message, wParam, lParam)) {
			switch (message) {
			case WM_CLOSE: sk_quit(); PostQuitMessage(0); break;
			default: return DefWindowProcW(hWnd, message, wParam, lParam);
			}
		}
		return (LRESULT)0;
	};
	wc.hInstance     = GetModuleHandle(NULL);
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = app_name_w;
	wc.hIcon         = win32_icon;
	if( !RegisterClassW(&wc) ) return false;

	win32_window = CreateWindowW(
		app_name_w,
		app_name_w,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		0, 0, 0, 0,
		0, 0, 
		wc.hInstance,
		nullptr);

	sk_free(app_name_w);
	if (!win32_window) {
		return false;
	}
	return true;
}

///////////////////////////////////////////

bool win32_init() {
	win32_render_sys = systems_find("FrameRender");

	// Find the icon from the exe itself
	wchar_t path[MAX_PATH];
	if (GetModuleFileNameW(GetModuleHandle(nullptr), path, MAX_PATH) != 0)
		ExtractIconExW(path, 0, &win32_icon, nullptr, 1);

	return true;
}

///////////////////////////////////////////

void win32_shutdown() {
}

///////////////////////////////////////////

bool win32_start_flat() {
	sk_info.display_type      = display_opaque;
	device_data.display_blend = display_blend_opaque;

	wchar_t *app_name_w = platform_to_wchar(sk_app_name);

	WNDCLASSW wc = {0};
	wc.lpfnWndProc   = [](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		if (!win32_window_message_common(message, wParam, lParam)) {
			switch(message) {
			case WM_CLOSE:      sk_quit(); PostQuitMessage(0);   break;
			case WM_SETFOCUS:   sk_focus = app_focus_active;     break;
			case WM_KILLFOCUS:  sk_focus = app_focus_background; break;
			case WM_MOUSEWHEEL: if (sk_focus == app_focus_active) win32_scroll += (short)HIWORD(wParam); break;
			case WM_SYSCOMMAND: {
				// Has the user pressed the restore/'un-maximize' button?
				// WM_SIZE happens -after- this event, and contains the new size.
				if (GET_SC_WPARAM(wParam) == SC_RESTORE)
					win32_check_resize = true;
			
				// Disable alt menu
				if (GET_SC_WPARAM(wParam) == SC_KEYMENU) 
					return (LRESULT)0; 
			} return DefWindowProcW(hWnd, message, wParam, lParam); 
			case WM_SIZE: {
				win32_resize_x = (UINT)LOWORD(lParam);
				win32_resize_y = (UINT)HIWORD(lParam);

				// Don't check every time the size changes, this can lead to ugly memory alloc.
				// If a restore event, a maximize, or something else says we should resize, check it!
				if (win32_check_resize || wParam == SIZE_MAXIMIZED) { 
					win32_check_resize = false; 
					win32_resize(win32_resize_x, win32_resize_y); 
				}
			} return DefWindowProcW(hWnd, message, wParam, lParam);
			case WM_EXITSIZEMOVE: {
				// If the user was dragging the window around, WM_SIZE is called -before- this 
				// event, so we can go ahead and resize now!
				win32_resize(win32_resize_x, win32_resize_y);
			} return DefWindowProcW(hWnd, message, wParam, lParam);
			default: return DefWindowProcW(hWnd, message, wParam, lParam);
			}
		}
		return (LRESULT)0;
	};
	wc.hInstance     = GetModuleHandleW(NULL);
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = app_name_w;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon         = win32_icon;
	if (!RegisterClassW(&wc)) { sk_free(app_name_w); return false; }
	win32_hinst = wc.hInstance;

	RECT rect = {};
	HKEY reg_key;
	if (backend_xr_get_type() == backend_xr_type_simulator && RegOpenKeyExW(HKEY_CURRENT_USER, REG_KEY_NAME, 0, KEY_READ, &reg_key) == ERROR_SUCCESS) {
		DWORD data_size = sizeof(RECT);
		RegQueryValueExW(reg_key, REG_VALUE_NAME, 0, nullptr, (BYTE*)&rect, &data_size);
		RegCloseKey     (reg_key);
	} else {
		rect.left   = sk_settings.flatscreen_pos_x;
		rect.right  = sk_settings.flatscreen_pos_x + sk_settings.flatscreen_width;
		rect.top    = sk_settings.flatscreen_pos_y;
		rect.bottom = sk_settings.flatscreen_pos_y + sk_settings.flatscreen_height;
		AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, false);
	}
	if (rect.right == rect.left) rect.right  = rect.left + sk_settings.flatscreen_width;
	if (rect.top == rect.bottom) rect.bottom = rect.top  + sk_settings.flatscreen_height;
	
	win32_window = CreateWindowW(
		app_name_w,
		app_name_w,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		maxi(0,rect.left),
		maxi(0,rect.top),
		rect.right  - rect.left,
		rect.bottom - rect.top,
		0, 0, 
		wc.hInstance,
		nullptr);

	sk_free(app_name_w);

	if( !win32_window ) return false;

	RECT bounds;
	GetClientRect(win32_window, &bounds);
	int32_t width  = bounds.right  - bounds.left;
	int32_t height = bounds.bottom - bounds.top;

	skg_tex_fmt_ color_fmt = skg_tex_fmt_rgba32_linear;
	skg_tex_fmt_ depth_fmt = (skg_tex_fmt_)render_preferred_depth_fmt();
#if defined(SKG_OPENGL)
	depth_fmt = skg_tex_fmt_depthstencil;
#endif
	win32_swapchain = skg_swapchain_create(win32_window, color_fmt, skg_tex_fmt_none, width, height);
	win32_swapchain_initialized = true;
	sk_info.display_width  = win32_swapchain.width;
	sk_info.display_height = win32_swapchain.height;
	device_data.display_width  = win32_swapchain.width;
	device_data.display_height = win32_swapchain.height;
	win32_target = tex_create(tex_type_rendertarget, tex_format_rgba32);
	tex_set_id       (win32_target, "sk/platform/swapchain");
	tex_set_color_arr(win32_target, sk_info.display_width, sk_info.display_height, nullptr, 1, nullptr, win32_multisample);

	tex_t zbuffer = tex_add_zbuffer (win32_target, (tex_format_)depth_fmt);
	tex_set_id (zbuffer, "sk/platform/swapchain_zbuffer");
	tex_release(zbuffer);

	log_diagf("Created swapchain: %dx%d color:%s depth:%s", win32_swapchain.width, win32_swapchain.height, render_fmt_name((tex_format_)color_fmt), render_fmt_name((tex_format_)depth_fmt));
	render_update_projection();

	return true;
}

///////////////////////////////////////////

void win32_stop_flat() { 
	if (backend_xr_get_type() == backend_xr_type_simulator) {
		RECT rect;
		HKEY reg_key;
		GetWindowRect(win32_window, &rect);
		if (RegCreateKeyExW(HKEY_CURRENT_USER, REG_KEY_NAME, 0, nullptr, 0, KEY_WRITE, nullptr, &reg_key, nullptr) == ERROR_SUCCESS) {
			RegSetValueExW(reg_key, REG_VALUE_NAME, 0, REG_BINARY, (const BYTE*)&rect, sizeof(RECT));
			RegCloseKey   (reg_key);
		}
	}

	tex_release          (win32_target);
	skg_swapchain_destroy(&win32_swapchain);
	win32_swapchain_initialized = false;


	if (win32_icon)   DestroyIcon  (win32_icon);
	if (win32_window) DestroyWindow(win32_window);
	wchar_t* app_name_w = platform_to_wchar(sk_app_name);
	UnregisterClassW(app_name_w, win32_hinst);
	sk_free(app_name_w);

	win32_icon   = nullptr;
	win32_window = nullptr;
	win32_hinst  = nullptr;
}

///////////////////////////////////////////

void win32_step_begin_xr() {
	MSG msg = {0};
	while (PeekMessage(&msg, win32_window, 0U, 0U, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage (&msg);
	}
}

///////////////////////////////////////////

void win32_step_begin_flat() {
	MSG msg = {0};
	while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage (&msg);
	}
}

///////////////////////////////////////////

void win32_step_end_flat() {
	skg_draw_begin();

	color128 col = render_get_clear_color_ln();
#if defined(SKG_OPENGL)
	skg_swapchain_bind(&win32_swapchain);
#else
	skg_tex_target_bind(&win32_target->tex);
#endif
	skg_target_clear(true, &col.r);

	matrix view = render_get_cam_final        ();
	matrix proj = render_get_projection_matrix();
	matrix_inverse(view, view);
	render_draw_matrix(&view, &proj, 1, render_get_filter());
	render_clear();

#if !defined(SKG_OPENGL)
	// This copies the color data over to the swapchain, and resolves any
	// multisampling on the primary target texture.
	skg_tex_copy_to_swapchain(&win32_target->tex, &win32_swapchain);
#endif

	win32_render_sys->profile_frame_duration = stm_since(win32_render_sys->profile_frame_start);
	skg_swapchain_present(&win32_swapchain);
}

///////////////////////////////////////////

void *win32_hwnd() {
	return win32_window;
}

} // namespace sk

#endif // defined(SK_OS_WINDOWS)