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

bool xr_ext_meta_environment_depth_available();
bool xr_ext_meta_environment_depth_running();
bool xr_ext_meta_environment_depth_supports_hand_removal();
bool xr_ext_meta_environment_depth_start();
void xr_ext_meta_environment_depth_stop();
bool xr_ext_meta_environment_depth_set_hand_removal(bool32_t enabled);
bool xr_ext_meta_environment_depth_try_get_latest(environment_depth_frame_t* out_frame);

}