// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

#pragma once

namespace sk {

bool offscreen_init      ();
void offscreen_step_begin();
void offscreen_step_end  ();
void offscreen_shutdown  ();

}