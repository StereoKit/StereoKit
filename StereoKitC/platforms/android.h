/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2025 Nick Klingensmith
 * Copyright (c) 2023-2025 Qualcomm Technologies, Inc.
 */

#pragma once
#include "_platform.h"

#if defined(SK_OS_ANDROID)

namespace sk {

void android_set_window     (void *window);
void android_set_window_xam (void *window);

bool android_read_asset(const char* asset_name, void** out_data, size_t* out_size);

} // namespace sk
#endif // defined(SK_OS_ANDROID)