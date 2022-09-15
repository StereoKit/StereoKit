#include "web.h"

#if defined(SK_OS_WEB)

#include <emscripten.h>
#include <emscripten/html5.h>

#include "../stereokit.h"
#include "../_stereokit.h"
#include "../asset_types/texture.h"
#include "../libraries/sokol_time.h"
#include "../libraries/unicode.h"
#include "../libraries/stref.h"
#include "../systems/system.h"
#include "../systems/render.h"
#include "../systems/input.h"
#include "../systems/input_keyboard.h"
#include "../hands/input_hand.h"
#include "flatscreen_input.h"

namespace sk {

///////////////////////////////////////////

typedef struct web_key_map_t {
	const char* name;
	key_        key;
} web_key_map_t;

skg_swapchain_t web_swapchain  = {};
bool            web_swapchain_initialized = false;
system_t       *web_render_sys = nullptr;
vec2            web_mouse_pos  = { 0,0 };
bool            web_mouse_tracked = false;
float           web_mouse_scroll  = 0;
bool            web_mouse_locked  = false;

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

WEB_EXPORT void sk_web_canvas_resize(int32_t width, int32_t height) {
	if (!web_swapchain_initialized || (width == sk_info.display_width && height == sk_info.display_height))
		return;
	sk_info.display_width  = width;
	sk_info.display_height = height;
	log_diagf("Resized to: %d<~BLK>x<~clr>%d", width, height);
	
	skg_swapchain_resize(&web_swapchain, sk_info.display_width, sk_info.display_height);
	render_update_projection();
}

///////////////////////////////////////////

bool web_start_pre_xr() {
	return true;
}

///////////////////////////////////////////

bool web_start_post_xr() {
	return true;
}

///////////////////////////////////////////

bool web_init() {
	web_render_sys = systems_find("FrameRender");
	return true;
}

///////////////////////////////////////////

void web_shutdown() {
}

///////////////////////////////////////////

bool web_start_flat() {
	sk_info.display_width  = sk_settings.flatscreen_width;
	sk_info.display_height = sk_settings.flatscreen_height;
	sk_info.display_type   = display_opaque;

	skg_tex_fmt_ color_fmt = skg_tex_fmt_rgba32_linear;
	skg_tex_fmt_ depth_fmt = (skg_tex_fmt_)render_preferred_depth_fmt(); // skg_tex_fmt_depthstencil

	web_swapchain = skg_swapchain_create(nullptr, color_fmt, depth_fmt, sk_info.display_width, sk_info.display_height);
	web_swapchain_initialized = true;
	sk_info.display_width  = web_swapchain.width;
	sk_info.display_height = web_swapchain.height;
	
	log_diagf("Created swapchain: %dx%d color:%s depth:%s", web_swapchain.width, web_swapchain.height, render_fmt_name((tex_format_)color_fmt), render_fmt_name((tex_format_)depth_fmt));

	flatscreen_input_init();

	return true;
}

///////////////////////////////////////////

void web_stop_flat() {
	flatscreen_input_shutdown();
	skg_swapchain_destroy    (&web_swapchain);
	web_swapchain_initialized = false;
}

///////////////////////////////////////////

void web_step_begin_xr() {
}

///////////////////////////////////////////

void web_step_begin_flat() {
	flatscreen_input_update();
}

///////////////////////////////////////////

void web_step_end_flat() {
	skg_draw_begin();

	color128 col = render_get_clear_color_ln();
	skg_swapchain_bind(&web_swapchain);
	skg_target_clear(true, &col.r);

	input_update_poses(true);

	matrix view = render_get_cam_final ();
	matrix proj = render_get_projection_matrix();
	matrix_inverse(view, view);
	render_draw_matrix(&view, &proj, 1, render_get_filter());
	render_clear();

	web_render_sys->profile_frame_duration = stm_since(web_render_sys->profile_frame_start);
	skg_swapchain_present(&web_swapchain);
}

///////////////////////////////////////////

bool web_get_cursor(vec2 &out_pos) {
	out_pos = web_mouse_pos;
	return web_mouse_tracked;
}

///////////////////////////////////////////

void web_set_cursor(vec2 window_pos) {
	web_mouse_pos = window_pos;
}

///////////////////////////////////////////

float web_get_scroll() {
	return web_mouse_scroll;
}

///////////////////////////////////////////

void (*web_app_update  )(void);
void (*web_app_shutdown)(void);
bool32_t web_anim_callback(double t, void *) {
	sk_step(web_app_update);
	
	return sk_running ? true : false;
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
	if (event_type == EMSCRIPTEN_EVENT_KEYPRESS) {
		const char *next;
		char32_t    ch = utf8_decode_fast(key_event->charValue, &next);
		input_text_inject_char(ch);
		return 1;
	}

	key_ keycode = key_none;
	for (int32_t i = 0; i < _countof(web_keymap); i++) {
		if (string_eq(web_keymap[i].name, key_event->code)) {
			keycode = web_keymap[i].key;
			break;
		}
	}
	if      (event_type == EMSCRIPTEN_EVENT_KEYDOWN) input_keyboard_inject_press  (keycode);
	else if (event_type == EMSCRIPTEN_EVENT_KEYUP)   input_keyboard_inject_release(keycode);

	// Disable/enable pointer lock if shift is released/pressed
	if (web_mouse_locked                                    &&
		sk_active_display_mode() == display_mode_flatscreen && 
		!sk_settings.disable_flatscreen_mr_sim              &&
		event_type               == EMSCRIPTEN_EVENT_KEYUP  &&
		keycode                  == key_shift)
	{
		emscripten_exit_pointerlock();
		web_mouse_locked = false;
	}

	return 1;
}

///////////////////////////////////////////

EM_BOOL web_on_mouse_press(int event_type, const EmscriptenMouseEvent *mouse_event, void *user_data) {
	key_ key = key_none;
	switch (mouse_event->button) {
	case 0: key = key_mouse_left;   break;
	case 1: key = key_mouse_center; break;
	case 2: key = key_mouse_right;  break;
	}
	if (key != key_none) {
		if      (event_type == EMSCRIPTEN_EVENT_MOUSEDOWN) input_keyboard_inject_press  (key);
		else if (event_type == EMSCRIPTEN_EVENT_MOUSEUP  ) input_keyboard_inject_release(key);
	}

	if (sk_active_display_mode() == display_mode_flatscreen && !sk_settings.disable_flatscreen_mr_sim && key == key_mouse_right) {
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

} // namespace sk

#endif // defined(SK_OS_WEB)