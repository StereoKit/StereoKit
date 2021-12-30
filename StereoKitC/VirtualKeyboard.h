#pragma once
#include "stereokit.h"

#ifdef __cplusplus
namespace sk {
#endif
	typedef enum special_key {
		special_key_none,
		special_key_shift,
		special_key_alt_gr,
		special_key_fn,
		special_key_close_keyboard,
		//Used to put more space between keys
		special_key_alt,
		special_key_ctrl,
		special_key_spacer,
	} special_key_;

	typedef struct keyboard_layout_Key_t {
		const char16_t* clicked_text;
		const char16_t* display_text;
		float width;
		key_ key_event_type;
		special_key_ special_key;
	} keyboard_layout_Key_t;

	typedef struct keyboard_layout_Layer_t {
		keyboard_layout_Key_t normal_keys[8][35];
	} keyboard_layout_Layer_t;

	typedef struct keyboard_layout_t {
		keyboard_layout_Layer_t text_layer[5];
		keyboard_layout_Layer_t text_uri_layer[5];
		keyboard_layout_Layer_t number_layer[5];
		keyboard_layout_Layer_t number_decimal_layer[5];
		keyboard_layout_Layer_t number_signed_layer[5];
		keyboard_layout_Layer_t number_signed_decimal_layer[5];
		// of all of these arrays
		// first layer is normal key layout array index 0
		// second layer is shift array index 1
		// third layer is AltGr array index 2
		// fourth layer is AltGr + Shift array index 3
		// fifth layer in fnkey array index 4
	} keyboard_layout_t;

	void     virtualkeyboard_initialize();

	void     virtualkeyboard_update();
	void     virtualkeyboard_open(bool open, text_context_ type);
	bool     virtualkeyboard_get_open();

#ifdef __cplusplus
}
#endif
