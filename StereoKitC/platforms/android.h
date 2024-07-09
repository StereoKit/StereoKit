/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2023 Nick Klingensmith
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 */

#pragma once
#include "_platform.h"

#if defined(SK_OS_ANDROID)

namespace sk {

void android_set_window     (void *window);
void android_set_window_xam (void *window);

bool android_check_manifest_permission(const char* permission);
bool android_check_app_permission     (const char* permission);
void android_request_permission       (const char* permission);

bool android_read_asset(const char* asset_name, void** out_data, size_t* out_size);

} // namespace sk
#endif // defined(SK_OS_ANDROID)