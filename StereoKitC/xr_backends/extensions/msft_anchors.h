/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#pragma once

#include "../../platforms/platform.h"
#include "../../stereokit.h"

namespace sk {

#if defined(SK_XR_OPENXR)

void         xr_ext_msft_spatial_anchors_register    ();
bool         xr_ext_msft_spatial_anchors_available   ();

anchor_t     xr_ext_msft_spatial_anchors_create      (pose_t pose, const char* name_utf8);
void         xr_ext_msft_spatial_anchors_destroy     (anchor_t anchor);
void         xr_ext_msft_spatial_anchors_clear_stored();
bool32_t     xr_ext_msft_spatial_anchors_persist     (anchor_t anchor, bool32_t persist);
anchor_caps_ xr_ext_msft_spatial_anchors_capabilities();

#else

// Stubs, so callers can stay readable inline rather than #if at each use
inline bool         xr_ext_msft_spatial_anchors_available   ()                    { return false; }
inline anchor_t     xr_ext_msft_spatial_anchors_create      (pose_t, const char*) { return nullptr; }
inline void         xr_ext_msft_spatial_anchors_destroy     (anchor_t)            {}
inline void         xr_ext_msft_spatial_anchors_clear_stored()                    {}
inline bool32_t     xr_ext_msft_spatial_anchors_persist     (anchor_t, bool32_t)  { return false; }
inline anchor_caps_ xr_ext_msft_spatial_anchors_capabilities()                    { return (anchor_caps_)0; }

#endif

}
