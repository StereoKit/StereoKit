/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

#pragma once
#include "../stereokit.h"

namespace sk {

void virtualkeyboard_initialize();
void virtualkeyboard_shutdown  ();
void virtualkeyboard_step      ();
void virtualkeyboard_open      (bool32_t open, text_context_ type);
bool virtualkeyboard_set_layout(text_context_ type, const char** keyboard_layout, int layouts_num);
bool virtualkeyboard_get_open  ();

}
