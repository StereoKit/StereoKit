// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

#pragma once

#include "../../platforms/platform.h"
#include "../../stereokit.h"

namespace sk {

#if defined(SK_XR_OPENXR)

void    xr_ext_interaction_render_model_register();
void    xr_ext_interaction_render_model_draw_controller(handed_ hand);
model_t xr_ext_interaction_render_model_get            (handed_ hand);
void    xr_ext_interaction_render_model_draw_others    ();
bool    xr_ext_interaction_render_model_available      ();

#else

// Stubs, so callers can stay readable inline rather than #if at each use
inline void    xr_ext_interaction_render_model_draw_controller(handed_) {}
inline model_t xr_ext_interaction_render_model_get            (handed_) { return nullptr; }
inline void    xr_ext_interaction_render_model_draw_others    ()        {}
inline bool    xr_ext_interaction_render_model_available      ()        { return false; }

#endif

}