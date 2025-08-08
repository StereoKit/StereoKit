/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

// This implements XR_EXT_palm_pose
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_EXT_palm_pose

#include "palm_pose.h"
#include "ext_management.h"
#include "../../libraries/stref.h"
#include "../../systems/input.h"

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

xr_system_ xr_ext_palm_pose_initialize(void*);
void       xr_ext_palm_pose_on_profile(void*, xr_interaction_profile_t* ref_profile);

///////////////////////////////////////////

void xr_ext_palm_pose_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_KHR_MAINTENANCE1_EXTENSION_NAME;
	sys.request_exts[sys.request_ext_count++] = XR_EXT_PALM_POSE_EXTENSION_NAME;
	sys.evt_initialize  = { xr_ext_palm_pose_initialize };
	sys.evt_profile     = { xr_ext_palm_pose_on_profile };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_palm_pose_initialize(void*) {
	// Check if we got at least one of our extensions
	if (!backend_openxr_ext_enabled(XR_KHR_MAINTENANCE1_EXTENSION_NAME) && !backend_openxr_ext_enabled(XR_EXT_PALM_POSE_EXTENSION_NAME))
		return xr_system_fail;

	// Snapdragon Spaces advertises the palm pose extension, but provides bad
	// data for it. Only enable it if it's explicitly requested.
	if (strstr(device_get_runtime(), "Snapdragon") != nullptr && ext_management_is_user_requested("XR_EXT_palm_pose") == false) {
		log_diag("XR_EXT_palm_pose - Rejected - Not fully implemented on Snapdragon Spaces.");
		return xr_system_fail;
	}

	return xr_system_succeed;
}

///////////////////////////////////////////

void xr_ext_palm_pose_on_profile(void*, xr_interaction_profile_t* ref_profile) {
	// This function will only get called if the system was successfully
	// initialized.

	if (backend_openxr_ext_enabled(XR_KHR_MAINTENANCE1_EXTENSION_NAME)) {
		// KHR_maintenance1 applies to all interaction profiles that use /user/hand/*
		// paths, so we need to check if the top level path matches.
		if (string_eq("/user/hand/left",  ref_profile->top_level_path)) ref_profile->binding[ref_profile->binding_ct++] = { xra_type_pose, input_pose_l_palm, "grip_surface/pose" };
		if (string_eq("/user/hand/right", ref_profile->top_level_path)) ref_profile->binding[ref_profile->binding_ct++] = { xra_type_pose, input_pose_r_palm, "grip_surface/pose" };
	} else if (backend_openxr_ext_enabled(XR_EXT_PALM_POSE_EXTENSION_NAME)) {
		// EXT_palm_pose applies to all interaction profiles that use /user/hand/*
		// paths, so we need to check if the top level path matches.
		if (string_eq("/user/hand/left",  ref_profile->top_level_path)) ref_profile->binding[ref_profile->binding_ct++] = { xra_type_pose, input_pose_l_palm, "palm_ext/pose" };
		if (string_eq("/user/hand/right", ref_profile->top_level_path)) ref_profile->binding[ref_profile->binding_ct++] = { xra_type_pose, input_pose_r_palm, "palm_ext/pose" };
	}

}

} // namespace sk