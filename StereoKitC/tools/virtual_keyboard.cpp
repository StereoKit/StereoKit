/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2023 Nick Klingensmith
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 */

#include "virtual_keyboard.h"
#include "virtual_keyboard_layouts.h"
#include "../stereokit_ui.h"
#include "../sk_math.h"
#include "../libraries/array.h"
#include "../libraries/unicode.h"
#include "../libraries/stref.h"

namespace sk {

bool32_t           keyboard_open         = false;
pose_t             keyboard_pose         = pose_identity;
array_t<key_>      keyboard_pressed_keys = {};
const keylayout_t* keyboard_active_layout;
int32_t            keyboard_layer_idx = 0;
text_context_      keyboard_text_context;

///////////////////////////////////////////

bool virtualkeyboard_parse_layout(const char* text_start, int32_t text_length, keylayout_info_t* out_layout);

///////////////////////////////////////////

const keylayout_t* virtualkeyboard_get_system_keyboard_layout() {
	return &virtualkeyboard_layout_en_us;
}

///////////////////////////////////////////

void virtualkeyboard_reset_modifiers() {
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
	keyboard_layer_idx    = 0;
}

///////////////////////////////////////////

bool virtualkeyboard_get_open() {
	return keyboard_open;
}

///////////////////////////////////////////

void virtualkeyboard_initialize() {
	const char* layout = R"(q|w|e|r|t|y|u|i|o|p
---1|a|s|d|f|g|h|j|k|l
spr:sk/ui/shift---3-go_1|z|x|c|v|b|n|m|spr:sk/ui/backspace-\b--3
spr:sk/ui/close----close|123---3-go_2|,| - --7|.|Return-\n--4|)";
	virtualkeyboard_parse_layout(layout, strlen(layout), &virtualkeyboard_layout_en_us.text[0]);
	layout = R"(Q|W|E|R|T|Y|U|I|O|P
---1|A|S|D|F|G|H|J|K|L
spr:sk/ui/shift---3-go_0|Z|X|C|V|B|N|M|spr:sk/ui/backspace-\b--3
spr:sk/ui/close----close|123---3-go_2|!| - --7|?|Return-\n--4|)";
	virtualkeyboard_parse_layout(layout, strlen(layout), &virtualkeyboard_layout_en_us.text[1]);
	layout = R"(1|2|3|4|5|6|7|8|9|0
---1|\-|/|:|;|(|)|$|&|@
spr:sk/ui/shift---3-go_0|*|=|+|#|%|'|"|spr:sk/ui/backspace-\b--3
spr:sk/ui/close----close|123---3-go_2|!| - --7|?|Return-\n--4|)";
	virtualkeyboard_parse_layout(layout, strlen(layout), &virtualkeyboard_layout_en_us.text[2]);
	layout = R"(7|8|9|spr:sk/ui/backspace-\b
4|5|6|\-
1|2|3
0|.|Return-\n--4-close|)";
	virtualkeyboard_parse_layout(layout, strlen(layout), &virtualkeyboard_layout_en_us.number[0]);
	layout = R"(`|1|2|3|4|5|6|7|8|9|0|\-|=|spr:sk/ui/backspace-\b--3|---2|spr:sk/ui/close----close
Tab-\t--3|q|w|e|r|t|y|u|i|o|p|[|]|\\|.com-.com--4
Enter-\n--4|a|s|d|f|g|h|j|k|l|;|'|Enter-\n--3|https://-https://--4
spr:sk/ui/shift---5-go_1|z|x|c|v|b|n|m|,|.|/|spr:sk/ui/shift---4-go_1|spr:sk/ui/arrow_up--38
Ctrl---4|Cmd---3|Alt---3| - --9|Alt---4|Ctrl---4|spr:sk/ui/arrow_left--37|spr:sk/ui/arrow_down--40|spr:sk/ui/arrow_right--39|)";
	virtualkeyboard_parse_layout(layout, strlen(layout), &virtualkeyboard_layout_en_us.uri[0]);
	layout = R"(~|!|@|#|$|%|^|&|*|(|)|_|+|spr:sk/ui/backspace-\b--3|---2|spr:sk/ui/close----close
Tab-\t--3|Q|W|E|R|T|Y|U|I|O|P|{|}|\||.com-.com--4
Enter-\n--4|A|S|D|F|G|H|J|K|L|:|"|Enter-\n--3|https://-https://--4
spr:sk/ui/shift---5-go_0|Z|X|C|V|B|N|M|<|>|?|spr:sk/ui/shift---4-go_0|spr:sk/ui/arrow_up--38
Ctrl---4|Cmd---3|Alt---3| - --9|Alt---4|Ctrl---4|spr:sk/ui/arrow_left--37|spr:sk/ui/arrow_down--40|spr:sk/ui/arrow_right--39|)";
	virtualkeyboard_parse_layout(layout, strlen(layout), &virtualkeyboard_layout_en_us.uri[1]);

	keyboard_active_layout = virtualkeyboard_get_system_keyboard_layout();
}

///////////////////////////////////////////

