/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2023 Nick Klingensmith
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 */

#pragma once
#include "_platform.h"

#if defined(SK_OS_LINUX)
namespace sk {

bool  linux_get_cursor(vec2 &out_pos);
float linux_get_scroll();
void  linux_set_cursor(vec2 window_pos);

} // namespace sk
#endif // defined(SK_OS_LINUX)