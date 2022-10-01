#pragma once
#include "../stereokit.h"

namespace sk {

void virtualkeyboard_initialize();
void virtualkeyboard_update    ();
void virtualkeyboard_open      (bool32_t open, text_context_ type, vec3 from_world);
bool virtualkeyboard_get_open  ();

}
