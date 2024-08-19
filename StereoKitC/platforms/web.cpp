/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
 */

#include "web.h"
#if defined(SK_OS_WEB)

#include <emscripten.h>
#include <emscripten/html5.h>

#include "../stereokit.h"
#include "../_stereokit.h"
#include "../sk_math.h"
#include "../asset_types/texture.h"
#include "../asset_types/font.h"
#include "../libraries/sokol_time.h"
#include "../libraries/unicode.h"
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
	array_t<window_event_t> events;
	skg_swapchain_t         swapchain;
	bool                    has_swapchain;
};

struct web_key_map_t {
	const char* name;
	key_        key;
};

window_t web_window        = {};
vec2     web_mouse_pos     = { 0,0 };
bool     web_mouse_tracked = false;
float    web_mouse_scroll  = 0;
bool     web_mouse_locked  = false;

web_key_map_t web_keymap[] = {
	{ "Backspace",      key_backspace },
	{ "Tab",            key_tab },
	{ "Enter",          key_return },
	{ "ShiftLeft",      key_shift },
	{ "ShiftRight",     key_shift },
	{ "ControlLeft",    key_ctrl },
	{ "ControlRight",   key_ctrl },
	{ "AltLeft",        key_alt },
	{ "AltRight",       key_alt },
	//{ "Pause",          key_pause },
	{ "CapsLock",       key_caps_lock },
	{ "Escape",         key_esc },
	{ "Space",          key_space },
	{ "PageUp",         key_page_up },
	{ "PageDown",       key_page_down },
	{ "End",            key_end },
	{ "Home",           key_home },
	{ "ArrowLeft",      key_left },
	{ "ArrowUp",        key_up },
	{ "ArrowRight",     key_right },
	{ "ArrowDown",      key_down },
	{ "PrintScreen",    key_printscreen },
	{ "Insert",         key_insert },
	{ "Delete",         key_del },
	{ "Digit0",         key_0 },
	{ "Digit1",         key_1 },
	{ "Digit2",         key_2 },
	{ "Digit3",         key_3 },
	{ "Digit4",         key_4 },
	{ "Digit5",         key_5 },
	{ "Digit6",         key_6 },
	{ "Digit7",         key_7 },
	{ "Digit8",         key_8 },
	{ "Digit9",         key_9 },
	{ "KeyA",           key_a },
	{ "KeyB",           key_b },
	{ "KeyC",           key_c },
	{ "KeyD",           key_d },
	{ "KeyE",           key_e },
	{ "KeyF",           key_f },
	{ "KeyG",           key_g },
	{ "KeyH",           key_h },
	{ "KeyI",           key_i },
	{ "KeyJ",           key_j },
	{ "KeyK",           key_k },
	{ "KeyL",           key_l },
	{ "KeyM",           key_m },
	{ "KeyN",           key_n },
	{ "KeyO",           key_o },
	{ "KeyP",           key_p },
	{ "KeyQ",           key_q },
	{ "KeyR",           key_r },
	{ "KeyS",           key_s },
	{ "KeyT",           key_t },
	{ "KeyU",           key_u },
	{ "KeyV",           key_v },
	{ "KeyW",           key_w },
	{ "KeyX",           key_x },
	{ "KeyY",           key_y },
	{ "KeyZ",           key_z },
	//{ "MetaLeft",       key_metaleft },
	//{ "MetaRight",      key_metaright },
	{ "Numpad0",        key_num0 },
	{ "Numpad1",        key_num1 },
	{ "Numpad2",        key_num2 },
	{ "Numpad3",        key_num3 },
	{ "Numpad4",        key_num4 },
	{ "Numpad5",        key_num5 },
	{ "Numpad6",        key_num6 },
	{ "Numpad7",        key_num7 },
	{ "Numpad8",        key_num8 },
	{ "Numpad9",        key_num9 },
	{ "NumpadMultiply", key_multiply },
	{ "NumpadAdd",      key_add },
	{ "NumpadSubtract", key_subtract },
	{ "NumpadDecimal",  key_decimal },
	{ "NumpadDivide",   key_divide },
	{ "F1",             key_f1 },
	{ "F2",             key_f2 },
	{ "F3",             key_f3 },
	{ "F4",             key_f4 },
	{ "F5",             key_f5 },
	{ "F6",             key_f6 },
	{ "F7",             key_f7 },
	{ "F8",             key_f8 },
	{ "F9",             key_f9 },
	{ "F10",            key_f10 },
	{ "F11",            key_f11 },
	{ "F12",            key_f12 },
	//{ "NumLock",        key_NUM_LOCK },
	//{ "ScrollLock",     key_SCROLL_LOCK },
	{ "Semicolon",      key_semicolon },
	{ "Equal",          key_equals },
	{ "Comma",          key_comma },
	{ "Minus",          key_minus },
	{ "Period",         key_period },
	{ "Slash",          key_slash_fwd },
	{ "Backquote",      key_apostrophe }, // GRAVE_ACCENT
	{ "BracketLeft",    key_bracket_open },
	{ "Backslash",      key_slash_back },
	{ "BracketRight",   key_bracket_close },
	{ "Quote",          key_backtick }, // GRAVE_ACCENT
	{ 0, key_none },
};


