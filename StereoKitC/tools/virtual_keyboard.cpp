/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2023 Nick Klingensmith
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 */

#include "virtual_keyboard.h"

#include "../stereokit_ui.h"
#include "../sk_math.h"
#include "../libraries/array.h"
#include "../libraries/unicode.h"
#include "../libraries/stref.h"
#include "../libraries/ferr_hash.h"

namespace sk {

typedef enum skey_ {
	skey_none,
	skey_close,
	skey_spacer,
	skey_nextline,
	skey_mod,
	skey_go,
	skey_visit,
} skey_;

typedef struct keylayout_key_t {
	const char*  clicked_text;
	union {
		const char* display_text;
		sprite_t    display_sprite;
	};
	uint8_t      width;
	uint8_t      key_event_type;
	uint8_t      special_key;
	uint8_t      is_sprite;
	uint8_t      go_to;
} keylayout_key_t;

typedef struct keylayout_info_t {
	keylayout_key_t* keys;
	int32_t          key_ct;
	int32_t          width_cells;
	int32_t          width_gutters;
} keylayout_info_t;

bool32_t                  keyboard_open             = false;
pose_t                    keyboard_pose             = pose_identity;
array_t<key_>             keyboard_pressed_keys     = {};
array_t<uint64_t>         keyboard_modifier_keys    = {};
uint64_t                  keyboard_go_toggle        = 0;
int32_t                   keyboard_visit_return_idx = -1;
text_context_             keyboard_text_context     = text_context_text;

array_t<keylayout_info_t> keyboard_layers           = {};
int32_t                   keyboard_active_root      = 0;
int32_t                   keyboard_active_idx       = 0;
int32_t                   keyboard_ctx_root_text    = 0;
int32_t                   keyboard_ctx_root_number  = 0;
int32_t                   keyboard_ctx_root_uri     = 0;

///////////////////////////////////////////

bool virtualkeyboard_parse_layout(const char* text_start, int32_t text_length, keylayout_info_t* out_layout);
void virtualkeyboard_go_to       (int32_t layout_idx);

///////////////////////////////////////////

uint64_t virtualkeyboard_hash(const keylayout_key_t* key) {
	return key->is_sprite
		? (uint64_t)key->display_sprite
		: (key->display_text != nullptr
			? hash_fnv64_string(key->display_text)
			: 0);
}

///////////////////////////////////////////

void virtualkeyboard_reset_modifiers() {
	const keylayout_info_t* keyboard = &keyboard_layers[keyboard_active_root + keyboard_active_idx];

	for (int32_t i = keyboard_modifier_keys.count-1; i >= 0; i-=1) {
		for (int32_t k = 0; k < keyboard->key_ct; k+=1) {
			keylayout_key_t *key = &keyboard->keys[k];
			uint64_t hash = virtualkeyboard_hash(key);
			if (hash == keyboard_modifier_keys[i]) {
				if (key->key_event_type != key_none) input_key_inject_release((key_)key->key_event_type);
				break;
			}
		}
	}
	keyboard_modifier_keys.clear();
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

	keyboard_text_context     = type;
	keyboard_open             = open;
	keyboard_visit_return_idx = -1;

	// Get the right layout for this text context
	switch (type) {
	case text_context_text:  keyboard_active_root = keyboard_ctx_root_text;   break;
	case text_context_number:keyboard_active_root = keyboard_ctx_root_number; break;
	case text_context_uri:   keyboard_active_root = keyboard_ctx_root_uri;    break;
	default:                 keyboard_active_root = keyboard_ctx_root_text;   break;
	}
	keyboard_active_idx = 0;
}

///////////////////////////////////////////

bool virtualkeyboard_get_open() {
	return keyboard_open;
}

///////////////////////////////////////////

void virtualkeyboard_initialize() {
	keylayout_info_t layer  = {};
	const char*      layout = nullptr;

	// These keyboards use VK codes for parity with the previous ones. Until
	// the new smaller mobile keyboard ships, we'll just stick with that, but
	// it's reasonable to imagine a virtual keyboard might not actually use VK
	// codes at all.

	////// Text keyboard //////
	layout =
R"(`-`-192|1-1-49|2-2-50|3-3-51|4-4-52|5-5-53|6-6-54|7-7-55|8-8-56|9-9-57|0-0-48|\--\--189|=-=-187|spr:sk/ui/backspace-\b-8-3|spr:sk/ui/close----close
Tab-\t-9-3|q-q-81|w-w-87|e-e-69|r-r-82|t-t-84|y-y-89|u-u-85|i-i-73|o-o-79|p-p-80|[-[-219|]-]-221|\\-\\-220
Enter-\n-13-4|a-a-65|s-s-83|d-d-68|f-f-70|g-g-71|h-h-72|j-j-74|k-k-75|l-l-76|;-;-186|'-'-222|Enter-\n-13-3
spr:sk/ui/shift--16-5-visit_1|z-z-90|x-x-88|c-c-67|v-v-86|b-b-66|n-n-78|m-m-77|,-,-188|.-.-190|/-/-191|spr:sk/ui/shift--16-2-visit_1|spr:sk/ui/arrow_up--38
Ctrl--17-4-mod|Cmd--91-3|Alt--18-3-mod| - -32-9|Alt--18-3-mod|Ctrl--17-3-mod|spr:sk/ui/arrow_left--37|spr:sk/ui/arrow_down--40|spr:sk/ui/arrow_right--39|)";
	if (virtualkeyboard_parse_layout(layout, (int32_t)strlen(layout), &layer)) keyboard_ctx_root_text = keyboard_layers.add(layer);
	layout =
R"(~-~-192|!-!-49|@-@-50|#-#-51|$-$-52|%-%-53|^-^-54|&-&-55|*-*-56|(-(-57|)-)-48|_-_-189|+-+-187|spr:sk/ui/backspace-\b-8-3|spr:sk/ui/close----close
Tab-\t-9-3|Q-Q-81|W-W-87|E-E-69|R-R-82|T-T-84|Y-Y-89|U-U-85|I-I-73|O-O-79|P-P-80|{-{-219|}-}-221|\|-\|-220
Enter-\n-13-4|A-A-65|S-S-83|D-D-68|F-F-70|G-G-71|H-H-72|J-J-74|K-K-75|L-L-76|:-:-186|"-"-222|Enter-\n-13-3
spr:sk/ui/shift--16-5-go_0|Z-Z-90|X-X-88|C-C-67|V-V-86|B-B-66|N-N-78|M-M-77|<-<-188|>->-190|?-?-191|spr:sk/ui/shift--16-2-go_0|spr:sk/ui/arrow_up--38
Ctrl--17-4-mod|Cmd--91-3|Alt--18-3-mod| - -32-9|Alt--18-3-mod|Ctrl--17-3-mod|spr:sk/ui/arrow_left--37|spr:sk/ui/arrow_down--40|spr:sk/ui/arrow_right--39|)";
	if (virtualkeyboard_parse_layout(layout, (int32_t)strlen(layout), &layer)) keyboard_layers.add(layer);

	////// URI keyboard //////
	layout =
