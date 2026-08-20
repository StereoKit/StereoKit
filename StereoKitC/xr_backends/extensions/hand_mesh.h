/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#pragma once

#include "../../platforms/platform.h"
#include "../../hands/input_hand.h"

namespace sk {

#if defined(SK_XR_OPENXR)

void xr_ext_msft_hand_mesh_register   ();
bool xr_ext_msft_hand_mesh_available  ();
void xr_ext_msft_hand_mesh_update_mesh(handed_ handed, hand_mesh_t* ref_hand_mesh);

#else

// Stubs, so callers can stay readable inline rather than #if at each use
inline bool xr_ext_msft_hand_mesh_available  ()                      { return false; }
inline void xr_ext_msft_hand_mesh_update_mesh(handed_, hand_mesh_t*) {}

#endif

}