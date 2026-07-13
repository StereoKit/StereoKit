// SPDX-License-Identifier: MIT */
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith
// Copyright (c) 2026 Qualcomm Technologies, Inc.

#pragma once

#include <openxr/openxr.h>

namespace sk {

void xr_ext_view_config_views_change_register();
bool xr_ext_view_config_views_change_consume (XrViewConfigurationType type);

}
