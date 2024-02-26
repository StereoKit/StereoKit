/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2023-2024 Nick Klingensmith
 * Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
 */

#pragma once

#include "../platforms/platform.h"
#if defined(SK_XR_OPENXR)

#include "../stereokit.h"

#include <openxr/openxr.h>
#include <stdint.h>

namespace sk {

bool32_t openxr_fb_entity_init    ();
void     openxr_fb_entity_shutdown();

} // namespace sk

#endif