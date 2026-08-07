/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Austin Hale
 */

#pragma once

#include "../../stereokit.h"
#include "../openxr.h"

namespace sk {

void xr_ext_android_depth_texture_register();
void xr_ext_android_depth_texture_update_frame(XrTime display_time);

bool               xr_ext_android_depth_texture_available();
bool               xr_ext_android_depth_texture_running();
sensor_depth_caps_ xr_ext_android_depth_texture_get_capabilities();
bool               xr_ext_android_depth_texture_start(sensor_depth_caps_ flags);
void               xr_ext_android_depth_texture_stop();
bool               xr_ext_android_depth_texture_set_caps(sensor_depth_caps_ flags);
tex_t              xr_ext_android_depth_texture_get_texture();
bool               xr_ext_android_depth_texture_try_get_latest(sensor_depth_frame_t* out_info);
bool               xr_ext_android_depth_texture_try_get_image(sensor_depth_image_ image, sensor_depth_frame_t* out_frame, void* out_data, size_t* out_data_size, int32_t view_index);
void               xr_ext_android_depth_texture_get_resolutions(const sensor_depth_resolution_t** out_arr_resolutions, int32_t* out_count);
void               xr_ext_android_depth_texture_set_resolution(sensor_depth_resolution_t resolution);

}
