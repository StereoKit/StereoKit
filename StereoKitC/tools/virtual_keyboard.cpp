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
#include "../libraries/stref.h"

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

keylayout_key_t* virtualkeyboard_parse_layout(const char* text_start, int32_t text_length);

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

///////////////////////////////////////////

bool next_separator(const char* start, int32_t start_len, const char **out_next, int32_t *out_next_length, char sep) {
	const char* end = start + start_len;
	if (*out_next == nullptr) {
		*out_next        = start;
		*out_next_length = 0;
	} else {
		*out_next        = *out_next + *out_next_length + 1;
		*out_next_length = 0;
	}
	const char* curr = *out_next;
	while (curr < end && *curr != sep) curr++;
	*out_next_length = curr - *out_next;
	return *out_next < end;
}

///////////////////////////////////////////

keylayout_key_t* virtualkeyboard_parse_layout(const char* text_start, int32_t text_length) {
	// File format is:
	// '|' separates keys on a row
	// '-' separates arguments for defining a key
	// A full definition would be:
	// KeyDisplay-KeySend-KeyPress-Width-Behavior
	// KeyDisplay - The text displayed on the button, or parentheses for indicating
	//              a sprite, eg: '(ui/sprite_close)'
	// KeySend    - The text injected when the key is pressed. If not present, this
	//              will be the same as the KeyDisplay, and if empty, this will not
	//              submit any text
	// KeyPress   - The key code to inject into the input system. This is none by
	//              default.
	// Width      - The width of the key, in keyboard cells. 1 is half a cell, 2 is
	//              a whole cell. This defaults to 2 if not specified.
	// Behavior   - Special behavior for the key. 'close' will close the keyboard,
	//              'go_#' will switch to the keyboard to the layout specified by
	//              the index.
	// 
	// For example:
	// 
	// q|w|e|r|t|y|u|i|o|p
	// ---1|a|s|d|f|g|h|j|k|l
	// Shift---3-go_1|z|x|c|v|b|n|m|<\--\b--3
	// X----close|123----go_2|,|- --7|.|Return-\n--4

	// Fix escape characters, and transform our syntax characters into chars that
	// won't get into our way. Count how many keys while we're at it :)
	char*       escape_text = sk_malloc_t(char, text_length+1);
	const char* src    = text_start;
	char*       dst    = escape_text;
	bool        escape = false;
	int32_t     key_count = 0;
	for (int32_t i = 0; i < text_length; i++) {
		if (*src == '\\') {
			escape = true;
		} else if (escape) {
			switch (*src) {
			case 'n': *dst = '\n'; break;
			case 'r': *dst = '\r'; break;
			case 't': *dst = '\t'; break;
			case 'b': *dst = '\b'; break;
			case '\\':*dst = '\\'; break;
			case '-': *dst = '-';  break;
			default:  *dst = *src; break;
			}
			dst++;
			escape = false;
		} else {
			if      (*src == '\n') { *dst = '\1'; key_count++; } // todo, may need extra keys for newlines the way it's set up now
			if      (*src == '\r') { }
			else if (*src == '|')  { *dst = '\2'; key_count++; }
			else if (*src == '-')  { *dst = '\3'; }
			else *dst = *src;
			dst++;
		}
		src++;
	}
	*dst = '\0';

	// Parse the keys into a full keyboard
	keylayout_key_t * result  = sk_malloc_zero_t(keylayout_key_t, key_count + 1);
	const char* text     = escape_text;
	int32_t     text_len = strlen(escape_text);
	const char* line     = nullptr;
	int32_t     line_len = 0;
	int32_t     key_idx  = 0;
	while (next_separator(text, text_len, &line, &line_len, '\1')) {
		const char* word     = nullptr;
		int32_t     word_len = 0;
		while (next_separator(line, line_len, &word, &word_len, '\2')) {
			const char* arg = nullptr;
			int32_t     arg_len = 0;
			int32_t     arg_idx = 0;
			result[key_idx].width = 2;

			while (next_separator(word, word_len, &arg, &arg_len, '\3')) {
				stref_t arg_stref = { arg, arg_len };
				switch (arg_idx) {
				case 0: if (arg_len > 0) result[key_idx].display_text = result[key_idx].clicked_text = stref_copy(arg_stref); break;
				case 1: if (arg_len > 0) result[key_idx].clicked_text = stref_copy(arg_stref); break;
				case 2: break; // todo, parse key codes
				case 3: if (arg_len > 0) result[key_idx].width = (uint8_t)stref_to_i(arg_stref); break;
				case 4: {
					if      (stref_startswith(arg_stref, "go_"  )) { result[key_idx].special_key = skey_go + stref_to_i({ arg_stref.start, arg_stref.length - 3 }); }
					else if (stref_equals    (arg_stref, "close")) { result[key_idx].special_key = skey_close; }
					else { result[key_idx].special_key = skey_none; }
				} break;
				}
				arg_idx++;
			}
			key_idx += 1;
		}
	}
	return result;
}

}