#pragma once

#include "../stereokit.h"

namespace sk {

struct key_event_t {
	key_          key;
	button_state_ state;
	int32_t       character;
};

void          input_keyboard_initialize    ();
void          input_keyboard_shutdown      ();
void          input_keyboard_suspend       (bool suspend);
void          input_keyboard_update        ();
void          input_keyboard_inject_press  (key_ key);
void          input_keyboard_inject_release(key_ key);
button_state_ input_keyboard_get           (key_ key);

void          input_keyboard_inject_char (uint16_t character);
uint16_t      input_keyboard_char_consume();
void          input_keyboard_char_reset  ();

} // namespace sk