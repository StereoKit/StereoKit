/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#pragma once

#include "../../hands/input_hand.h"
#include "../../stereokit.h"
#include <openxr/openxr.h>

namespace sk {

void             xr_ext_hand_tracking_register   (void);
bool             xr_ext_hand_tracking_available  (void);
XrHandTrackerEXT xr_ext_hand_tracking_get_tracker(handed_ hand);

}