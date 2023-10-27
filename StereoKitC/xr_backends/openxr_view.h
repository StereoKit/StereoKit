/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2023 Nick Klingensmith
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 */

#pragma once

#include "../platforms/platform.h"
#if defined(SK_XR_OPENXR)

#include "../stereokit.h"
#include "../libraries/array.h"

#include <openxr/openxr.h>
#include <stdint.h>

namespace sk {

bool openxr_views_create    ();
void openxr_views_destroy   ();
void openxr_views_update_fov();

void     xr_extension_structs_clear();
bool32_t xr_set_blend              (display_blend_ blend);
bool32_t xr_blend_valid            (display_blend_ blend);
bool32_t xr_view_type_valid        (XrViewConfigurationType type);

} // namespace sk

#endif