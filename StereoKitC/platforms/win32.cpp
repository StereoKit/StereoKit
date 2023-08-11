/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2023 Nick Klingensmith
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 */

#include "win32.h"

#if defined(SK_OS_WINDOWS)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>


#include "platform.h"
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

struct window_event_t {
	platform_evt_           type;
	platform_evt_data_t     data;
};

struct window_t {
	wchar_t*                title;
	HWND                    handle;
	array_t<window_event_t> events;

	skg_swapchain_t         swapchain;
	bool                    has_swapchain;

	bool                    check_resize;
	int32_t                 resize_x, resize_y;
	RECT                    save_rect;
};

///////////////////////////////////////////

array_t<window_t> win32_windows      = {};

HINSTANCE         win32_hinst        = nullptr;
HICON             win32_icon         = nullptr;
tex_t             win32_target       = {};
float             win32_scroll       = 0;
system_t         *win32_render_sys   = nullptr;
platform_win_t    win32_platform_win = -1;

///////////////////////////////////////////

// Constants for the registry key and value names
const wchar_t* REG_KEY_NAME   = L"Software\\StereoKit Simulator";
const wchar_t* REG_VALUE_NAME = L"WindowLocation";

///////////////////////////////////////////

void    win32_physical_key_interact();
void    win32_target_resize        (tex_t *target, int32_t width, int32_t height);
void    platform_check_events      ();
LRESULT platform_message_callback  (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

///////////////////////////////////////////

bool win32_init() {
	win32_render_sys = systems_find("FrameRender");
	win32_hinst      = GetModuleHandle(NULL);

	// Find the icon from the exe itself
	wchar_t path[MAX_PATH];
	if (GetModuleFileNameW(GetModuleHandle(nullptr), path, MAX_PATH) != 0)
		ExtractIconExW(path, 0, &win32_icon, nullptr, 1);

	return true;
}

///////////////////////////////////////////

void win32_shutdown() {
	for (int32_t i = 0; i < win32_windows.count; i++) {
		platform_win_destroy(i);
	}
	win32_windows.free();

	win32_hinst        = nullptr;
	win32_icon         = nullptr;
	win32_target       = {};
	win32_scroll       = 0;
	win32_render_sys   = nullptr;
	win32_platform_win = -1;
}

///////////////////////////////////////////

bool win32_start_pre_xr() {
	return true;
}

///////////////////////////////////////////

bool win32_start_post_xr() {
	const sk_settings_t *settings = sk_get_settings_ref();

	win32_platform_win = platform_win_make(settings->app_name, recti_t{}, platform_surface_none);
	return win32_platform_win != -1;
}

///////////////////////////////////////////

void win32_step_begin_xr() {
	platform_check_events();

	platform_evt_       evt;
	platform_evt_data_t data;
	while (platform_win_next_event(win32_platform_win, &evt, &data)) {
		switch (evt) {
		case platform_evt_key_press:   input_key_inject_press  (data.press_release); win32_physical_key_interact(); break;
		case platform_evt_key_release: input_key_inject_release(data.press_release); win32_physical_key_interact(); break;
		case platform_evt_character:   input_text_inject_char  (data.character);     break;
		case platform_evt_close:       sk_quit(); break;
		case platform_evt_none: break;
		default: break;
		}
	}
}

///////////////////////////////////////////

bool win32_start_flat() {
	const sk_settings_t* settings = sk_get_settings_ref();

	device_data.display_blend = display_blend_opaque;

	// Get the window size
	RECT rect = {};
	HKEY reg_key;
	if (backend_xr_get_type() == backend_xr_type_simulator && RegOpenKeyExW(HKEY_CURRENT_USER, REG_KEY_NAME, 0, KEY_READ, &reg_key) == ERROR_SUCCESS) {
		DWORD data_size = sizeof(RECT);
		RegQueryValueExW(reg_key, REG_VALUE_NAME, 0, nullptr, (BYTE*)&rect, &data_size);
		RegCloseKey     (reg_key);
	} else {
		rect.left   = settings->flatscreen_pos_x;
		rect.right  = settings->flatscreen_pos_x + settings->flatscreen_width;
		rect.top    = settings->flatscreen_pos_y;
		rect.bottom = settings->flatscreen_pos_y + settings->flatscreen_height;
		AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, false);
	}

	win32_platform_win = platform_win_make(settings->app_name, {rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top}, platform_surface_swapchain);
	if (win32_platform_win == -1)
		return false;

	skg_swapchain_t* swapchain = platform_win_get_swapchain(win32_platform_win);
	win32_target_resize(&win32_target, swapchain->width, swapchain->height);

	return true;
}

///////////////////////////////////////////

