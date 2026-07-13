// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2025 Nick Klingensmith
// Copyright (c) 2023-2025 Qualcomm Technologies, Inc.

#pragma once

#include "../stereokit.h"
#include "../_stereokit.h"
#include "../systems/input.h"
#include "../tools/file_picker.h"
#include <sk_app.h>

namespace sk {

///////////////////////////////////////////
// sk_app scancode to SK key_ mapping
// Shared between simulator, window, and openxr backends

inline key_ ska_scancode_to_key(ska_scancode_ scancode) {
	switch (scancode) {
	case ska_scancode_a: return key_a;
	case ska_scancode_b: return key_b;
	case ska_scancode_c: return key_c;
	case ska_scancode_d: return key_d;
	case ska_scancode_e: return key_e;
	case ska_scancode_f: return key_f;
	case ska_scancode_g: return key_g;
	case ska_scancode_h: return key_h;
	case ska_scancode_i: return key_i;
	case ska_scancode_j: return key_j;
	case ska_scancode_k: return key_k;
	case ska_scancode_l: return key_l;
	case ska_scancode_m: return key_m;
	case ska_scancode_n: return key_n;
	case ska_scancode_o: return key_o;
	case ska_scancode_p: return key_p;
	case ska_scancode_q: return key_q;
	case ska_scancode_r: return key_r;
	case ska_scancode_s: return key_s;
	case ska_scancode_t: return sk::key_t;
	case ska_scancode_u: return key_u;
	case ska_scancode_v: return key_v;
	case ska_scancode_w: return key_w;
	case ska_scancode_x: return key_x;
	case ska_scancode_y: return key_y;
	case ska_scancode_z: return key_z;
	case ska_scancode_1: return key_1;
	case ska_scancode_2: return key_2;
	case ska_scancode_3: return key_3;
	case ska_scancode_4: return key_4;
	case ska_scancode_5: return key_5;
	case ska_scancode_6: return key_6;
	case ska_scancode_7: return key_7;
	case ska_scancode_8: return key_8;
	case ska_scancode_9: return key_9;
	case ska_scancode_0: return key_0;
	case ska_scancode_return:    return key_return;
	case ska_scancode_escape:    return key_esc;
	case ska_scancode_backspace: return key_backspace;
	case ska_scancode_tab:       return key_tab;
	case ska_scancode_space:     return key_space;
	case ska_scancode_minus:     return key_minus;
	case ska_scancode_equals:    return key_equals;
	case ska_scancode_leftbracket:  return key_bracket_open;
	case ska_scancode_rightbracket: return key_bracket_close;
	case ska_scancode_backslash:    return key_slash_back;
	case ska_scancode_semicolon:    return key_semicolon;
	case ska_scancode_apostrophe:   return key_apostrophe;
	case ska_scancode_grave:        return key_backtick;
	case ska_scancode_comma:        return key_comma;
	case ska_scancode_period:       return key_period;
	case ska_scancode_slash:        return key_slash_fwd;
	case ska_scancode_capslock:     return key_caps_lock;
	case ska_scancode_f1:  return key_f1;
	case ska_scancode_f2:  return key_f2;
	case ska_scancode_f3:  return key_f3;
	case ska_scancode_f4:  return key_f4;
	case ska_scancode_f5:  return key_f5;
	case ska_scancode_f6:  return key_f6;
	case ska_scancode_f7:  return key_f7;
	case ska_scancode_f8:  return key_f8;
	case ska_scancode_f9:  return key_f9;
	case ska_scancode_f10: return key_f10;
	case ska_scancode_f11: return key_f11;
	case ska_scancode_f12: return key_f12;
	case ska_scancode_printscreen: return key_printscreen;
	case ska_scancode_scrolllock:  return key_none; // SK doesn't have scroll lock
	case ska_scancode_pause:       return key_none; // No direct mapping
	case ska_scancode_insert:      return key_insert;
	case ska_scancode_home:        return key_home;
	case ska_scancode_pageup:      return key_page_up;
	case ska_scancode_delete:      return key_del;
	case ska_scancode_end:         return key_end;
	case ska_scancode_pagedown:    return key_page_down;
	case ska_scancode_right:       return key_right;
	case ska_scancode_left:        return key_left;
	case ska_scancode_down:        return key_down;
	case ska_scancode_up:          return key_up;
	case ska_scancode_lctrl:       return key_ctrl;
	case ska_scancode_lshift:      return key_shift;
	case ska_scancode_lalt:        return key_alt;
	case ska_scancode_lgui:        return key_lcmd;
	case ska_scancode_rctrl:       return key_ctrl;
	case ska_scancode_rshift:      return key_shift;
	case ska_scancode_ralt:        return key_alt;
	case ska_scancode_rgui:        return key_rcmd;
	default: return key_none;
	}
}

inline key_ ska_mouse_button_to_key(ska_mouse_button_ button) {
	switch (button) {
	case ska_mouse_button_left:   return key_mouse_left;
	case ska_mouse_button_middle: return key_mouse_center;
	case ska_mouse_button_right:  return key_mouse_right;
	case ska_mouse_button_x1:     return key_mouse_back;
	case ska_mouse_button_x2:     return key_mouse_forward;
	default: return key_none;
	}
}

// Decode UTF-8 text to UTF-32 codepoints and inject into input system
inline void ska_inject_text_input(const char* text) {
	while (*text) {
		uint32_t codepoint = 0;
		if ((*text & 0x80) == 0) {
			codepoint = *text++;
		} else if ((*text & 0xE0) == 0xC0) {
			codepoint = (*text++ & 0x1F) << 6;
			if (*text) codepoint |= (*text++ & 0x3F);
		} else if ((*text & 0xF0) == 0xE0) {
			codepoint = (*text++ & 0x0F) << 12;
			if (*text) codepoint |= (*text++ & 0x3F) << 6;
			if (*text) codepoint |= (*text++ & 0x3F);
		} else if ((*text & 0xF8) == 0xF0) {
			codepoint = (*text++ & 0x07) << 18;
			if (*text) codepoint |= (*text++ & 0x3F) << 12;
			if (*text) codepoint |= (*text++ & 0x3F) << 6;
			if (*text) codepoint |= (*text++ & 0x3F);
		} else {
			text++;
			continue;
		}
		input_text_inject_char((char32_t)codepoint);
	}
}

///////////////////////////////////////////
// Common event handler for events that backends don't handle themselves
// Backends poll ska_event_poll() and handle backend-specific events, then
// pass unhandled events to this function for common processing.

inline void ska_handle_event(ska_event_t* evt) {
	switch (evt->type) {
	case ska_event_window_focus_gained:
		sk_set_app_focus(app_focus_active);
		break;
	case ska_event_window_focus_lost:
		sk_set_app_focus(app_focus_background);
		break;
	case ska_event_key_down: {
		key_ key = ska_scancode_to_key(evt->keyboard.scancode);
		if (key != key_none)
			input_key_inject_press(key);
	} break;
	case ska_event_key_up: {
		key_ key = ska_scancode_to_key(evt->keyboard.scancode);
		if (key != key_none)
			input_key_inject_release(key);
	} break;
	case ska_event_text_input:
		ska_inject_text_input(evt->text.text);
		break;
	case ska_event_mouse_button_down: {
		if (sk_app_focus() == app_focus_active) {
			key_ key = ska_mouse_button_to_key(evt->mouse_button.button);
			if (key != key_none)
				input_key_inject_press(key);
		}
	} break;
	case ska_event_mouse_button_up: {
		if (sk_app_focus() == app_focus_active) {
			key_ key = ska_mouse_button_to_key(evt->mouse_button.button);
			if (key != key_none)
				input_key_inject_release(key);
		}
	} break;
	case ska_event_mouse_wheel:
		// sk_app normalizes scroll to ~1.0 per notch, but StereoKit's legacy
		// behavior used raw Windows scroll deltas (~120 per notch). Scale up
		// to maintain backwards compatibility with existing scroll-based code.
		if (sk_app_focus() == app_focus_active)
			input_scroll_inject(evt->mouse_wheel.precise_y * 120.0f);
		break;
	case ska_event_window_close:
		sk_quit(quit_reason_user);
		break;
	case ska_event_file_dialog:
		file_picker_handle_dialog_event(&evt->file_dialog);
		break;
	default:
		break;
	}
}

}
