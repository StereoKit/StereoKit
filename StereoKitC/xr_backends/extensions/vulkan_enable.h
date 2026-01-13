/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#pragma once

#include <sk_renderer.h>

namespace sk {

void  xr_ext_vulkan_enable_register();

// Configure sk_renderer for OpenXR Vulkan graphics binding (call after openxr_create_system)
bool  xr_ext_vulkan_enable_setup_skr(skr_settings_t* ref_settings, const char*** out_instance_exts, uint32_t* out_instance_ext_count);

}