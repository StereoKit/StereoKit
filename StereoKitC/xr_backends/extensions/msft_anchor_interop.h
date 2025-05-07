/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#pragma once

namespace sk {

void xr_ext_msft_anchor_interop_register();
bool xr_ext_msft_anchor_interop_try_get_perception_anchor(XrSpatialAnchorMSFT anchor, void** out_perception_spatial_anchor);

}