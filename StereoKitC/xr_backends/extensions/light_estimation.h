/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 * Copyright (c) 2026 Qualcomm Technologies, Inc.
 */

#pragma once

#include "../../stereokit.h"

namespace sk {

void xr_ext_android_light_estimation_register (void);
bool xr_ext_android_light_estimation_available(void);

bool xr_ext_light_estimation_start            (void);
void xr_ext_light_estimation_stop             (void);
bool xr_ext_light_estimation_update_sh        (spherical_harmonics_t* ref_sh);
bool xr_ext_light_estimation_update_reflection(tex_t ref_cubemap);

}