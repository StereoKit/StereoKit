/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2023 Nick Klingensmith
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 */

#pragma once
#include "_platform.h"

#if defined(SK_OS_WINDOWS_UWP)
namespace sk {

bool  uwp_get_mouse   (vec2 &out_pos);
void  uwp_set_mouse   (vec2 window_pos);
float uwp_get_scroll  ();

void  uwp_show_keyboard   (bool show);
bool  uwp_keyboard_visible();

} // namespace sk
#endif // defined(SK_OS_WINDOWS_UWP)