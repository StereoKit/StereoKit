/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

// XR_KHR_composition_layer_depth spec here:
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_KHR_composition_layer_depth

// NOTE! openxr_view.cpp has part of the implementation for this extension!!!

#include "ext_management.h"
#include "msft_observer.h"
#include "../openxr_view.h"

///////////////////////////////////////////

namespace sk {

typedef struct xr_composition_depth_state_t {
	bool available;
} xr_composition_depth_state_t;
static xr_composition_depth_state_t local = { };

///////////////////////////////////////////

xr_system_ xr_ext_composition_depth_init(void*);

///////////////////////////////////////////

void xr_ext_composition_depth_register() {
	local = {};

	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME;
	sys.evt_initialize = { xr_ext_composition_depth_init };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_composition_depth_init(void*) {
	// Check if we got our extension
	if (!backend_openxr_ext_enabled(XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME))
		return xr_system_fail;

	local.available = true;
	return xr_system_succeed;
}

///////////////////////////////////////////

bool xr_ext_composition_depth_available() {
	return local.available;
}

///////////////////////////////////////////

} // namespace sk
