// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

#pragma once
#include "platform.h"
#include <sk_renderer.h>

namespace sk {

bool platform_impl_init    ();
void platform_impl_shutdown();
void platform_impl_step    ();

skr_surface_t* platform_win_get_surface(platform_win_t window);

///////////////////////////////////////////

} // namespace sk