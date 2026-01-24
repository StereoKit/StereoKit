/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2023 Nick Klingensmith
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 */

#pragma once
#include "_platform.h"

#if defined(SK_OS_WINDOWS)
namespace sk {

wchar_t *platform_to_wchar  (const char    *utf8_string);
char    *platform_from_wchar(const wchar_t *string);

} // namespace sk
#endif // defined(SK_OS_WINDOWS)