R"(`-`-192|1-1-49|2-2-50|3-3-51|4-4-52|5-5-53|6-6-54|7-7-55|8-8-56|9-9-57|0-0-48|\--\--189|=-=-187|spr:sk/ui/backspace-\b-8-3|---2|spr:sk/ui/close----close
Tab-\t-9-3|q-q-81|w-w-87|e-e-69|r-r-82|t-t-84|y-y-89|u-u-85|i-i-73|o-o-79|p-p-80|[-[-219|]-]-221|\\-\\-220|.com-.com--4
Enter-\n-13-4|a-a-65|s-s-83|d-d-68|f-f-70|g-g-71|h-h-72|j-j-74|k-k-75|l-l-76|;-;-186|'-'-222|Enter-\n-13-3|.net-.net--4
spr:sk/ui/shift--16-5-visit_1|z-z-90|x-x-88|c-c-67|v-v-86|b-b-66|n-n-78|m-m-77|,-,-188|.-.-190|/-/-191|spr:sk/ui/shift--16-2-visit_1|spr:sk/ui/arrow_up--38|https://-https://--4
Ctrl--17-4-mod|Cmd--91-3|Alt--18-3-mod| - -32-9|Alt--18-3-mod|Ctrl--17-3-mod|spr:sk/ui/arrow_left--37|spr:sk/ui/arrow_down--40|spr:sk/ui/arrow_right--39|)";
	if (virtualkeyboard_parse_layout(layout, (int32_t)strlen(layout), &layer)) keyboard_ctx_root_uri = keyboard_layers.add(layer);
	layout =
R"(~-~-192|!-!-49|@-@-50|#-#-51|$-$-52|%-%-53|^-^-54|&-&-55|*-*-56|(-(-57|)-)-48|_-_-189|+-+-187|spr:sk/ui/backspace-\b-8-3|---2|spr:sk/ui/close----close
Tab-\t-9-3|Q-Q-81|W-W-87|E-E-69|R-R-82|T-T-84|Y-Y-89|U-U-85|I-I-73|O-O-79|P-P-80|{-{-219|}-}-221|\|-\|-220|.com-.com--4
Enter-\n-13-4|A-A-65|S-S-83|D-D-68|F-F-70|G-G-71|H-H-72|J-J-74|K-K-75|L-L-76|:-:-186|"-"-222|Enter-\n-13-3|.net-.net--4
spr:sk/ui/shift--16-5-go_0|Z-Z-90|X-X-88|C-C-67|V-V-86|B-B-66|N-N-78|M-M-77|<-<-188|>->-190|?-?-191|spr:sk/ui/shift--16-2-go_0|spr:sk/ui/arrow_up--38|https://-https://--4
Ctrl--17-4-mod|Cmd--91-3|Alt--18-3-mod| - -32-9|Alt--18-3-mod|Ctrl--17-3-mod|spr:sk/ui/arrow_left--37|spr:sk/ui/arrow_down--40|spr:sk/ui/arrow_right--39|)";
	if (virtualkeyboard_parse_layout(layout, (int32_t)strlen(layout), &layer)) keyboard_layers.add(layer);

	////// Numeric keyboard //////
	layout =
