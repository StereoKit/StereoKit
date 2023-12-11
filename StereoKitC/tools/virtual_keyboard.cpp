/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2023 Nick Klingensmith
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 */

#include "virtual_keyboard.h"
#include "virtual_keyboard_layouts.h"
#include "../stereokit_ui.h"
#include "../libraries/array.h"
#include "../libraries/unicode.h"

namespace sk {

bool32_t           keyboard_open         = false;
pose_t             keyboard_pose         = pose_identity;
array_t<key_>      keyboard_pressed_keys = {};
const keylayout_t* keyboard_active_layout;
text_context_      keyboard_text_context;

bool32_t           keyboard_symbols = false;
bool32_t           keyboard_shift   = false;
bool32_t           keyboard_ctrl    = false;
bool32_t           keyboard_alt     = false;

///////////////////////////////////////////

const keylayout_t* virtualkeyboard_get_system_keyboard_layout() {
	return &virtualkeyboard_layout_en_us;
}

///////////////////////////////////////////

void virtualkeyboard_reset_modifiers() {
	if (keyboard_alt    ) { input_key_inject_release(key_alt  ); keyboard_alt     = false; }
	if (keyboard_ctrl   ) { input_key_inject_release(key_ctrl ); keyboard_ctrl    = false; }
	if (keyboard_shift  ) { input_key_inject_release(key_shift); keyboard_shift   = false; }
	if (keyboard_symbols) {                                      keyboard_symbols = false; }
}

///////////////////////////////////////////

void virtualkeyboard_open(bool32_t open, text_context_ type) {
	if (open == keyboard_open && type == keyboard_text_context) return;

	// Position the keyboard in front of the user if this just opened
	if (open && !keyboard_open) {
		keyboard_pose = matrix_transform_pose(matrix_invert(render_get_cam_root()), ui_popup_pose({0,-0.1f,0}));
	}

	// Reset the keyboard to its default state
	virtualkeyboard_reset_modifiers();

	keyboard_text_context = type;
	keyboard_open         = open;
}

///////////////////////////////////////////

bool virtualkeyboard_get_open() {
	return keyboard_open;
}

///////////////////////////////////////////

void virtualkeyboard_initialize() {
	keyboard_active_layout = virtualkeyboard_get_system_keyboard_layout();
}

///////////////////////////////////////////

void send_key_data(const char* charkey,key_ key) {
	keyboard_pressed_keys.add(key);
	input_key_inject_press(key);
	char32_t c = 0;
	while (utf8_decode_fast_b(charkey, &charkey, &c)) {
		input_text_inject_char(c);
	}
}

///////////////////////////////////////////

void remove_last_clicked_keys() {
	for (int32_t i = 0; i < keyboard_pressed_keys.count; i++) {
		input_key_inject_release(keyboard_pressed_keys[0]);
		keyboard_pressed_keys.remove(0);
	}
}

///////////////////////////////////////////

void virtualkeyboard_keypress(keylayout_key_t key) {
	send_key_data(key.clicked_text, (key_)key.key_event_type);
	if (key.special_key == skey_close) {
		keyboard_open = false;
	}
	virtualkeyboard_reset_modifiers();
}

///////////////////////////////////////////

void virtualkeyboard_update() {
	if (!keyboard_open) return;
	if (keyboard_active_layout == nullptr) return;

	remove_last_clicked_keys();
	ui_push_preserve_keyboard(true);
	hierarchy_push(render_get_cam_root());
	ui_window_begin("SK/Keyboard", keyboard_pose, {0,0}, ui_win_body, ui_system_get_move_type());

	// Check layer keys for switching between keyboard layout layers
	int layer_index = 0;
	if      (keyboard_shift)   layer_index = 1;
	else if (keyboard_symbols) layer_index = 2;

	// Until we have more, this is capped at 2
	assert(layer_index < 3);

	// Get the right layout for this text context
	const keylayout_key_t* layer = nullptr;
	switch (keyboard_text_context) {
	case text_context_text:  layer = keyboard_active_layout->text  [layer_index]; break;
	case text_context_number:layer = keyboard_active_layout->number[layer_index]; break;
	case text_context_uri:   layer = keyboard_active_layout->uri   [layer_index]; break;
	default:                 layer = keyboard_active_layout->text  [layer_index]; break;
	}

	float gutter = ui_get_gutter();
	// Draw the keyboard
	float   button_size = ui_line_height();
	int32_t i           = 0;
	while (layer[i].special_key != skey_end) {
		const keylayout_key_t *curr = &layer[i];
		i += 1;

		float curr_width = curr->width * 0.5f;
		float width = curr_width * button_size;
		float width_pct = curr_width - (int)curr_width;
		if (curr_width > 1)
			width += (((int)curr_width) - 1) * gutter;

		if (curr_width < 1)
			width -= gutter/2;
		else if (width_pct != 0)
			width += gutter/2;

		vec2 size = vec2{ width, button_size };
		if (curr->special_key == skey_nextline) ui_nextline();
		if (curr_width == 0) continue;

		ui_push_idi(i + 1000);
		switch(curr->special_key) {
		case skey_spacer:   ui_layout_reserve(size); break;
		case skey_nextline: break;
		case skey_fn:       ui_toggle_sz(curr->display_text, keyboard_symbols, size); break;
		case skey_alt: {
			if (ui_toggle_sz(curr->display_text, keyboard_alt, size)) {
				if (keyboard_alt) input_key_inject_press  (key_alt);
				else              input_key_inject_release(key_alt);
			}
		} break;
		case skey_ctrl: {
			if (ui_toggle_sz(curr->display_text, keyboard_ctrl, size)) {
				if (keyboard_ctrl) input_key_inject_press  (key_ctrl);
				else               input_key_inject_release(key_ctrl);
			}
		} break;
		case skey_shift: {
			if (ui_toggle_img_sz(curr->display_text, keyboard_shift, nullptr, nullptr, ui_btn_layout_none, size)) {
				if (keyboard_shift) input_key_inject_press  (key_shift);
				else                input_key_inject_release(key_shift);
			}
		} break;
		case skey_syms: {
			ui_toggle_img_sz(curr->display_text, keyboard_symbols, nullptr, nullptr, ui_btn_layout_none, size);
		} break;
		default: {
			if (ui_button_sz(curr->display_text, size))
				virtualkeyboard_keypress(*curr);
		} break;
		}
		ui_pop_id();
		ui_sameline();
	}
	ui_window_end();
	hierarchy_pop();
	ui_pop_preserve_keyboard();
}

}