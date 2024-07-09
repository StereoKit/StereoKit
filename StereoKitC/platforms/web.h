/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2023 Nick Klingensmith
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 */

#pragma once
#include "_platform.h"

#if defined(SK_OS_WEB)
namespace sk {

void web_start_main_loop(void (*app_update)(void), void (*app_shutdown)(void));

bool  web_get_cursor(vec2 &out_pos);
void  web_set_cursor(vec2 window_pos);
float web_get_scroll();

} // namespace sk
#endif // defined(SK_OS_WEB)