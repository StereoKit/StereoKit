// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

#pragma once

namespace sk {

bool window_init();
void window_step_begin();
void window_step_end();
void window_shutdown();

}