R"(7|8|9|spr:sk/ui/backspace-\b
4|5|6|\-
1|2|3
0|.|Return-\n--4-close|)";
	if (virtualkeyboard_parse_layout(layout, (int32_t)strlen(layout), &layer)) keyboard_ctx_root_number = keyboard_layers.add(layer);

	// Mobile friendly design. This needs touch selection functionality for Input
	// elements before it can be shipped.
	/*layout =
R"(q|w|e|r|t|y|u|i|o|p
---1|a|s|d|f|g|h|j|k|l
spr:sk/ui/shift---3-go_1|z|x|c|v|b|n|m|spr:sk/ui/backspace-\b--3
spr:sk/ui/close----close|123---3-go_2|,| - --7|.|Return-\n--4|)";
	if (virtualkeyboard_parse_layout(layout, strlen(layout), &layer)) keyboard_ctx_root_text = keyboard_layers.add(layer);
	layout =
R"(Q|W|E|R|T|Y|U|I|O|P
---1|A|S|D|F|G|H|J|K|L
spr:sk/ui/shift---3-go_0|Z|X|C|V|B|N|M|spr:sk/ui/backspace-\b--3
spr:sk/ui/close----close|123---3-go_2|!| - --7|?|Return-\n--4|)";
	if (virtualkeyboard_parse_layout(layout, strlen(layout), &layer)) keyboard_layers.add(layer);
	layout =
R"(1|2|3|4|5|6|7|8|9|0
---1|\-|/|:|;|(|)|$|&|@
spr:sk/ui/shift---3-go_0|*|=|+|#|%|'|"|spr:sk/ui/backspace-\b--3
spr:sk/ui/close----close|123---3-go_0|!| - --7|?|Return-\n--4|)";
	if (virtualkeyboard_parse_layout(layout, strlen(layout), &layer)) keyboard_layers.add(layer);*/

	keyboard_active_root = keyboard_ctx_root_text;
	keyboard_active_idx  = 0;
}

///////////////////////////////////////////

void virtualkeyboard_release_keys() {
	for (int32_t i = 0; i < keyboard_pressed_keys.count; i++) {
		input_key_inject_release(keyboard_pressed_keys[i]);
	}
	keyboard_pressed_keys.clear();
}

///////////////////////////////////////////

void submit_chars(const char *utf8) {
	if (utf8 == nullptr) return;

	char32_t c = 0;
	while (utf8_decode_fast_b(utf8, &utf8, &c)) {
		input_text_inject_char(c);
	}
}

///////////////////////////////////////////

void virtualkeyboard_keypress(keylayout_key_t key) {
	if (key.key_event_type != key_none) {
		keyboard_pressed_keys.add((key_)key.key_event_type);
		input_key_inject_press((key_)key.key_event_type);
	}
	if (keyboard_modifier_keys.count == 0)
		submit_chars(key.clicked_text);
	virtualkeyboard_reset_modifiers();

	if (keyboard_visit_return_idx != -1) {
		virtualkeyboard_go_to(keyboard_visit_return_idx);
		keyboard_visit_return_idx = -1;
		keyboard_go_toggle        = 0;
	}
}

///////////////////////////////////////////

void virtualkeyboard_keymodifier(keylayout_key_t key, bool32_t modifier_state) {
	uint64_t hash = virtualkeyboard_hash(&key);
	if (modifier_state) {
		keyboard_modifier_keys.add(hash);
	} else {
		int32_t idx = keyboard_modifier_keys.index_of(hash);
		if (idx >= 0) keyboard_modifier_keys.remove(idx);
	}

	if (key.key_event_type != key_none) {
		if (modifier_state) input_key_inject_press  ((key_)key.key_event_type);
		else                input_key_inject_release((key_)key.key_event_type);
	}
	if (modifier_state) submit_chars(key.clicked_text);
}

///////////////////////////////////////////

void virtualkeyboard_go_to(int32_t layout_idx) {
	keyboard_active_idx = layout_idx;
}

///////////////////////////////////////////

