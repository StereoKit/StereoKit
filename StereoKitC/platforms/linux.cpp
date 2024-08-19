/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
 */

#include "linux.h"
#if defined(SK_OS_LINUX)

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/extensions/Xfixes.h>

#include <unistd.h>
#include <dirent.h>
#include <libgen.h>
#include <fontconfig/fontconfig.h>

#include <sk_gpu.h>

#include "../xr_backends/openxr.h"
#include "../systems/render.h"
#include "../systems/input_keyboard.h"
#include "../systems/system.h"
#include "../_stereokit.h"
#include "../device.h"
#include "../log.h"
#include "../sk_math.h"
#include "../asset_types/texture.h"
#include "../libraries/sokol_time.h"
#include "../libraries/unicode.h"
#include "../libraries/stref.h"
#include "string.h"
#include "sys/stat.h"


namespace sk {

///////////////////////////////////////////

struct window_event_t {
	platform_evt_           type;
	platform_evt_data_t     data;
};

struct window_t {
	char*                   title;
	array_t<window_event_t> events;

	Window                  x_window;

	skg_swapchain_t         swapchain;
	bool                    has_swapchain;
};

///////////////////////////////////////////

array_t<window_t> linux_windows = {};
Display*          linux_display = nullptr;

// GLX/Xlib

XIM x_linux_input_method;
XIC x_linux_input_context;

// Are we X or XWayland?
bool xwayland = false;

key_  linux_xk_map_upper[256] = {};
key_  linux_xk_map_lower[256] = {};
float linux_scroll  = 0;
int   linux_mouse_x = 0;
int   linux_mouse_y = 0;
bool  linux_mouse_in_window = false;

///////////////////////////////////////////

void linux_init_key_lookups();
void platform_check_events ();

///////////////////////////////////////////

bool platform_impl_init() {
	linux_init_key_lookups();

	xwayland = getenv("WAYLAND_DISPLAY") != nullptr;

	return true;
}

///////////////////////////////////////////

void platform_impl_shutdown() {
	for (int32_t i = 0; i < linux_windows.count; i++) {
		platform_win_destroy(i);
	}
	linux_windows.free();

	// We aren't closing the display here, because EGL seems to close it for us
	// in skg_shutdown
	linux_display = nullptr;
}

///////////////////////////////////////////

void platform_impl_step() {
	platform_check_events();
}


///////////////////////////////////////////
// Window code                           //
///////////////////////////////////////////

platform_win_type_ platform_win_type() { return platform_win_type_creatable; }

///////////////////////////////////////////

platform_win_t platform_win_get_existing(platform_surface_ surface_type) { return -1; }

///////////////////////////////////////////

platform_win_t platform_win_make(const char* title, recti_t win_rect, platform_surface_ surface_type) {
	window_t win = {};

	if (linux_display == nullptr) linux_display = XOpenDisplay(nullptr);
	if (linux_display == nullptr) {
		log_fail_reason(90, log_error, "Cannot connect to X server");
		return -1;
	}
	Window x_root = DefaultRootWindow(linux_display);
	if (x_root == 0) {
		log_fail_reason(90, log_error, "No root window found!");
		return -1;
	}

	XSetWindowAttributes window_attribs = {};
	window_attribs.event_mask = ExposureMask | KeyPressMask;
	win.x_window = XCreateWindow(
		linux_display, x_root,
		win_rect.x, win_rect.y, win_rect.w, win_rect.h, 0,
		CopyFromParent, InputOutput, CopyFromParent,
		CWColormap | CWEventMask, &window_attribs);

	XSizeHints *hints = XAllocSizeHints();
	if (hints == nullptr) {
		log_err("XAllocSizeHints failed.");
	} else {
		hints->flags      = PMinSize;
		hints->min_width  = 100;
		hints->min_height = 100;
		XSetWMNormalHints(linux_display, win.x_window, hints);
		XSetWMSizeHints  (linux_display, win.x_window, hints, PMinSize);
	}

	XMapWindow     (linux_display, win.x_window);
	XChangeProperty(linux_display, win.x_window,
		XInternAtom(linux_display, "_NET_WM_NAME", False),
		XInternAtom(linux_display, "UTF8_STRING",  False), 8, PropModeReplace,
		(unsigned char *)title, strlen(title));

	// loads the XMODIFIERS environment variable to see what input method to use
	XSetLocaleModifiers("");
	x_linux_input_method = XOpenIM(linux_display, 0, 0, 0);
	if (!x_linux_input_method) {
		// fallback to internal input method
		XSetLocaleModifiers("@im=none");
		x_linux_input_method = XOpenIM(linux_display, 0, 0, 0);
	}

	// X input context, you can have multiple for text boxes etc, but having a
	// single one is the easiest.
	x_linux_input_context = XCreateIC(
		x_linux_input_method,
		XNInputStyle,   XIMPreeditNothing | XIMStatusNothing,
		XNClientWindow, win.x_window,
		XNFocusWindow,  win.x_window,
		nullptr);
	XSetICFocus(x_linux_input_context);

	// Setup for window events
	XSelectInput(linux_display, win.x_window, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask | EnterWindowMask | LeaveWindowMask | FocusChangeMask);
	Atom wm_delete = XInternAtom(linux_display, "WM_DELETE_WINDOW", true);
	XSetWMProtocols(linux_display, win.x_window, &wm_delete, 1);

	if (surface_type == platform_surface_swapchain) {
		skg_tex_fmt_ color_fmt = skg_tex_fmt_rgba32_linear;
		skg_tex_fmt_ depth_fmt = (skg_tex_fmt_)render_preferred_depth_fmt();
		win.swapchain     = skg_swapchain_create((void *)win.x_window, color_fmt, depth_fmt, win_rect.w, win_rect.h);
		win.has_swapchain = true;

		log_diagf("Created swapchain: %dx%d color:%s depth:%s", win.swapchain.width, win.swapchain.height, render_fmt_name((tex_format_)color_fmt), render_fmt_name((tex_format_)depth_fmt));
	}

	linux_windows.add(win);
	return linux_windows.count - 1;
}

///////////////////////////////////////////

void platform_win_destroy(platform_win_t window_id) {
	window_t* win = &linux_windows[window_id];

	if (win->has_swapchain) {
		skg_swapchain_destroy(&win->swapchain);
	}
	XDestroyWindow(linux_display, win->x_window);
	win->events.free();

	*win = {};
}

///////////////////////////////////////////

void platform_win_resize(platform_win_t window_id, int32_t width, int32_t height) {
	window_t* win = &linux_windows[window_id];

	width  = maxi(1, width);
	height = maxi(1, height);

	if (win->has_swapchain == false || (width == win->swapchain.width && height == win->swapchain.height))
		return;

	skg_swapchain_resize(&win->swapchain, width, height);
}

///////////////////////////////////////////

void linux_init_key_lookups() {
	linux_xk_map_upper[0xFF & XK_BackSpace] = key_backspace;
	linux_xk_map_upper[0xFF & XK_Tab      ] = key_tab;
	linux_xk_map_upper[0xFF & XK_Linefeed ] = key_return;
	linux_xk_map_upper[0xFF & XK_Return   ] = key_return;
	linux_xk_map_upper[0xFF & XK_Escape   ] = key_esc;
	linux_xk_map_upper[0xFF & XK_Delete   ] = key_del;
	linux_xk_map_upper[0xFF & XK_Home     ] = key_home;
	linux_xk_map_upper[0xFF & XK_Left     ] = key_left;
	linux_xk_map_upper[0xFF & XK_Up       ] = key_up;
	linux_xk_map_upper[0xFF & XK_Right    ] = key_right;
	linux_xk_map_upper[0xFF & XK_Down     ] = key_down;
	linux_xk_map_upper[0xFF & XK_End      ] = key_end;
	linux_xk_map_upper[0xFF & XK_Begin    ] = key_home;
	linux_xk_map_upper[0xFF & XK_Page_Up  ] = key_page_up;
	linux_xk_map_upper[0xFF & XK_Page_Down] = key_page_down;
	linux_xk_map_upper[0xFF & XK_KP_0] = key_num0;
	linux_xk_map_upper[0xFF & XK_KP_1] = key_num1;
	linux_xk_map_upper[0xFF & XK_KP_2] = key_num2;
	linux_xk_map_upper[0xFF & XK_KP_3] = key_num3;
	linux_xk_map_upper[0xFF & XK_KP_4] = key_num4;
	linux_xk_map_upper[0xFF & XK_KP_5] = key_num5;
	linux_xk_map_upper[0xFF & XK_KP_6] = key_num6;
	linux_xk_map_upper[0xFF & XK_KP_7] = key_num7;
	linux_xk_map_upper[0xFF & XK_KP_8] = key_num8;
	linux_xk_map_upper[0xFF & XK_KP_9] = key_num9;
	linux_xk_map_upper[0xFF & XK_F1 ] = key_f1;
	linux_xk_map_upper[0xFF & XK_F2 ] = key_f2;
	linux_xk_map_upper[0xFF & XK_F3 ] = key_f3;
	linux_xk_map_upper[0xFF & XK_F4 ] = key_f4;
	linux_xk_map_upper[0xFF & XK_F5 ] = key_f5;
	linux_xk_map_upper[0xFF & XK_F6 ] = key_f6;
	linux_xk_map_upper[0xFF & XK_F7 ] = key_f7;
	linux_xk_map_upper[0xFF & XK_F8 ] = key_f8;
	linux_xk_map_upper[0xFF & XK_F9 ] = key_f9;
	linux_xk_map_upper[0xFF & XK_F10] = key_f10;
	linux_xk_map_upper[0xFF & XK_F11] = key_f11;
	linux_xk_map_upper[0xFF & XK_F12] = key_f12;
	linux_xk_map_upper[0xFF & XK_Control_L] = key_ctrl;
	linux_xk_map_upper[0xFF & XK_Control_R] = key_ctrl;
	linux_xk_map_upper[0xFF & XK_Shift_L  ] = key_shift;
	linux_xk_map_upper[0xFF & XK_Shift_R  ] = key_shift;
	linux_xk_map_upper[0xFF & XK_Caps_Lock] = key_caps_lock;

	linux_xk_map_lower[XK_space       ] = key_space;
	linux_xk_map_lower[XK_apostrophe  ] = key_apostrophe;
	linux_xk_map_lower[XK_comma       ] = key_comma;
	linux_xk_map_lower[XK_minus       ] = key_minus;
	linux_xk_map_lower[XK_period      ] = key_period;
	linux_xk_map_lower[XK_slash       ] = key_slash_fwd;
	linux_xk_map_lower[XK_semicolon   ] = key_semicolon;
	linux_xk_map_lower[XK_equal       ] = key_equals;
	linux_xk_map_lower[XK_bracketleft ] = key_bracket_open;
	linux_xk_map_lower[XK_bracketright] = key_bracket_close;
	linux_xk_map_lower[XK_backslash   ] = key_slash_back;
	linux_xk_map_lower[XK_grave       ] = key_backtick;
	linux_xk_map_lower[XK_0] = key_0;
	linux_xk_map_lower[XK_1] = key_1;
	linux_xk_map_lower[XK_2] = key_2;
	linux_xk_map_lower[XK_3] = key_3;
	linux_xk_map_lower[XK_4] = key_4;
	linux_xk_map_lower[XK_5] = key_5;
	linux_xk_map_lower[XK_6] = key_6;
	linux_xk_map_lower[XK_7] = key_7;
	linux_xk_map_lower[XK_8] = key_8;
	linux_xk_map_lower[XK_9] = key_9;
	linux_xk_map_lower[XK_a] = key_a;
	linux_xk_map_lower[XK_b] = key_b;
	linux_xk_map_lower[XK_c] = key_c;
	linux_xk_map_lower[XK_d] = key_d;
	linux_xk_map_lower[XK_e] = key_e;
	linux_xk_map_lower[XK_f] = key_f;
	linux_xk_map_lower[XK_g] = key_g;
	linux_xk_map_lower[XK_h] = key_h;
	linux_xk_map_lower[XK_i] = key_i;
	linux_xk_map_lower[XK_j] = key_j;
	linux_xk_map_lower[XK_k] = key_k;
	linux_xk_map_lower[XK_l] = key_l;
	linux_xk_map_lower[XK_m] = key_m;
	linux_xk_map_lower[XK_n] = key_n;
	linux_xk_map_lower[XK_o] = key_o;
	linux_xk_map_lower[XK_p] = key_p;
	linux_xk_map_lower[XK_q] = key_q;
	linux_xk_map_lower[XK_r] = key_r;
	linux_xk_map_lower[XK_s] = key_s;
	linux_xk_map_lower[XK_t] = key_t;
	linux_xk_map_lower[XK_u] = key_u;
	linux_xk_map_lower[XK_v] = key_v;
	linux_xk_map_lower[XK_w] = key_w;
	linux_xk_map_lower[XK_x] = key_x;
	linux_xk_map_lower[XK_y] = key_y;
	linux_xk_map_lower[XK_z] = key_z;
}

///////////////////////////////////////////

void platform_check_events() {
	if (linux_display == nullptr) {
		return;
	}

	XEvent evt;
	while (XPending(linux_display)) {
		XNextEvent(linux_display, &evt);

		// Find what window this event is for
		window_t*      win    = nullptr;
		platform_win_t win_id = -1;
		for (int32_t i = 0; i < linux_windows.count; i++) {
			if (linux_windows[i].x_window == evt.xany.window) {
				win    = &linux_windows[i];
				win_id = i;
				break;
			}
		}
		if (win == nullptr) continue;

		bool is_pressed = false;
		window_event_t e = {};
		switch(evt.type) {
			case KeyPress: is_pressed = true;
			case KeyRelease: {
				// Translate keypress to utf-8 character, and submit that
				char   keys_return[32];
				Status status;
				KeySym keysym = NoSymbol;
				int    count  = Xutf8LookupString(x_linux_input_context, &evt.xkey, keys_return, sizeof(keys_return), &keysym, &status);
				keys_return[count] = 0;

				if ((status == XLookupChars || status == XLookupBoth) && keysym != 0xFF08) {
					const char *next;
					char32_t    ch = utf8_decode_fast(keys_return, &next);

					e.type = platform_evt_character;
					e.data.character = ch;
					win->events.add(e);
				}

				// Translate keypress to a hardware key, and submit that
				uint32_t sym = XkbKeycodeToKeysym(linux_display, evt.xkey.keycode, 0, 0);
				key_     key = key_none;
				if      ((sym & 0xFFFFFF00U) == 0xFF00U) key = linux_xk_map_upper[sym & 0xFFU];
				else if ((sym & 0xFFFFFF00U) == 0x0000U) key = linux_xk_map_lower[sym];

				if (key != key_none) {
					e.type = is_pressed
						? platform_evt_key_press
						: platform_evt_key_release;
					e.data.press_release = key;
					win->events.add(e);

					// Some non-text characters get fed into the text system as
					// well
					if (is_pressed) {
						if (key == key_backspace || key == key_return || key == key_esc) {
							e.type = platform_evt_character;
							e.data.character = (char32_t)key;
							win->events.add(e);
						}
					}
				}
			} break;
			case ButtonPress: {
				switch (evt.xbutton.button) {
				case (1): e.type = platform_evt_mouse_press; e.data.press_release = key_mouse_left;    win->events.add(e); break;
				case (2): e.type = platform_evt_mouse_press; e.data.press_release = key_mouse_center;  win->events.add(e); break;
				case (3): e.type = platform_evt_mouse_press; e.data.press_release = key_mouse_right;   win->events.add(e); break;
				case (9): e.type = platform_evt_mouse_press; e.data.press_release = key_mouse_forward; win->events.add(e); break;
				case (8): e.type = platform_evt_mouse_press; e.data.press_release = key_mouse_back;    win->events.add(e); break;
				case (4): e.type = platform_evt_scroll;      e.data.scroll = +120;                     win->events.add(e); linux_scroll += 120; break; // scroll up
				case (5): e.type = platform_evt_scroll;      e.data.scroll = -120;                     win->events.add(e); linux_scroll -= 120; break; // scroll down
				}
			} break;
			case ButtonRelease: {
				if (sk_app_focus() == app_focus_active) {
					switch (evt.xbutton.button) {
					case (1): e.type = platform_evt_mouse_release; e.data.press_release = key_mouse_left;    win->events.add(e); break;
					case (2): e.type = platform_evt_mouse_release; e.data.press_release = key_mouse_center;  win->events.add(e); break;
					case (3): e.type = platform_evt_mouse_release; e.data.press_release = key_mouse_right;   win->events.add(e); break;
					case (9): e.type = platform_evt_mouse_release; e.data.press_release = key_mouse_forward; win->events.add(e); break;
					case (8): e.type = platform_evt_mouse_release; e.data.press_release = key_mouse_back;    win->events.add(e); break;
					}
				}
			} break;
			case MotionNotify: {
				linux_mouse_x = evt.xmotion.x;
				linux_mouse_y = evt.xmotion.y;
			} break;
			case FocusIn:     { e.type = platform_evt_app_focus; e.data.app_focus = app_focus_active;     win->events.add(e); } break;
			case FocusOut:    { e.type = platform_evt_app_focus; e.data.app_focus = app_focus_background; win->events.add(e); } break;
			case EnterNotify: { linux_mouse_in_window = true;  } break;
			case LeaveNotify: { linux_mouse_in_window = false; } break;
			case ConfigureNotify: {
				e.type = platform_evt_resize;
				e.data.resize.width  = evt.xconfigure.width;
				e.data.resize.height = evt.xconfigure.height;
				win->events.add(e);
				//Todo: only call this when the user is done resizing
				platform_win_resize(win_id, evt.xconfigure.width, evt.xconfigure.height);
			} break;
			case ClientMessage: {
				if (!strcmp(XGetAtomName(linux_display, evt.xclient.message_type), "WM_PROTOCOLS")) {
					e.type = platform_evt_close;
					win->events.add(e);
					return;
				}
			} break;
		}
	}
}

///////////////////////////////////////////

bool platform_win_next_event(platform_win_t window_id, platform_evt_* out_event, platform_evt_data_t* out_event_data) {
	window_t* window = &linux_windows[window_id];
	if (window->events.count > 0) {
		*out_event = window->events[0].type;
		*out_event_data = window->events[0].data;
		window->events.remove(0);
		return true;
	} return false;
}

///////////////////////////////////////////

skg_swapchain_t* platform_win_get_swapchain(platform_win_t window) { return linux_windows[window].has_swapchain ? &linux_windows[window].swapchain : nullptr; }

///////////////////////////////////////////

bool platform_get_cursor(vec2 *out_pos) {
	out_pos->x = (float)linux_mouse_x;
	out_pos->y = (float)linux_mouse_y;
	return linux_mouse_in_window;
}

///////////////////////////////////////////

void platform_set_cursor(vec2 window_pos) {
	// This XFixesHideCursor/ShowCursor fix is really bizarre.
	// I tracked this down through https://developer.blender.org/T53004#467383
	// and https://github.com/blender/blender/blob/a8f7d41d3898a8d3ae8afb4f95ea9f4f44db2a69/intern/ghost/intern/GHOST_SystemX11.cpp#L1680 .
	// I don't know why this works, but it definitely does - the pointer correctly warps to the right spot as of July 2022.

	if (linux_windows.count == 0) return;
	window_t *win = &linux_windows[0];

	if (xwayland){
		XFixesHideCursor(linux_display, win->x_window);
	}

	XWarpPointer(linux_display, win->x_window, win->x_window, 0, 0, 0, 0, window_pos.x,window_pos.y);

	if (xwayland){
		XFixesShowCursor(linux_display, win->x_window);
	}
	XFlush(linux_display);
}

///////////////////////////////////////////

float platform_get_scroll() {
	return linux_scroll;
}

///////////////////////////////////////////

recti_t platform_win_rect(platform_win_t window_id) {
	window_t* win = &linux_windows[window_id];
	return win->has_swapchain
		? recti_t{ 0,0, win->swapchain.width, win->swapchain.height }
		: recti_t{ 0, 0, 0, 0 };
}

///////////////////////////////////////////

font_t platform_default_font() {
	array_t<char*> fonts = array_t<char*>::make(2);
	font_t         result = nullptr;

	FcConfig* config = FcInitLoadConfigAndFonts();
	FcPattern* pattern = FcNameParse((const FcChar8*)("sans-serif"));
	FcConfigSubstitute(config, pattern, FcMatchPattern);
	FcDefaultSubstitute(pattern);

	FcResult   fc_result;
	FcPattern* font = FcFontMatch(config, pattern, &fc_result);
	if (font) {
		FcChar8* file = nullptr;
		if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch) {
			// Put the font file path in the proper string
			fonts.add(string_copy((char*)file));
		}
		FcPatternDestroy(font);
	}
	FcPatternDestroy(pattern);
	FcConfigDestroy(config);

