/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#pragma once

namespace sk {

// OpenXR provider for the spatial entity system (XR_EXT_spatial_entity
// and friends). Feeds discovered entities into the registry in
// systems/spatial_entity.h, which owns the public-facing surface.
void xr_ext_spatial_entity_register();

}
