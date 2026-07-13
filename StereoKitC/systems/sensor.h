/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 */

#pragma once

#include "../stereokit.h"

namespace sk {

void sensor_register();
void sensor_readback_request();
void sensor_readback_update (tex_t gpu_tex, int32_t width, int32_t height, int32_t array_count, const void* frame_meta, size_t frame_meta_size, uint64_t frame_id);
bool sensor_readback_get    (void* out_frame_meta, size_t frame_meta_size, void* out_data, size_t* out_data_size, int32_t array_count, int32_t view_index);
void sensor_readback_cleanup();

}