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

bool xr_ext_light_estimation_get_split        (spherical_harmonics_t* out_ambient, sh_light_t* out_light);

// True when the system can provide cubemap light estimates at all. Starting
// them also needs permission_type_reflection_estimation.
bool xr_ext_light_estimation_cubemap_available(void);

// Format and face resolution that started cubemap estimates arrive in, so the
// caller can create a matching cubemap for the update below.
bool xr_ext_light_estimation_reflection_info  (tex_format_* out_format, int32_t* out_face_size);
// Requests a fresh cubemap estimate from the runtime and uploads it into
// ref_cubemap, sized by xr_ext_light_estimation_reflection_info. This is the
// expensive part of an estimate for the runtime to service, so call it only
// when the lighting has changed enough to warrant a reflection rebuild.
bool xr_ext_light_estimation_fetch_reflection (tex_t ref_cubemap);

}
