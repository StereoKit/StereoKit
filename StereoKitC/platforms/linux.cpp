#include "linux.h"

#if defined(SK_OS_LINUX)

#include <GL/glx.h>

#include <X11/keysym.h>
#include <X11/XKBlib.h>

#include "../xr_backends/openxr.h"
#include "flatscreen_input.h"
#include "../systems/render.h"
#include "../systems/input.h"
#include "../systems/input_keyboard.h"
#include "../systems/system.h"
#include "../_stereokit.h"
#include "../log.h"
#include "../libraries/sk_gpu.h"
#include "../libraries/sokol_time.h"
#include "../libraries/unicode.h"

#if defined(SKG_LINUX_EGL)

extern EGLDisplay egl_display;
extern EGLContext egl_context;
extern EGLConfig  egl_config;

#endif

namespace sk {

///////////////////////////////////////////

skg_swapchain_t         linux_swapchain;
bool32_t                linux_swapchain_initialized = false;
system_t               *linux_render_sys = nullptr;

// GLX/Xlib

Display                *x_dpy;
XIM                     x_linux_input_method;
XIC                     x_linux_input_context;

XVisualInfo            *x_vi;
GLXFBConfig             x_fb_config;
Colormap                x_cmap;
XSetWindowAttributes    x_swa;
Window                  x_win;
Window                  x_root;

///////////////////////////////////////////
// Start input
///////////////////////////////////////////

key_  linux_xk_map_upper[256] = {};
key_  linux_xk_map_lower[256] = {};
float linux_scroll  = 0;
int   linux_mouse_x = 0;
int   linux_mouse_y = 0;
bool  linux_mouse_in_window = false;
bool  linux_window_focused = false;

void linux_init_key_lookups() {
	linux_xk_map_upper[0xFF & XK_BackSpace] = key_backspace;
	linux_xk_map_upper[0xFF & XK_Tab] = key_tab;
	linux_xk_map_upper[0xFF & XK_Linefeed] = key_return;
	linux_xk_map_upper[0xFF & XK_Return] = key_return;
	linux_xk_map_upper[0xFF & XK_Escape] = key_esc;
	linux_xk_map_upper[0xFF & XK_Delete] = key_del;
	linux_xk_map_upper[0xFF & XK_Home] = key_home;
	linux_xk_map_upper[0xFF & XK_Left] = key_left;
	linux_xk_map_upper[0xFF & XK_Up] = key_up;
	linux_xk_map_upper[0xFF & XK_Right] = key_right;
	linux_xk_map_upper[0xFF & XK_Down] = key_down;
	linux_xk_map_upper[0xFF & XK_End] = key_end;
	linux_xk_map_upper[0xFF & XK_Begin] = key_home;
	linux_xk_map_upper[0xFF & XK_Page_Up] = key_page_up;
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
	linux_xk_map_upper[0xFF & XK_F1] = key_f1;
	linux_xk_map_upper[0xFF & XK_F2] = key_f2;
	linux_xk_map_upper[0xFF & XK_F3] = key_f3;
	linux_xk_map_upper[0xFF & XK_F4] = key_f4;
	linux_xk_map_upper[0xFF & XK_F5] = key_f5;
	linux_xk_map_upper[0xFF & XK_F6] = key_f6;
	linux_xk_map_upper[0xFF & XK_F7] = key_f7;
	linux_xk_map_upper[0xFF & XK_F8] = key_f8;
	linux_xk_map_upper[0xFF & XK_F9] = key_f9;
	linux_xk_map_upper[0xFF & XK_F10] = key_f10;
	linux_xk_map_upper[0xFF & XK_F11] = key_f11;
	linux_xk_map_upper[0xFF & XK_F12] = key_f12;
	linux_xk_map_upper[0xFF & XK_Control_L] = key_ctrl;
	linux_xk_map_upper[0xFF & XK_Control_R] = key_ctrl;
	linux_xk_map_upper[0xFF & XK_Shift_L] = key_shift;
	linux_xk_map_upper[0xFF & XK_Shift_R] = key_shift;
	linux_xk_map_upper[0xFF & XK_Caps_Lock] = key_caps_lock;

	linux_xk_map_lower[XK_space] = key_space;
	linux_xk_map_lower[XK_apostrophe] = key_apostrophe;
	linux_xk_map_lower[XK_comma] = key_comma;
	linux_xk_map_lower[XK_minus] = key_minus;
	linux_xk_map_lower[XK_period] = key_period;
	linux_xk_map_lower[XK_slash] = key_slash_fwd;
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
	linux_xk_map_lower[XK_semicolon] = key_semicolon;
	linux_xk_map_lower[XK_equal] = key_equals;
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
	linux_xk_map_lower[XK_bracketleft] = key_bracket_open;
	linux_xk_map_lower[XK_bracketright] = key_bracket_close;
	linux_xk_map_lower[XK_backslash] = key_slash_back;
	linux_xk_map_lower[XK_grave] = key_backtick;
}

void linux_events() {
	XEvent event;

	while (XPending(x_dpy)) {
		XNextEvent(x_dpy, &event);

		bool is_pressed = false;
		switch(event.type) {
			case KeyPress: is_pressed = true;
			case KeyRelease: {
				// Translate keypress to utf-8 character, and submit that
				char   keys_return[32];
				Status status;
				KeySym keysym = NoSymbol;
				int    count  = Xutf8LookupString(x_linux_input_context, &event.xkey, keys_return, sizeof(keys_return), &keysym, &status);
				keys_return[count] = 0;

				if ((status == XLookupChars || status == XLookupBoth) && keysym != 0xFF08) {
					const char *next;
					char32_t    ch = utf8_decode_fast(keys_return, &next);
					input_text_inject_char(ch);
				}

				// Translate keypress to a hardware key, and submit that
				uint32_t sym = XkbKeycodeToKeysym(x_dpy, event.xkey.keycode, 0, 0);
				key_     key = key_none;
				if      ((sym & 0xFFFFFF00U) == 0xFF00U) key = linux_xk_map_upper[sym & 0xFFU];
				else if ((sym & 0xFFFFFF00U) == 0x0000U) key = linux_xk_map_lower[sym];

				if (key != key_none) {
					if (is_pressed) input_keyboard_inject_press  (key);
					else            input_keyboard_inject_release(key);

					// On desktop, we want to hide soft keyboards on physical
					// presses
					input_last_physical_keypress = time_getf();
					platform_keyboard_show(false, text_context_text);

					// Some non-text characters get fed into the text system as
					// well
					if (is_pressed) {
						if (key == key_backspace || key == key_return || key == key_esc) {
							input_text_inject_char((char32_t)key);
						}
					}
				}
			} break;
			case ButtonPress: {
				if (sk_focus == app_focus_active) {
					switch (event.xbutton.button) {
					case (1): input_keyboard_inject_press(key_mouse_left);    break;
					case (2): input_keyboard_inject_press(key_mouse_center);  break;
					case (3): input_keyboard_inject_press(key_mouse_right);   break;
					case (9): input_keyboard_inject_press(key_mouse_forward); break;
					case (8): input_keyboard_inject_press(key_mouse_back);    break;
					case (4): linux_scroll += 120; break; // scroll up
					case (5): linux_scroll -= 120; break; // scroll down
					}
				}
			} break;
			case ButtonRelease: {
				if (sk_focus == app_focus_active) {
					switch (event.xbutton.button) {
					case (1): input_keyboard_inject_release(key_mouse_left);    break;
					case (2): input_keyboard_inject_release(key_mouse_center);  break;
					case (3): input_keyboard_inject_release(key_mouse_right);   break;
					case (9): input_keyboard_inject_release(key_mouse_forward); break;
					case (8): input_keyboard_inject_release(key_mouse_back);    break;
					}
				}
			} break;
			case MotionNotify: {
				linux_mouse_x = event.xmotion.x;
				linux_mouse_y = event.xmotion.y;
			} break;
			case EnterNotify: {
				linux_mouse_in_window = true;
			} break;
			case LeaveNotify: {
				linux_mouse_in_window = false;
			} break;
			case FocusIn: {
				linux_window_focused = true;
			} break;
			case FocusOut: {
				linux_window_focused = false;
			} break;
			case ConfigureNotify: {
				//Todo: only call this when the user is done resizing
				linux_resize(event.xconfigure.width, event.xconfigure.height);
			} break;
			case ClientMessage: {
				if (!strcmp(XGetAtomName(x_dpy, event.xclient.message_type), "WM_PROTOCOLS")) {
					sk_quit();
					return;
				}
			} break;
		}
	}
}

///////////////////////////////////////////
// End input
///////////////////////////////////////////

bool setup_x_window() {
	x_dpy = XOpenDisplay(nullptr);
	if (x_dpy == nullptr) {
		log_fail_reason(90, log_error, "Cannot connect to X server");
		return false;
	}
	x_root = DefaultRootWindow(x_dpy);
	if (x_root == 0) {
		log_fail_reason(90, log_error, "No root window found!");
		return false;
	}

	// FBConfigs were added in GLX version 1.3.
	int32_t glx_major, glx_minor;
	if (!glXQueryVersion(x_dpy, &glx_major, &glx_minor) || 
		((glx_major == 1) && (glx_minor < 3) ) || (glx_major < 1)) {
		log_fail_reason(90, log_error, "GLX 1.3+ required.");
		return false;
	}

	GLint fb_attributes[] = {
		GLX_X_RENDERABLE,  true,
		GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
		GLX_RENDER_TYPE,   GLX_RGBA_BIT,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_DOUBLEBUFFER,  true,
		GLX_RED_SIZE,      8,
		GLX_GREEN_SIZE,    8,
		GLX_BLUE_SIZE,     8,
		GLX_ALPHA_SIZE,    8,
		GLX_DEPTH_SIZE,    16,
		None
	};

	int32_t      config_count = 0;
	GLXFBConfig *configs      = glXChooseFBConfig(x_dpy, 0, fb_attributes, &config_count);
	if (configs == nullptr) {
		log_fail_reason(90, log_error, "No matching display configurations found!");
		return false;
	}

	// find a configuration with multisample enabled
	int32_t best_config = -1, best_samples = -1;
	for (int32_t i=0; i<config_count; i++) {
		XVisualInfo *vi = glXGetVisualFromFBConfig(x_dpy, configs[i]);
		if (vi != nullptr) {
			int32_t samp_buf, samples;
			glXGetFBConfigAttrib(x_dpy, configs[i], GLX_SAMPLE_BUFFERS, &samp_buf);
			glXGetFBConfigAttrib(x_dpy, configs[i], GLX_SAMPLES       , &samples );

			if (best_config < 0 || (samp_buf != 0 && samples > best_samples && samples <= 8)) {
				best_config  = i;
				best_samples = samples;
			}
		}
		XFree( vi );
	}
	if (best_config == -1) {
		log_fail_reason(90, log_error, "No appropriate GLX visual found");
		return false;
	}

	x_fb_config = configs[best_config];
	XFree(configs);

	x_vi   = glXGetVisualFromFBConfig(x_dpy, x_fb_config);
	x_cmap = XCreateColormap(x_dpy, x_root, x_vi->visual, AllocNone);

	x_swa.colormap   = x_cmap;
	x_swa.event_mask = ExposureMask | KeyPressMask;
	x_win = XCreateWindow(x_dpy, x_root, 0, 0, sk_settings.flatscreen_width, sk_settings.flatscreen_height, 0, x_vi->depth, InputOutput, x_vi->visual, CWColormap | CWEventMask, &x_swa);

	XSizeHints *hints = XAllocSizeHints();
	if (hints == nullptr) {
		log_err("XAllocSizeHints failed.");
	} else {
		hints->flags      = PMinSize;
		hints->min_width  = 100;
		hints->min_height = 100;
		XSetWMNormalHints(x_dpy, x_win, hints);
		XSetWMSizeHints  (x_dpy, x_win, hints, PMinSize);
	}

	XMapWindow(x_dpy, x_win);
	XChangeProperty(x_dpy, x_win, XInternAtom(x_dpy, "_NET_WM_NAME", False),
								XInternAtom(x_dpy, "UTF8_STRING", False), 8, PropModeReplace,
								(unsigned char *)sk_app_name, strlen(sk_app_name));

	// loads the XMODIFIERS environment variable to see what input method to use
	XSetLocaleModifiers("");

	x_linux_input_method = XOpenIM(x_dpy, 0, 0, 0);
	if (!x_linux_input_method) {
		// fallback to internal input method
		XSetLocaleModifiers("@im=none");
		x_linux_input_method = XOpenIM(x_dpy, 0, 0, 0);
	}

	// X input context, you can have multiple for text boxes etc, but having a
	// single one is the easiest.
	x_linux_input_context = XCreateIC(x_linux_input_method,
						XNInputStyle,   XIMPreeditNothing | XIMStatusNothing,
						XNClientWindow, x_win,
						XNFocusWindow,  x_win,
						nullptr);
	XSetICFocus(x_linux_input_context);

	// Setup for window events
	XSelectInput(x_dpy, x_win, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask | EnterWindowMask | LeaveWindowMask | FocusChangeMask);
	Atom wm_delete = XInternAtom(x_dpy, "WM_DELETE_WINDOW", true);
	XSetWMProtocols(x_dpy, x_win, &wm_delete, 1);

	sk_info.display_width  = sk_settings.flatscreen_width;
	sk_info.display_height = sk_settings.flatscreen_height;
	sk_info.display_type   = display_opaque;
	skg_tex_fmt_ color_fmt = skg_tex_fmt_rgba32_linear;
	skg_tex_fmt_ depth_fmt = (skg_tex_fmt_)render_preferred_depth_fmt();
	linux_swapchain = skg_swapchain_create((void *) x_win, color_fmt, depth_fmt, sk_info.display_width, sk_info.display_height);
	linux_swapchain_initialized = true;
	sk_info.display_width  = linux_swapchain.width;
	sk_info.display_height = linux_swapchain.height;

	XWindowAttributes wa;
	XGetWindowAttributes(x_dpy,x_win,&wa);

	skg_setup_xlib(x_dpy, x_vi, &x_fb_config, &x_win);
	return true;
}

///////////////////////////////////////////

#if defined(SKG_LINUX_EGL)

bool setup_egl_flat() {
	return true;
}

#endif

///////////////////////////////////////////

bool linux_init() {
	linux_render_sys = systems_find("FrameRender");
	linux_init_key_lookups();

	#if !defined(SKG_LINUX_EGL)

	if (!setup_x_window())
		return false;

	#endif

	return true;
}

///////////////////////////////////////////

bool linux_start_pre_xr() {
	return true;
}

///////////////////////////////////////////

bool linux_start_post_xr() {
	return true;
}

///////////////////////////////////////////

bool linux_start_flat() {
	#if defined(SKG_LINUX_EGL)
	if (!setup_x_window())
		return false;
	if (!setup_egl_flat())
		return false;
	#endif
	
	flatscreen_input_init();

	return true;
}

///////////////////////////////////////////

void linux_resize(int width, int height) {
	if (!linux_swapchain_initialized || (width == sk_info.display_width && height == sk_info.display_height))
		return;
	sk_info.display_width  = width;
	sk_info.display_height = height;
	log_diagf("Resized to: %d<~BLK>x<~clr>%d", width, height);

	skg_swapchain_resize(&linux_swapchain, sk_info.display_width, sk_info.display_height);
	render_update_projection();
}

///////////////////////////////////////////

bool linux_get_cursor(vec2 &out_pos) {
	out_pos.x = (float)linux_mouse_x;
	out_pos.y = (float)linux_mouse_y;
	return linux_window_focused || linux_mouse_in_window;
}

///////////////////////////////////////////

float linux_get_scroll() {
	return linux_scroll;
}

///////////////////////////////////////////

void linux_set_cursor(vec2 window_pos) {
	XWarpPointer(x_dpy, x_win, x_win, 0, 0, 0, 0, window_pos.x,window_pos.y);
	XFlush(x_dpy);
}

///////////////////////////////////////////

void linux_stop_flat() {
	flatscreen_input_shutdown();
	skg_swapchain_destroy(&linux_swapchain);
}

///////////////////////////////////////////

void linux_shutdown() {
	XCloseDisplay(x_dpy);
	linux_swapchain_initialized = false;
}

///////////////////////////////////////////

void linux_step_begin_xr() {
	linux_events();
}

///////////////////////////////////////////

void linux_step_begin_flat() {
	linux_events();
	flatscreen_input_update();
}

///////////////////////////////////////////

void linux_step_end_flat() {
	skg_draw_begin();

	color128 col = render_get_clear_color_ln();
	skg_swapchain_bind(&linux_swapchain);
	skg_target_clear(true, &col.r);

	linux_render_sys->profile_frame_start = stm_now();

	input_update_poses(true);

	matrix view = render_get_cam_final        ();
	matrix proj = render_get_projection_matrix();
	matrix_inverse(view, view);
	render_draw_matrix(&view, &proj, 1, render_get_filter());
	render_clear();

	skg_swapchain_present(&linux_swapchain);
}

} // namespace sk

#endif // SK_OS_LINUX
