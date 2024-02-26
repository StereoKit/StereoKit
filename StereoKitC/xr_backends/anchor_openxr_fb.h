/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

#pragma once

#include "../platforms/platform.h"
#if defined(SK_XR_OPENXR)

#include "../stereokit.h"
#include "../libraries/array.h"

#include <openxr/openxr.h>
#include <stdint.h>

namespace sk {

bool32_t     anchor_oxr_fb_init        ();
void         anchor_oxr_fb_shutdown    ();
void         anchor_oxr_fb_step        ();
anchor_t     anchor_oxr_fb_create      (pose_t pose, const char* name_utf8);
void         anchor_oxr_fb_destroy     (anchor_t anchor);
void         anchor_oxr_fb_clear_stored();
bool32_t     anchor_oxr_fb_persist     (anchor_t anchor, bool32_t persist);
anchor_caps_ anchor_oxr_fb_capabilities();

} // namespace sk

#endif