void win32_target_resize(tex_t *target, int32_t width, int32_t height) {
	if (*target == nullptr) {
		*target = tex_create(tex_type_rendertarget, tex_format_rgba32);
		tex_set_id       (*target, "sk/platform/swapchain");
		tex_set_color_arr(*target, width, height, nullptr, 1, nullptr, 8);

		tex_t zbuffer = tex_add_zbuffer(*target, render_preferred_depth_fmt());
		tex_set_id (zbuffer, "sk/platform/swapchain_zbuffer");
		tex_release(zbuffer);
	}
	
	device_data.display_width  = width;
	device_data.display_height = height;

	if (width == tex_get_width(*target) && height == tex_get_height(*target))
		return;

	log_diagf("Resizing to: %d<~BLK>x<~clr>%d", width, height);

	tex_set_color_arr(*target, width, height, nullptr, 1, nullptr, 8);
	render_update_projection();
}

///////////////////////////////////////////

void win32_stop_flat() {
	if (backend_xr_get_type() == backend_xr_type_simulator) {
		recti_t r       = platform_win_rect(win32_platform_win);
		RECT    rect    = {r.x, r.y, r.x+r.w, r.y+r.h};
		HKEY    reg_key = {};
		if (RegCreateKeyExW(HKEY_CURRENT_USER, REG_KEY_NAME, 0, nullptr, 0, KEY_WRITE, nullptr, &reg_key, nullptr) == ERROR_SUCCESS) {
			RegSetValueExW(reg_key, REG_VALUE_NAME, 0, REG_BINARY, (const BYTE*)&rect, sizeof(RECT));
			RegCloseKey   (reg_key);
		}
	}

	tex_release(win32_target);

	win32_icon  = nullptr;
	win32_hinst = nullptr;
}

///////////////////////////////////////////

void win32_step_begin_flat() {
	platform_check_events();

	platform_evt_       evt  = {};
	platform_evt_data_t data = {};
	while (platform_win_next_event(win32_platform_win, &evt, &data)) {
		switch (evt) {
		case platform_evt_app_focus:    sk_set_app_focus (data.app_focus); break;
		case platform_evt_key_press:    input_key_inject_press  (data.press_release); win32_physical_key_interact(); break;
		case platform_evt_key_release:  input_key_inject_release(data.press_release); win32_physical_key_interact(); break;
		case platform_evt_character:    input_text_inject_char  (data.character);                                    break;
		case platform_evt_mouse_press:  if (sk_app_focus() == app_focus_active) input_key_inject_press  (data.press_release); break;
		case platform_evt_mouse_release:if (sk_app_focus() == app_focus_active) input_key_inject_release(data.press_release); break;
		case platform_evt_scroll:       if (sk_app_focus() == app_focus_active) win32_scroll += data.scroll;                  break;
		case platform_evt_close:        sk_quit(); break;
		case platform_evt_resize:       win32_target_resize(&win32_target, data.resize.width, data.resize.height); break;
		case platform_evt_none: break;
		default: break;
		}
	}
}

///////////////////////////////////////////

void win32_step_end_flat() {
	skg_event_begin("Setup");

	skg_draw_begin();

	color128 col = render_get_clear_color_ln();
	skg_tex_target_bind(&win32_target->tex);
	skg_target_clear(true, &col.r);

	skg_event_end();
	skg_event_begin("Draw");

	matrix view = render_get_cam_final        ();
	matrix proj = render_get_projection_matrix();
	matrix_inverse(view, view);
	render_draw_matrix(&view, &proj, 1, render_get_filter());
	render_clear();

	skg_event_end();
	skg_event_begin("Present");

	// This copies the color data over to the swapchain, and resolves any
	// multisampling on the primary target texture.
	skg_swapchain_t* swapchain = platform_win_get_swapchain(win32_platform_win);
	skg_tex_copy_to_swapchain(&win32_target->tex, swapchain);

	win32_render_sys->profile_frame_duration = stm_since(win32_render_sys->profile_frame_start);
	skg_swapchain_present(swapchain);

	skg_event_end();
}

///////////////////////////////////////////

void win32_physical_key_interact() {
	// On desktop, we want to hide soft keyboards on physical presses
	input_set_last_physical_keypress_time(time_totalf_unscaled());
	platform_keyboard_show(false, text_context_text);
}

///////////////////////////////////////////

void *win32_hwnd() {
	return win32_windows[win32_platform_win].handle;
}

///////////////////////////////////////////
// Window code                           //
///////////////////////////////////////////

