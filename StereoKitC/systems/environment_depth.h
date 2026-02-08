/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 */

#pragma once

#include "../stereokit.h"

namespace sk {

typedef enum environment_depth_system_ {
	environment_depth_system_none,
	environment_depth_system_openxr_meta,
} environment_depth_system_;

void environment_depth_register();

}