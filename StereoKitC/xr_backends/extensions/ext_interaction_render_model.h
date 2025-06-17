// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

#pragma once

#include "../../stereokit.h"

namespace sk {

void xr_ext_interaction_render_model_register();
void xr_ext_interaction_render_model_draw_controller(handed_ hand);
void xr_ext_interaction_render_model_draw_others    ();
bool xr_ext_interaction_render_model_available      ();

}