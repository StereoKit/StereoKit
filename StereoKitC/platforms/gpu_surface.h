// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith

#pragma once
#include "platform.h"

#include <sk_app.h>
#include <sk_renderer.h>

namespace sk {

// The native surface handle is the one thing the windowed backends can't
// express against sk_renderer alone.
bool        gpu_surface_create        (ska_window_t* window, skr_surface_t* out_surface);
skr_vec2i_t gpu_surface_drawable_size (ska_window_t* window);

// The color format the surface presents. Render targets that resolve into it
// have to match exactly, and the choice is the swapchain's, not ours.
tex_format_ gpu_surface_color_format  (const skr_surface_t* surface);

} // namespace sk
