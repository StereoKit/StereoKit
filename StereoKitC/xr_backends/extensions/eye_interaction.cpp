/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

// This implements XR_EXT_eye_gaze_interaction
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_EXT_eye_gaze_interaction

#include "ext_management.h"
#include "eye_interaction.h"
#include "../../systems/input.h"
#include "../../device.h"

#include <string.h>

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

typedef struct xr_ext_eye_gaze_state_t {
	bool available;
} xrext_eye_gaze_state_t;
static xr_ext_eye_gaze_state_t local = { };

///////////////////////////////////////////

void xr_profile_ext_eye_gaze_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_EXT_EYE_GAZE_INTERACTION_EXTENSION_NAME;
	sys.evt_initialize = { [](void*) {
		if (!backend_openxr_ext_enabled(XR_EXT_EYE_GAZE_INTERACTION_EXTENSION_NAME))
			return xr_system_fail;

		// Check if the instance supports eye gaze
		XrSystemProperties                      properties      = { XR_TYPE_SYSTEM_PROPERTIES };
		XrSystemEyeGazeInteractionPropertiesEXT properties_gaze = { XR_TYPE_SYSTEM_EYE_GAZE_INTERACTION_PROPERTIES_EXT };
		properties.next = &properties_gaze;
		XrResult xr = xrGetSystemProperties(xr_instance, xr_system_id, &properties);
		if (XR_FAILED(xr)) {
			log_infof("%s [%s]", "xrGetSystemProperties", openxr_string(xr));
			return xr_system_fail;
		}
		if (properties_gaze.supportsEyeGazeInteraction == false)
			return xr_system_fail;

		// ext/eye_gaze_interaction
		// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_EXT_eye_gaze_interaction
		xr_interaction_profile_t profile = { "ext/eye_gaze_interaction" };
		profile.top_level_path = "/user/eyes_ext";
		profile.palm_offset    = pose_identity;
		profile.is_hand        = false;
		profile.binding[profile.binding_ct++] = { xra_type_pose, input_pose_eyes, "gaze_ext/pose" };
		oxri_register_profile(profile);

		local.available = true;
		device_data.has_eye_gaze = true;

		return xr_system_succeed;
	} };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

bool xr_ext_eye_gaze_available() {
	return local.available;
}

} // namespace sk