///////////////////////////////////////////

bool platform_impl_init() {
	return true;
}

///////////////////////////////////////////

void platform_impl_shutdown() {
}

///////////////////////////////////////////

void platform_impl_step() {
}

///////////////////////////////////////////

WEB_EXPORT void sk_web_canvas_resize(int32_t width, int32_t height) {
	window_t* win = &web_window;

	width  = maxi(1, width);
	height = maxi(1, height);

	if (win->has_swapchain == false || (width == win->swapchain.width && height == win->swapchain.height))
		return;
	//log_diagf("Resized to: %d<~BLK>x<~clr>%d", width, height);
	
	skg_swapchain_resize(&win->swapchain, width, height);

	window_event_t e = { platform_evt_resize };
	e.data.resize = { width, height };
	win->events.add(e);
}

///////////////////////////////////////////

void (*web_app_update  )(void);
void (*web_app_shutdown)(void);
bool32_t web_anim_callback(double t, void *) {
	sk_step(web_app_update);
	
	return sk_is_running() ? true : false;
}

///////////////////////////////////////////

int web_on_mouse(int event_type, const EmscriptenMouseEvent *mouse_event, void *user_data) {
	if (web_mouse_locked) web_mouse_pos += vec2 { (float)mouse_event->movementX, (float)mouse_event->movementY };
	else                  web_mouse_pos  = vec2 { (float)mouse_event->targetX,   (float)mouse_event->targetY };

	return 0;
}

///////////////////////////////////////////

EM_BOOL web_on_scroll(int event_type, const EmscriptenWheelEvent *wheel_event, void *user_data) {
	web_mouse_scroll -= wheel_event->deltaY;
	return 0;
}

///////////////////////////////////////////

EM_BOOL web_on_key(int event_type, const EmscriptenKeyboardEvent *key_event, void *user_data) {
	window_t* win = &web_window;
	if (event_type == EMSCRIPTEN_EVENT_KEYPRESS) {
		const char *next;
		window_event_t e = { platform_evt_character };
		e.data.character = utf8_decode_fast(key_event->charValue, &next);
		win->events.add(e);
		return 1;
	}

	key_ keycode = key_none;
	for (int32_t i = 0; i < _countof(web_keymap); i++) {
		if (string_eq(web_keymap[i].name, key_event->code)) {
			keycode = web_keymap[i].key;
			break;
		}
	}
	window_event_t e = {};
	e.data.press_release = keycode;
	if      (event_type == EMSCRIPTEN_EVENT_KEYDOWN) { e.type = platform_evt_key_press;   win->events.add(e); }
	else if (event_type == EMSCRIPTEN_EVENT_KEYUP)   { e.type = platform_evt_key_release; win->events.add(e); }

	// Disable/enable pointer lock if shift is released/pressed
	if (web_mouse_locked                                      &&
		sk_get_settings_ref()->mode == app_mode_simulator     &&
		event_type                  == EMSCRIPTEN_EVENT_KEYUP &&
		keycode                     == key_shift)
	{
		emscripten_exit_pointerlock();
		web_mouse_locked = false;
	}

	return 1;
}

