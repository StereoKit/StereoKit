#pragma once
#include "../stereokit.h"

namespace sk {

void virtualkeyboard_initialize();
void virtualkeyboard_update    ();
void virtualkeyboard_open      (bool32_t open, text_context_ type);
bool virtualkeyboard_set_layout(text_context_ type, char** keyboard_layout, int layouts_num);
bool virtualkeyboard_get_open  ();

}
