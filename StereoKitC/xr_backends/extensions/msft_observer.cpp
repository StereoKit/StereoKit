/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

// This implements XR_MSFT_first_person_observer and XR_MSFT_secondary_view_configuration
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_MSFT_first_person_observer
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_MSFT_secondary_view_configuration

// NOTE! openxr_view.cpp has part of the implementation for this extension!!!

#include "ext_management.h"
#include "msft_observer.h"
#include "../openxr_view.h"

///////////////////////////////////////////

namespace sk {

typedef struct xr_observer_state_t {
	bool available;
	XrSecondaryViewConfigurationSessionBeginInfoMSFT secondary;
	XrViewConfigurationType                          secondary_type;
} xr_observer_state_t;
static xr_observer_state_t local = { };

///////////////////////////////////////////

xr_system_ xr_ext_msft_observer_init         (void*);
xr_system_ xr_ext_msft_observer_begin_session(void*, XrBaseHeader* ref_begin_info);

///////////////////////////////////////////

void xr_ext_msft_observer_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_MSFT_FIRST_PERSON_OBSERVER_EXTENSION_NAME;
	sys.request_exts[sys.request_ext_count++] = XR_MSFT_SECONDARY_VIEW_CONFIGURATION_EXTENSION_NAME;
	sys.evt_initialize    = { xr_ext_msft_observer_init };
	sys.evt_begin_session = { xr_ext_msft_observer_begin_session };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_msft_observer_init(void*) {
	// Check if we got our extension
	if (!backend_openxr_ext_enabled(XR_MSFT_SPATIAL_GRAPH_BRIDGE_EXTENSION_NAME))
		return xr_system_fail;

	local.available = true;
	return xr_system_succeed;
}

///////////////////////////////////////////

xr_system_ xr_ext_msft_observer_begin_session(void*, XrBaseHeader* ref_begin_info) {
	if (!local.available) return xr_system_succeed;

	// If the XR_MSFT_first_person_observer extension is present,
	// we may have a secondary display we need to enable. This is
	// typically the HoloLens video recording or streaming feature.
	local.secondary      = { XR_TYPE_SECONDARY_VIEW_CONFIGURATION_SESSION_BEGIN_INFO_MSFT };
	local.secondary_type = XR_VIEW_CONFIGURATION_TYPE_SECONDARY_MONO_FIRST_PERSON_OBSERVER_MSFT;
	if (xr_view_type_valid(local.secondary_type)) {
		local.secondary.viewConfigurationCount = 1;
		local.secondary.enabledViewConfigurationTypes = &local.secondary_type;
		xr_insert_next(ref_begin_info, (XrBaseHeader*)&local.secondary);
	}
	return xr_system_succeed;
}

///////////////////////////////////////////

} // namespace sk
