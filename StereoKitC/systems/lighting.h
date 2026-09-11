// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

#pragma once

#include "../stereokit.h"

namespace sk {

// Global shader register for the specular reflection cubemap, the GGX
// convolved chain sk_cubemap/sk_cubemap_i refer to. The sky is not a global.
const int32_t       render_reflection_register = 11;
const render_layer_ render_sky_layer           = render_layer_vfx;

// Max face resolution for reflections the lighting system generates.
#define SK_LIGHTING_REFLECTION_SIZE 64

bool lighting_init         ();
void lighting_step         ();
void lighting_shutdown     ();
void lighting_check_pending();

// For render.cpp to get lighting data for the global shader buffer
const vec4* lighting_get_lighting();
// The main light as 2 vec4s: [0].xyz direction toward it, [1].rgb linear
// color, [1].w 1 when a light exists. All zero when there is none.
const vec4* lighting_get_main_light_fast();

} // namespace sk
