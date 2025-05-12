/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#pragma once

#include <openxr/openxr.h>

namespace sk {

void xr_ext_future_register ();
void xr_ext_future_on_finish(XrFutureEXT future, void(*on_finish)(void* context, XrFutureEXT future), void* context);
bool xr_ext_future_check    (XrFutureEXT future);

}