///////////////////////////////////////////

EM_BOOL web_on_mouse_press(int event_type, const EmscriptenMouseEvent *mouse_event, void *user_data) {
	window_t* win = &web_window;
	key_      key = key_none;
	switch (mouse_event->button) {
	case 0: key = key_mouse_left;   break;
	case 1: key = key_mouse_center; break;
	case 2: key = key_mouse_right;  break;
	}

	if (key != key_none) {
		window_event_t e = {};
		e.data.press_release = key;
		if      (event_type == EMSCRIPTEN_EVENT_MOUSEDOWN) { e.type = platform_evt_mouse_press;   win->events.add(e); }
		else if (event_type == EMSCRIPTEN_EVENT_MOUSEUP  ) { e.type = platform_evt_mouse_release; win->events.add(e); }
	}

	if (sk_get_settings_ref()->mode == app_mode_simulator && key == key_mouse_right) {
		if (web_mouse_locked == false && event_type == EMSCRIPTEN_EVENT_MOUSEDOWN && input_key(key_shift) & button_state_active) {
			web_mouse_locked = emscripten_request_pointerlock("canvas", false) == EMSCRIPTEN_RESULT_SUCCESS;
		}
		else if (web_mouse_locked == true && event_type == EMSCRIPTEN_EVENT_MOUSEUP) {
			emscripten_exit_pointerlock();
			web_mouse_locked = false;
		}
	}
	return 1;
}

///////////////////////////////////////////

EM_BOOL web_on_pointerlock(int event_type, const EmscriptenPointerlockChangeEvent *pointerlock_change_event, void *user_data) {
	web_mouse_locked = pointerlock_change_event->isActive;
	return 1;
}

///////////////////////////////////////////

EM_BOOL web_on_mouse_enter(int event_type, const EmscriptenMouseEvent *mouse_event, void *user_data) {
	if      (event_type == EMSCRIPTEN_EVENT_MOUSEENTER) web_mouse_tracked = true;
	else if (event_type == EMSCRIPTEN_EVENT_MOUSELEAVE) web_mouse_tracked = false;
	return 1;
}

///////////////////////////////////////////

void web_start_main_loop(void (*app_update)(void), void (*app_shutdown)(void)) {
	web_app_update   = app_update;
	web_app_shutdown = app_shutdown;
	emscripten_request_animation_frame_loop(web_anim_callback, 0);

	emscripten_set_mousemove_callback ("canvas", nullptr, false, web_on_mouse);
	emscripten_set_wheel_callback     ("canvas", nullptr, false, web_on_scroll);
	emscripten_set_keydown_callback   (EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, false, web_on_key);
	emscripten_set_keyup_callback     (EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, false, web_on_key);
	emscripten_set_keypress_callback  (EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, false, web_on_key);
	emscripten_set_mousedown_callback ("canvas", nullptr, true, web_on_mouse_press);
	emscripten_set_mouseup_callback   ("canvas", nullptr, true, web_on_mouse_press);
	emscripten_set_mouseenter_callback("canvas", nullptr, false, web_on_mouse_enter);
	emscripten_set_mouseleave_callback("canvas", nullptr, false, web_on_mouse_enter);
	emscripten_set_pointerlockchange_callback("canvas", nullptr, false, web_on_pointerlock);
}

///////////////////////////////////////////
// Window code                           //
///////////////////////////////////////////

platform_win_type_ platform_win_type() { return platform_win_type_existing; }

