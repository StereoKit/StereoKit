/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

// This implements XR_MSFT_perception_anchor_interop and depends on XR_MSFT_spatial_anchor
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_MSFT_perception_anchor_interop
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_MSFT_spatial_anchor

#include "../openxr_platform.h"
#include "ext_management.h"
#include "msft_bridge.h"

#if defined(SK_OS_WINDOWS_UWP)

#include "../../_stereokit.h"
#include <string.h>

///////////////////////////////////////////

#define XR_EXT_FUNCTIONS( X )                        \
	X(xrCreateSpatialAnchorFromPerceptionAnchorMSFT) \
	X(xrTryGetPerceptionAnchorFromSpatialAnchorMSFT) \
	X(xrCreateSpatialAnchorSpaceMSFT               ) \
	X(xrDestroySpatialAnchorMSFT                   )
OPENXR_DEFINE_FN_STATIC(XR_EXT_FUNCTIONS);

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

xr_system_ xr_ext_msft_anchor_interop_init    (void*);
void       xr_ext_msft_anchor_interop_shutdown(void*);

///////////////////////////////////////////

void xr_ext_msft_anchor_interop_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_MSFT_PERCEPTION_ANCHOR_INTEROP_EXTENSION_NAME;
	sys.request_exts[sys.request_ext_count++] = XR_MSFT_SPATIAL_ANCHOR_EXTENSION_NAME;
	sys.evt_initialize = { xr_ext_msft_anchor_interop_init };
	sys.evt_shutdown   = { xr_ext_msft_anchor_interop_shutdown };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_msft_anchor_interop_init(void*) {
	// Check if we got our extensions
	if (!backend_openxr_ext_enabled(XR_MSFT_PERCEPTION_ANCHOR_INTEROP_EXTENSION_NAME) ||
		!backend_openxr_ext_enabled(XR_MSFT_SPATIAL_ANCHOR_EXTENSION_NAME))
		return xr_system_fail;

	// Load all extension functions
	OPENXR_LOAD_FN_RETURN(XR_EXT_FUNCTIONS, xr_system_fail);

	sk_get_info_ref()->perception_bridge_present = true;
	return xr_system_succeed;
}

///////////////////////////////////////////

xr_system_ xr_ext_msft_anchor_interop_shutdown(void*) {
	OPENXR_CLEAR_FN(XR_EXT_FUNCTIONS);
}

///////////////////////////////////////////

pose_t world_from_perception_anchor(void* perception_spatial_anchor) {
	if (!xr_session) {
		log_warn("No OpenXR session available for converting perception anchors!");
		return pose_identity;
	}
	if (!sk_get_info_ref()->perception_bridge_present) {
		log_warn("This system doesn't support the perception bridge! Check SK.System.perceptionBridgePresent.");
		return pose_identity;
	}

	pose_t result;
	if (!world_try_from_perception_anchor(perception_spatial_anchor, &result))
		log_warn("world_from_perception_anchor call failed, maybe a bad spatial anchor?");
	return result;
}

///////////////////////////////////////////

bool32_t world_try_from_perception_anchor(void* perception_spatial_anchor, pose_t* out_pose) {
	*out_pose = pose_identity;
	if (!xr_session || !sk_get_info_ref()->perception_bridge_present)
		return false;

	// Create an anchor from what the user gave us
	XrSpatialAnchorMSFT anchor = {};
	xrCreateSpatialAnchorFromPerceptionAnchorMSFT(xr_session, (IUnknown*)perception_spatial_anchor, &anchor);

	// Create a Space from the anchor
	XrSpace                            space;
	XrSpatialAnchorSpaceCreateInfoMSFT info = { XR_TYPE_SPATIAL_ANCHOR_SPACE_CREATE_INFO_MSFT };
	info.anchor            = anchor;
	info.poseInAnchorSpace = { {0,0,0,1}, {0,0,0} };
	if (XR_FAILED(xrCreateSpatialAnchorSpaceMSFT(xr_session, &info, &space)))
		return false;

	// Convert the space into a pose
	if (!openxr_get_space(space, out_pose)) {
		xrDestroySpatialAnchorMSFT(anchor);
		return false;
	}

	// Release the anchor, and return the resulting pose!
	xrDestroySpatialAnchorMSFT(anchor);
	return true;
}

///////////////////////////////////////////

bool xr_ext_msft_anchor_interop_try_get_perception_anchor(XrSpatialAnchorMSFT anchor, void** ref_perception_spatial_anchor) {
	if (xrTryGetPerceptionAnchorFromSpatialAnchorMSFT)
		return false;

	XrResult result = xrTryGetPerceptionAnchorFromSpatialAnchorMSFT(xr_session, anchor, (IUnknown**)ref_perception_spatial_anchor);
	if (XR_FAILED(result)) {
		log_warnf("xrTryGetPerceptionAnchorFromSpatialAnchorMSFT failed: %s", openxr_string(result));
		return false;
	}
	return true;
}

///////////////////////////////////////////

} // namespace sk

#else

namespace sk {

///////////////////////////////////////////

void xr_ext_msft_anchor_interop_register() {}

///////////////////////////////////////////

pose_t world_from_perception_anchor(void* perception_spatial_anchor) {
	log_warn("world_from_perception_anchor call failed, invalid platform.");
	return pose_identity;
}

///////////////////////////////////////////

bool32_t world_try_from_perception_anchor(void* perception_spatial_anchor, pose_t* out_pose) {
	*out_pose = pose_identity;
	return false;

}

///////////////////////////////////////////

bool xr_ext_msft_anchor_interop_try_get_perception_anchor(XrSpatialAnchorMSFT anchor, void** ref_perception_spatial_anchor) {
	return false;
}

///////////////////////////////////////////

} // namespace sk

#endif