bool _key_button(const keylayout_key_t* key, vec2 size) {
	return key->is_sprite
		? ui_button_img_sz("key", key->display_sprite, ui_btn_layout_center_no_text, size)
		: ui_button_sz(key->display_text, size);
}

bool _key_toggle(const keylayout_key_t* key, bool32_t *toggle, vec2 size) {
	return key->is_sprite
		? ui_toggle_img_sz("key", *toggle, key->display_sprite, key->display_sprite, ui_btn_layout_center_no_text, size)
		: ui_toggle_img_sz(key->display_text, *toggle, nullptr, nullptr, ui_btn_layout_none, size);
}

///////////////////////////////////////////

void virtualkeyboard_update() {
	if (!keyboard_open) return;
	const keylayout_info_t* keyboard = &keyboard_layers[keyboard_active_root + keyboard_active_idx];

	float gutter      = ui_get_gutter ();
	float margin      = ui_get_margin ();
	float cell_height = ui_line_height();
	float cell_size   = cell_height * 0.5f;
	// Calculate the width in advance, so the keyboard doesn't "pop"
	float window_width = cell_size * keyboard->width_cells + gutter * keyboard->width_gutters + margin * 2;

	// Make sure any keys pressed last frame get a release event this frame.
	virtualkeyboard_release_keys();

	ui_push_preserve_keyboard(true);
	hierarchy_push(render_get_cam_root());
	ui_push_idi(keyboard_active_root+keyboard_active_idx);
	ui_window_begin("SK/Keyboard", keyboard_pose, {window_width,0}, ui_win_body, ui_system_get_move_type());

	// Draw the keyboard
	for (int32_t i=0; i< keyboard->key_ct; i+=1) {
		const keylayout_key_t *curr = &keyboard->keys[i];

		float final_width = curr->width * cell_size; // Width starts with the number of cells it crossed.
		final_width += (int32_t)(curr->width/2.0f - 0.5f) * gutter; // for every 2 cells, we need to jump a gutter
		if (curr->width%2 == 1) final_width -= gutter/2; // if it doesn't evenly land on a gutter, we need an offset to make it look right.
		vec2 size = vec2{ final_width, cell_height };

		ui_push_idi(i + 1000);
		if      (curr->special_key  == skey_nextline) { ui_nextline(); }
		else if (curr->display_text == nullptr      ) { ui_layout_reserve(size); ui_sameline(); }
		else if (curr->special_key  == skey_close   ) {
			ui_push_tint({ 0.8f,0.8f,0.8f, 1 });
			if (_key_button(curr, size)) virtualkeyboard_open(false, text_context_text);
			ui_pop_tint();
			ui_sameline();
		} else if (curr->special_key == skey_mod) {
			uint64_t hash    = virtualkeyboard_hash(curr);
			bool32_t toggled = false;
			for (int32_t t = 0; t < keyboard_modifier_keys.count; t++) {
				if (keyboard_modifier_keys[t] == hash) {
					toggled = true;
					break;
				}
			}
			if (_key_toggle(curr, &toggled, size)) {
				virtualkeyboard_keymodifier(*curr, toggled);
			}
			ui_sameline();
		} else if (curr->special_key == skey_go || curr->special_key == skey_visit) {
			uint64_t hash    = virtualkeyboard_hash(curr);
			bool32_t toggled = keyboard_go_toggle == hash;

			ui_push_tint({ 0.8f,0.8f,0.8f, 1 });
			if (_key_toggle(curr, &toggled, size)) {
				keyboard_visit_return_idx = curr->special_key == skey_visit && toggled ? keyboard_active_idx : -1;
				keyboard_go_toggle        = toggled ? hash : 0;
				virtualkeyboard_go_to(curr->go_to);
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
	ui_pop_id();
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
	*out_next_length = (int32_t)(curr - *out_next);
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
	//              the index. 'visit_#' will switch the keyboard, but return
	//              after a key has been pressed. 'mod' will treat the key as a
	//              toggleable modifier key, like alt or ctrl, and will not submit
	//              characters.
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
	int32_t     text_len = (int32_t)strlen(escape_text);
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
					if      (stref_startswith(arg_stref, "go_"   )) { result[key_idx].special_key = skey_go;    result[key_idx].go_to = (uint8_t)stref_to_i({ arg_stref.start+3, arg_stref.length - 3 }); }
					else if (stref_startswith(arg_stref, "visit_")) { result[key_idx].special_key = skey_visit; result[key_idx].go_to = (uint8_t)stref_to_i({ arg_stref.start+6, arg_stref.length - 6 }); }
					else if (stref_equals    (arg_stref, "close" )) { result[key_idx].special_key = skey_close; }
					else if (stref_equals    (arg_stref, "mod"   )) { result[key_idx].special_key = skey_mod; }
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