///////////////////////////////////////////

platform_win_t platform_win_make(const char* title, recti_t win_rect, platform_surface_ surface_type) { return -1; }

///////////////////////////////////////////

platform_win_t platform_win_get_existing(platform_surface_ surface_type) {
	const sk_settings_t* settings = sk_get_settings_ref();
	window_t*            win      = &web_window;

	// Not all windows need a swapchain, but here's where we make 'em for those
	// that do.
	if (surface_type == platform_surface_swapchain) {
		skg_tex_fmt_ color_fmt = skg_tex_fmt_rgba32_linear;
		skg_tex_fmt_ depth_fmt = (skg_tex_fmt_)render_preferred_depth_fmt();

		win->swapchain     = skg_swapchain_create(nullptr, color_fmt, depth_fmt, settings->flatscreen_width, settings->flatscreen_height);
		win->has_swapchain = true;

		log_diagf("Created swapchain: %dx%d color:%s depth:%s", win->swapchain.width, win->swapchain.height, render_fmt_name((tex_format_)color_fmt), render_fmt_name((tex_format_)depth_fmt));
	}
	return 1;
}

///////////////////////////////////////////

void platform_win_destroy(platform_win_t window) {
	window_t* win = &web_window;

	if (win->has_swapchain) {
		skg_swapchain_destroy(&win->swapchain);
	}

	win->events.free();
	*win = {};
}

///////////////////////////////////////////

void platform_check_events() {
}

///////////////////////////////////////////

bool platform_win_next_event(platform_win_t window_id, platform_evt_* out_event, platform_evt_data_t* out_event_data) {
	if (window_id != 1) return false;
	window_t* win = &web_window;

	if (win->events.count > 0) {
		*out_event      = win->events[0].type;
		*out_event_data = win->events[0].data;
		win->events.remove(0);
		return true;
	} return false;
}

///////////////////////////////////////////

skg_swapchain_t* platform_win_get_swapchain(platform_win_t window) {
	if (window != 1) return nullptr;
	window_t* win = &web_window;

	return win->has_swapchain ? &win->swapchain : nullptr;
}

///////////////////////////////////////////

recti_t platform_win_rect(platform_win_t window_id) {
	if (window_id != 1) return {};
	window_t* win = &web_window;

	return recti_t{ 0, 0,
		win->swapchain.width,
		win->swapchain.height };
}

///////////////////////////////////////////

bool  platform_get_cursor(vec2 *out_pos   ) { *out_pos = web_mouse_pos; return web_mouse_tracked; }
void  platform_set_cursor(vec2  window_pos) { web_mouse_pos = window_pos; }
float platform_get_scroll(                ) { return web_mouse_scroll; }

///////////////////////////////////////////

// TODO: find an alternative to the registry for Web
bool platform_key_save_bytes(const char* key, void* data,       int32_t data_size)   { return false; }
bool platform_key_load_bytes(const char* key, void* ref_buffer, int32_t buffer_size) { return false; }

///////////////////////////////////////////

void platform_msgbox_err(const char* text, const char* header) {
	log_warn("No messagebox capability for this platform!");
}

///////////////////////////////////////////

void platform_xr_keyboard_show   (bool show) { }
bool platform_xr_keyboard_present()          { return false; }
bool platform_xr_keyboard_visible()          { return false; }

///////////////////////////////////////////

font_t platform_default_font() {
	return font_create_default();
}

///////////////////////////////////////////

void platform_iterate_dir(const char* directory_path, void* callback_data, void (*on_item)(void* callback_data, const char* name, const platform_file_attr_t file_attr)) {}
///////////////////////////////////////////

char* platform_working_dir() {
	return string_copy("/");
}

///////////////////////////////////////////

void platform_print_callstack() { }

///////////////////////////////////////////

void platform_sleep(int ms) {
	emscripten_sleep(ms);
}

} // namespace sk

#endif // defined(SK_OS_WEB)