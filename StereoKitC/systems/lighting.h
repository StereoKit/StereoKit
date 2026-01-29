// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

#pragma once

#include "../stereokit.h"

namespace sk {

const int32_t render_skytex_register = 11;

bool lighting_init         ();
void lighting_step         ();
void lighting_shutdown     ();
void lighting_check_pending();

// For render.cpp to get lighting data for the global shader buffer
const vec4* lighting_get_lighting();

} // namespace sk
