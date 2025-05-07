/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#pragma once

#include <openxr/openxr.h>

namespace sk {

void   xr_ext_time_register();
XrTime xr_ext_time_acquire_time();

}