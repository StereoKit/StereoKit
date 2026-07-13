// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith
// Copyright (c) 2026 Qualcomm Technologies, Inc.

#pragma once

#include "../../stereokit.h"

namespace sk {

void    xr_fb_render_model_register       ();
void    xr_fb_render_model_draw_controller(handed_ hand);
model_t xr_fb_render_model_get            (handed_ hand);
bool    xr_fb_render_model_available      ();

}