void send_key_data(const char* charkey,key_ key) {
	if (key != key_none) {
		keyboard_pressed_keys.add(key);
		input_key_inject_press(key);
	}
	if (charkey != nullptr) {
		char32_t c = 0;
		while (utf8_decode_fast_b(charkey, &charkey, &c)) {
			input_text_inject_char(c);
		}
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
	virtualkeyboard_reset_modifiers();
}

///////////////////////////////////////////

bool _key_button(const keylayout_key_t* key, vec2 size) {
	return key->is_sprite
		? ui_button_img_sz("key", key->display_sprite, ui_btn_layout_center_no_text, size)
		: ui_button_sz(key->display_text, size);
}

///////////////////////////////////////////

void virtualkeyboard_update() {
	if (!keyboard_open) return;
	if (keyboard_active_layout == nullptr) return;

	// Get the right layout for this text context
	const keylayout_info_t* layer = nullptr;
	switch (keyboard_text_context) {
	case text_context_text:  layer = &keyboard_active_layout->text  [keyboard_layer_idx]; break;
	case text_context_number:layer = &keyboard_active_layout->number[keyboard_layer_idx]; break;
	case text_context_uri:   layer = &keyboard_active_layout->uri   [keyboard_layer_idx]; break;
	default:                 layer = &keyboard_active_layout->text  [keyboard_layer_idx]; break;
	}

	float gutter      = ui_get_gutter ();
	float margin      = ui_get_margin ();
	float cell_height = ui_line_height();
	float cell_size   = cell_height * 0.5;
	// Calculate the width in advance, so the keyboard doesn't "pop"
	float window_width = cell_size * layer->width_cells + gutter * layer->width_gutters + margin * 2;

	remove_last_clicked_keys();
	ui_push_preserve_keyboard(true);
	hierarchy_push(render_get_cam_root());
	ui_window_begin("SK/Keyboard", keyboard_pose, {window_width,0}, ui_win_body, ui_system_get_move_type());

	// Draw the keyboard
	for (int32_t i=0; i<layer->key_ct; i+=1) {
		const keylayout_key_t *curr = &layer->keys[i];

		float final_width = curr->width * cell_size; // Width starts with the number of cells it crossed.
		final_width += (int32_t)(curr->width/2.0f - 0.5f) * gutter; // for every 2 cells, we need to jump a gutter
		if (curr->width%2 == 1) final_width -= gutter/2; // if it doesn't evenly land on a gutter, we need an offset to make it look right.
		vec2 size = vec2{ final_width, cell_height };

		ui_push_idi(i + 1000);
		if      (curr->special_key  == skey_nextline) { ui_nextline(); }
		else if (curr->display_text == nullptr      ) { ui_layout_reserve(size); ui_sameline(); }
		else if (curr->special_key  == skey_close   ) {
			ui_push_tint({ 0.8f,0.8f,0.8f, 1 });
			if (_key_button(curr, size)) keyboard_open = false;
			ui_pop_tint();
			ui_sameline();
		} else if (curr->special_key >= skey_go) {
			ui_push_tint({ 0.8f,0.8f,0.8f, 1 });
			if (_key_button(curr, size)) {
				keyboard_layer_idx = curr->special_key - skey_go;
			}
			ui_pop_tint();
			ui_sameline();
		} else {
			if (_key_button(curr, size))
				virtualkeyboard_keypress(*curr);
			ui_sameline();
		}
		ui_pop_id();
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

bool virtualkeyboard_parse_layout(const char* text_start, int32_t text_length, keylayout_info_t *out_layout) {
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
		if (escape) {
			switch (*src) {
			case 'n': *dst = '\n'; break;
			case 'r': *dst = '\r'; break;
			case '|': *dst = '|';  break;
			case '-': *dst = '-';  break;
			case 't': *dst = '\t'; break;
			case 'b': *dst = '\b'; break;
			case '\\':*dst = '\\'; break;
			default:  *dst = *src; break;
			}
			dst++;
			escape = false;
		} else if (*src == '\\') {
			escape = true;
		} else {
			if      (*src == '\n') { *dst = '\1'; key_count+=2; }
			else if (*src == '\r') { }
			else if (*src == '|')  { *dst = '\2'; key_count++; }
			else if (*src == '-')  { *dst = '\3'; }
			else *dst = *src;
			dst++;
		}
		src++;
	}
	*dst = '\0';

	// Parse the keys into a full keyboard
	keylayout_key_t *result  = sk_malloc_zero_t(keylayout_key_t, key_count + 1);
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
				stref_t arg_stref = { arg, (uint32_t)arg_len };
				switch (arg_idx) {
				case 0: {
					if (arg_len > 0) {
						if (stref_startswith(arg_stref, "spr:")) {
							char *id = stref_copy(stref_substr(arg + 4, arg_len - 4));
							result[key_idx].display_sprite = sprite_find(id);
							result[key_idx].is_sprite      = true;
							sk_free(id);
						} else {
							result[key_idx].display_text = result[key_idx].clicked_text = stref_copy(arg_stref);
						}
					}
				} break;
				case 1: result[key_idx].clicked_text = arg_len > 0 ? stref_copy(arg_stref) : nullptr; break;
				case 2: if (arg_len > 0) result[key_idx].key_event_type = (uint8_t)stref_to_i(arg_stref); break;
				case 3: if (arg_len > 0) result[key_idx].width = (uint8_t)stref_to_i(arg_stref); break;
				case 4: {
					if      (stref_startswith(arg_stref, "go_"  )) { result[key_idx].special_key = skey_go + stref_to_i({ arg_stref.start+3, arg_stref.length - 3 }); }
					else if (stref_equals    (arg_stref, "close")) { result[key_idx].special_key = skey_close; }
					else { result[key_idx].special_key = skey_none; }
				} break;
				}
				arg_idx++;
			}
			key_idx += 1;
		}
		result[key_idx].special_key = skey_nextline;
		result[key_idx].width = 0;
		key_idx += 1;
	}

	*out_layout = {};
	out_layout->keys   = result;
	out_layout->key_ct = key_count;
	for (int32_t i = 0; i < key_count; i++)
	{
		if (result[i].special_key == skey_nextline) {
			out_layout->width_gutters -= 1;
			break;
		}
		out_layout->width_cells   += result[i].width;
		out_layout->width_gutters += 1 + (int32_t)(result[i].width / 2.0f - 0.5f);
	}
	return true;
}

}