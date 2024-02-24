// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

#pragma once

namespace sk {

bool xr_init      ();
void xr_step_begin();
void xr_step_end  ();
void xr_shutdown  ();

}