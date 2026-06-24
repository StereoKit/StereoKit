/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 */

#pragma once

#include "../../stereokit.h"

namespace sk {

void         xr_ext_android_spatial_anchors_register    ();
bool         xr_ext_android_spatial_anchors_available   ();

anchor_t     xr_ext_android_spatial_anchors_create      (pose_t pose, const char* name_utf8);
void         xr_ext_android_spatial_anchors_destroy     (anchor_t anchor);
void         xr_ext_android_spatial_anchors_clear_stored();
bool32_t     xr_ext_android_spatial_anchors_persist     (anchor_t anchor, bool32_t persist);
anchor_caps_ xr_ext_android_spatial_anchors_capabilities();

}