platform_win_t platform_win_make(const char* title, recti_t win_rect, platform_surface_ surface_type) {
	window_t win = {};
	win.title = platform_to_wchar(title);

	WNDCLASSW wc = {};
	wc.lpszClassName = win.title;
	wc.hInstance     = win32_hinst;
	wc.hIcon         = win32_icon;
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.lpfnWndProc   = platform_message_callback;
	if (!RegisterClassW(&wc)) { sk_free(win.title); return -1; }

	RECT rect = {};
	rect.left   = win_rect.x;
	rect.right  = win_rect.x + win_rect.w;
	rect.top    = win_rect.y;
	rect.bottom = win_rect.y + win_rect.h;
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, false);
	if (rect.right == rect.left)   rect.right  = rect.left + win_rect.w;
	if (rect.top   == rect.bottom) rect.bottom = rect.top  + win_rect.h;

	win.handle = CreateWindowW(
		win.title,
		win.title,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		maxi(0, rect.left),
		maxi(0, rect.top),
		rect.right - rect.left,
		rect.bottom - rect.top,
		0, 0,
		wc.hInstance,
		nullptr);

	if (!win.handle) { sk_free(win.title); return -1; }

	// Save the initial size, let's not trust the numbers we asked for.
	RECT r = {};
	if (GetWindowRect(win.handle, &r))
		win.save_rect = r;

	// The final window size can often vary from what we expect, so this just
	// ensures we're working with the correct size.
	GetClientRect(win.handle, &rect);
	int32_t final_width  = maxi(1, rect.right  - rect.left);
	int32_t final_height = maxi(1, rect.bottom - rect.top);
	win.resize_x = final_width;
	win.resize_y = final_height;

	// Not all windows need a swapchain, but here's where we make 'em for those
	// that do.
	if (surface_type == platform_surface_swapchain) {
		skg_tex_fmt_ color_fmt = skg_tex_fmt_rgba32_linear;
		skg_tex_fmt_ depth_fmt = (skg_tex_fmt_)render_preferred_depth_fmt();
#if defined(SKG_OPENGL)
		depth_fmt = skg_tex_fmt_depthstencil;
#endif
		win.swapchain     = skg_swapchain_create(win.handle, color_fmt, skg_tex_fmt_none, final_width, final_height);
		win.has_swapchain = true;

		log_diagf("Created swapchain: %dx%d color:%s depth:%s", win.swapchain.width, win.swapchain.height, render_fmt_name((tex_format_)color_fmt), render_fmt_name((tex_format_)depth_fmt));
	}

	win32_windows.add(win);
	return win32_windows.count - 1;
}

///////////////////////////////////////////

void platform_win_destroy(platform_win_t window) {
	window_t* win = &win32_windows[window];

	if (win->has_swapchain) {
		skg_swapchain_destroy(&win->swapchain);
	}

	if (win->handle) DestroyWindow   (win->handle);
	if (win->title)  UnregisterClassW(win->title, win32_hinst);

	win->events.free();
	sk_free(win->title);
	*win = {};
}

///////////////////////////////////////////

void platform_win_resize(platform_win_t window_id, int32_t width, int32_t height) {
	window_t* win = &win32_windows[window_id];

	width  = maxi(1, width);
	height = maxi(1, height);

	if (win->has_swapchain == false || (width == win->swapchain.width && height == win->swapchain.height))
		return;

	// Save the window position when it changes
	RECT r = {};
	if (GetWindowRect(win->handle, &r))
		win->save_rect = r;

	skg_swapchain_resize(&win->swapchain, width, height);
}

///////////////////////////////////////////

void platform_check_events() {
	MSG msg = { 0 };
	while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage (&msg);
	}
}

///////////////////////////////////////////

