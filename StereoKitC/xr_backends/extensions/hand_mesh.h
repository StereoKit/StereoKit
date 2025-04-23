/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#pragma once

#include "../../hands/input_hand.h"

namespace sk {

void xr_ext_msft_hand_mesh_register   ();
bool xr_ext_msft_hand_mesh_available  ();
void xr_ext_msft_hand_mesh_update_mesh(handed_ handed, hand_mesh_t* ref_hand_mesh);

}