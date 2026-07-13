/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 * Copyright (c) 2026 Qualcomm Technologies, Inc.
 */

// This implements XR_META_detached_controllers 
// https://registry.khronos.org/OpenXR/specs/1.1/html/xrspec.html#XR_META_detached_controllers
// to use with
// https://registry.khronos.org/OpenXR/specs/1.1/man/html/XR_META_simultaneous_hands_and_controllers.html
#include "ext_management.h"
#include "meta_detached_controllers.h"
#include "../openxr_input.h"
#include "../../systems/input.h"

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

typedef struct xr_ext_meta_detached_controllers_state_t {
	bool available;
} xr_ext_meta_detached_controllers_state_t;
static xr_ext_meta_detached_controllers_state_t local = {};

///////////////////////////////////////////

void xr_profile_meta_detached_controllers_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_META_DETACHED_CONTROLLERS_EXTENSION_NAME;
	sys.evt_initialize = { [](void*) {
		if (!backend_openxr_ext_enabled(XR_META_DETACHED_CONTROLLERS_EXTENSION_NAME))
			return xr_system_fail;

		// oculus/touch_controller 
		// https://registry.khronos.org/OpenXR/specs/1.1/html/xrspec.html#XR_META_detached_controllers
		{
			xr_interaction_profile_t profile_l = { "oculus/touch_controller" };
			profile_l.top_level_path = "/user/detached_controller_meta/left";
			profile_l.palm_offset    = pose_identity;
			profile_l.is_hand        = true;
			profile_l.binding[profile_l.binding_ct++] = { xra_type_pose, input_pose_l_detached, "grip/pose" };
			oxri_register_profile(profile_l);

			xr_interaction_profile_t profile_r = { "oculus/touch_controller" };
			profile_r.top_level_path = "/user/detached_controller_meta/right";
			profile_r.palm_offset    = pose_identity;
			profile_r.is_hand        = true;
			profile_r.binding[profile_r.binding_ct++] = { xra_type_pose, input_pose_r_detached, "grip/pose" };
			oxri_register_profile(profile_r);
		}

		local.available = true;

		return xr_system_succeed;
	} };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

bool xr_meta_detached_controllers_available() {
	return local.available;
}

} // namespace sk