	result = font_create_files((const char**)fonts.data, fonts.count);
	fonts.each(free);
	fonts.free();
	return result;
}

///////////////////////////////////////////

void platform_iterate_dir(const char *directory_path, void *callback_data, void (*on_item)(void *callback_data, const char *name, const platform_file_attr_t file_attr)) {
	if (string_eq(directory_path, "")) {
		directory_path = platform_path_separator;
	}

	DIR           *dir;
	struct dirent *dir_info;
	dir = opendir(directory_path);
	if (!dir) return;

	while ((dir_info = readdir(dir)) != nullptr) {
		if (string_eq(dir_info->d_name, ".") || string_eq(dir_info->d_name, "..")) continue;
		platform_file_attr_t file_attr;
		if (dir_info->d_type == DT_DIR) {
			file_attr.size    = 0;
			file_attr.file    = false;
			on_item(callback_data, dir_info->d_name, file_attr);
		}
		else if (dir_info->d_type == DT_REG) {
			char* file_path = new char[strlen(directory_path) + strlen(dir_info->d_name) + 1];
			strcpy(file_path, directory_path);
			strcat(file_path, "/");
			strcat(file_path, dir_info->d_name);
			struct stat file_stat;
			stat(file_path, &file_stat);
			free(file_path);
			platform_file_attr_t file_attr;
			file_attr.size = file_stat.st_size;
			on_item(callback_data, dir_info->d_name, file_attr);
		}
	}
	closedir(dir);
}

///////////////////////////////////////////

void platform_sleep(int ms) {
	sleep(ms / 1000);
}

///////////////////////////////////////////

// TODO: find an alternative to the registry for Linux
bool platform_key_save_bytes(const char* key, void* data,       int32_t data_size)   { return false; }
bool platform_key_load_bytes(const char* key, void* ref_buffer, int32_t buffer_size) { return false; }

///////////////////////////////////////////

void platform_msgbox_err(const char *text, const char *header) {
	log_warn("No messagebox capability for this platform!");
}

///////////////////////////////////////////

void platform_xr_keyboard_show   (bool show) { }
bool platform_xr_keyboard_present()          { return false; }
bool platform_xr_keyboard_visible()          { return false; }

///////////////////////////////////////////

void platform_print_callstack() { }

} // namespace sk

#endif // SK_OS_LINUX