LRESULT platform_message_callback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	window_t      *win       = nullptr;
	platform_win_t window_id = -1;
	for (int32_t i = 0; i < win32_windows.count; i++) {
		if (hwnd == win32_windows[i].handle) {
			win       = &win32_windows[i];
			window_id = i;
			break;
		}
	}
	if (win == nullptr)
		return DefWindowProcW(hwnd, message, wParam, lParam);

	window_event_t e = {};
	switch(message) {
	case WM_LBUTTONDOWN: e.type = platform_evt_mouse_press;   e.data.press_release = key_mouse_left;                                                              win->events.add(e); return true;
	case WM_LBUTTONUP:   e.type = platform_evt_mouse_release; e.data.press_release = key_mouse_left;                                                              win->events.add(e); return true;
	case WM_RBUTTONDOWN: e.type = platform_evt_mouse_press;   e.data.press_release = key_mouse_right;                                                             win->events.add(e); return true;
	case WM_RBUTTONUP:   e.type = platform_evt_mouse_release; e.data.press_release = key_mouse_right;                                                             win->events.add(e); return true;
	case WM_MBUTTONDOWN: e.type = platform_evt_mouse_press;   e.data.press_release = key_mouse_center;                                                            win->events.add(e); return true;
	case WM_MBUTTONUP:   e.type = platform_evt_mouse_release; e.data.press_release = key_mouse_center;                                                            win->events.add(e); return true;
	case WM_XBUTTONDOWN: e.type = platform_evt_mouse_press;   e.data.press_release = GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? key_mouse_back : key_mouse_forward; win->events.add(e); return true;
	case WM_XBUTTONUP:   e.type = platform_evt_mouse_release; e.data.press_release = GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? key_mouse_back : key_mouse_forward; win->events.add(e); return true;
	case WM_KEYDOWN:     e.type = platform_evt_key_press;     e.data.press_release = (key_)wParam; win->events.add(e);
		if ((key_)wParam == key_del) { 
			window_event_t e2 = { platform_evt_character }; e2.data.character = 0x7f; win->events.add(e2);
		}
		return true;
	case WM_KEYUP:       e.type = platform_evt_key_release;   e.data.press_release = (key_)wParam;         win->events.add(e); return true;
	case WM_SYSKEYDOWN:  e.type = platform_evt_key_press;     e.data.press_release = (key_)wParam;         win->events.add(e); return true;
	case WM_SYSKEYUP:    e.type = platform_evt_key_release;   e.data.press_release = (key_)wParam;         win->events.add(e); return true;
	case WM_CHAR:        e.type = platform_evt_character;     e.data.character     = (char32_t)wParam;     win->events.add(e); return true;
	case WM_MOUSEWHEEL:  e.type = platform_evt_scroll;        e.data.scroll        = (short)HIWORD(wParam);win->events.add(e); return true;
	case WM_CLOSE:       e.type = platform_evt_close;                                                      win->events.add(e); PostQuitMessage(0); break;
	case WM_SETFOCUS:    e.type = platform_evt_app_focus;     e.data.app_focus     = app_focus_active;     win->events.add(e); break;
	case WM_KILLFOCUS:   e.type = platform_evt_app_focus;     e.data.app_focus     = app_focus_background; win->events.add(e); break;
	case WM_SYSCOMMAND: {
		// Has the user pressed the restore/'un-maximize' button? WM_SIZE
		// happens -after- this event, and contains the new size.
		if (GET_SC_WPARAM(wParam) == SC_RESTORE)
			win->check_resize = true;

		// Disable alt menu
		if (GET_SC_WPARAM(wParam) == SC_KEYMENU)
			return (LRESULT)0;
	} break;
	case WM_MOVE: {
		// Save the window position when it changes
		RECT r = {};
		if (GetWindowRect(win->handle, &r))
			win->save_rect = r;
	} break;
	case WM_SIZE: {
		win->resize_x = (UINT)LOWORD(lParam);
		win->resize_y = (UINT)HIWORD(lParam);

		// Don't check every time the size changes, this can lead to ugly
		// memory alloc. If a restore event, a maximize, or something else says
		// we should resize, check it!
		if (win->check_resize || wParam == SIZE_MAXIMIZED) {
			win->check_resize = false;

			platform_win_resize(window_id, win->resize_x, win->resize_y);
			e.type        = platform_evt_resize;
			e.data.resize = { win->resize_x, win->resize_y };
			win->events.add(e);
		}
	} break;
	case WM_EXITSIZEMOVE: {
		// If the user was dragging the window around, WM_SIZE is called
		// -before- this event, so we can go ahead and resize now!
		platform_win_resize(window_id, win->resize_x, win->resize_y);
		e.type        = platform_evt_resize;
		e.data.resize = { win->resize_x, win->resize_y };
		win->events.add(e);
	}break;
	default: break;
	}
	return DefWindowProcW(hwnd, message, wParam, lParam);
}

///////////////////////////////////////////

bool platform_win_next_event(platform_win_t window_id, platform_evt_* out_event, platform_evt_data_t* out_event_data) {
	window_t *window = &win32_windows[window_id];
	if (window->events.count > 0) {
		*out_event      = window->events[0].type;
		*out_event_data = window->events[0].data;
		window->events.remove(0);
		return true;
	} return false;
}

///////////////////////////////////////////

platform_win_type_ platform_win_type() { return platform_win_type_creatable; }

///////////////////////////////////////////

platform_win_t platform_win_get_existing() { return -1; }

///////////////////////////////////////////

skg_swapchain_t* platform_win_get_swapchain(platform_win_t window) { return win32_windows[window].has_swapchain ? &win32_windows[window].swapchain : nullptr; }

///////////////////////////////////////////

recti_t platform_win_rect(platform_win_t window_id) {
	window_t* win = &win32_windows[window_id];

	// See if we can update it real quick
	RECT r = {};
	if (GetWindowRect(win->handle, &r))
		win->save_rect = r;

	return recti_t{
		win->save_rect.left,
		win->save_rect.top,
		win->save_rect.right  - win->save_rect.left,
		win->save_rect.bottom - win->save_rect.top };
}

} // namespace sk

#endif // defined(SK_OS_WINDOWS)