#pragma once

#include "../stereokit.h"

namespace sk {

void          input_keyboard_initialize();
void          input_keyboard_shutdown  ();
void          input_keyboard_suspend   (bool suspend);
void          input_keyboard_update    ();
button_state_ input_keyboard_get       (key_ key);
bool          input_text_insertable    (char32_t character);

float         input_get_last_physical_keypress_time();
void          input_set_last_physical_keypress_time(float time);

} // namespace sk