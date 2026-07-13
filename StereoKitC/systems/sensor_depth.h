/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 */

#pragma once

#include "../stereokit.h"

namespace sk {

typedef enum sensor_depth_system_ {
	sensor_depth_system_none,
	sensor_depth_system_openxr_meta,
} sensor_depth_system_;

void sensor_depth_register();

}