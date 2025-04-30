/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

// This implements XR_EXTX_overlay
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_EXTX_overlay

#include "../../_stereokit.h"
#include "ext_management.h"
#include "overlay.h"
#include "../../device.h"

///////////////////////////////////////////

typedef struct overlay_state_t {
	XrSessionCreateInfoOverlayEXTX overlay_info;
} overlay_state_t;
static overlay_state_t local = { };

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

xr_system_ xr_ext_overlay_pre_session(void*, XrBaseHeader* session_create_info);

///////////////////////////////////////////

void xr_ext_overlay_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_EXTX_OVERLAY_EXTENSION_NAME;
	sys.evt_pre_session = { xr_ext_overlay_pre_session };

	if (sk_get_settings_ref()->overlay_app)
		ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_overlay_pre_session(void*, XrBaseHeader* session_create_info) {
	if (!backend_openxr_ext_enabled(XR_EXTX_OVERLAY_EXTENSION_NAME)) {
		log_warn("Overlay was requested, but is not available on this system!");
		return xr_system_fail;
	}

	// overlay_info must be declared in a long lasting scope! The memory must
	// remain alive at least until the session is created!
	local.overlay_info = { XR_TYPE_SESSION_CREATE_INFO_OVERLAY_EXTX };
	local.overlay_info.sessionLayersPlacement = sk_get_settings_ref()->overlay_priority;

	// Insert our overlay info into the the session creation "next" chain
	xr_insert_next(session_create_info, (XrBaseHeader*)&local.overlay_info);

	// If this is an overlay app and the user has not explicitly requested a blend
	// mode, then we'll auto-switch to 'blend', as that's likely the most
	// appropriate mode for the app.
	log_diag("Starting as an overlay app.");
	if (sk_get_settings_ref()->blend_preference == display_blend_none) {
		log_diag("Overlay app defaulting to 'blend' display_blend.");
		device_data.display_blend = display_blend_blend;
	}
	return xr_system_succeed;
}

} // namespace sk