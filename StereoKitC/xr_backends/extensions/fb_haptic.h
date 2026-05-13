/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 * Copyright (c) 2026 Qualcomm Technologies, Inc.
 */

#pragma once

#include "../../stereokit.h"

#include <openxr/openxr.h>

namespace sk {

void xr_fb_haptic_register          ();
bool xr_fb_haptic_pcm_available     ();
bool xr_fb_haptic_envelope_available();

// Wraps xrGetDeviceSampleRateFB. Returns 0 if XR_FB_haptic_pcm isn't enabled
// or the runtime declines to provide a rate.
float xr_fb_haptic_pcm_get_sample_rate(const XrHapticActionInfo* info);

}
