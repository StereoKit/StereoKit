#pragma once

#include "../stereokit.h"

namespace sk {

struct key_event_t {
	key_          key;
	button_state_ state;
	char32_t      character;
};

void          input_keyboard_initialize();
void          input_keyboard_shutdown  ();
void          input_keyboard_suspend   (bool suspend);
void          input_keyboard_update    ();
button_state_ input_keyboard_get       (key_ key);
button_state_ input_keyboard_get_code  (key_ key);

extern float input_last_physical_keypress;

} // namespace sk