/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 */

#pragma once

#include "../../stereokit.h"
#include "../openxr.h"

namespace sk {

void xr_ext_meta_environment_depth_register();
void xr_ext_meta_environment_depth_update_frame(XrTime display_time);

bool               xr_ext_meta_environment_depth_available();
bool               xr_ext_meta_environment_depth_running();
sensor_depth_caps_ xr_ext_meta_environment_depth_get_capabilities();
bool               xr_ext_meta_environment_depth_start(sensor_depth_caps_ flags);
void               xr_ext_meta_environment_depth_stop();
bool               xr_ext_meta_environment_depth_set_caps(sensor_depth_caps_ flags);
tex_t              xr_ext_meta_environment_depth_get_texture();
bool               xr_ext_meta_environment_depth_try_get_latest(sensor_depth_frame_t* out_info);

}