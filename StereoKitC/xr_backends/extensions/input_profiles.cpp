/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#include "ext_management.h"
#include "../openxr_input.h"
#include "../../systems/input.h"

#include <string.h>

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

// This implements "hp/mixed_reality_controller" in XR_EXT_hp_mixed_reality_controller
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_EXT_hp_mixed_reality_controller

void xr_profile_ext_hp_mr_controller_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_EXT_HP_MIXED_REALITY_CONTROLLER_EXTENSION_NAME;
	sys.evt_initialize = { [](void*) {
		if (!backend_openxr_ext_enabled(XR_EXT_HP_MIXED_REALITY_CONTROLLER_EXTENSION_NAME))
			return xr_system_fail;

		pose_t palm_offset = device_display_get_blend() == display_blend_opaque
			? pose_t{ {0.01f, -0.01f,  0.015f}, quat_from_angles(-45, 0, 0) }
			: pose_t{ {0,      0.005f, 0     }, quat_from_angles(-68, 0, 0) };

		xr_interaction_profile_t profile_l = { "hp/mixed_reality_controller" };
		profile_l.top_level_path = "/user/hand/left";
		profile_l.is_hand        = false;
		profile_l.palm_offset    = palm_offset;
		profile_l.binding[profile_l.binding_ct++] = { xra_type_pose,  input_pose_l_grip,     "grip/pose"        };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_pose,  input_pose_l_aim,      "aim/pose"         };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_float, input_float_l_trigger, "trigger/value"    };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_float, input_float_l_grip,    "squeeze/value"    };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_xy,    input_xy_l_stick,      "thumbstick"       };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_bool,  input_button_l_stick,  "thumbstick/click" };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_bool,  input_button_l_menu,   "menu/click"       };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_bool,  input_button_l_x1,     "x/click"          };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_bool,  input_button_l_x2,     "y/click"          };
		oxri_register_profile(profile_l);

		xr_interaction_profile_t profile_r = { "hp/mixed_reality_controller" };
		profile_r.top_level_path = "/user/hand/right";
		profile_r.is_hand        = false;
		profile_r.palm_offset    = palm_offset;
		profile_r.binding[profile_r.binding_ct++] = { xra_type_pose,  input_pose_r_grip,     "grip/pose"        };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_pose,  input_pose_r_aim,      "aim/pose"         };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_float, input_float_r_trigger, "trigger/value"    };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_float, input_float_r_grip,    "squeeze/value"    };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_xy,    input_xy_r_stick,      "thumbstick"       };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_bool,  input_button_r_stick,  "thumbstick/click" };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_bool,  input_button_r_menu,   "menu/click"       };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_bool,  input_button_r_x1,     "a/click"          };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_bool,  input_button_r_x2,     "b/click"          };
		oxri_register_profile(profile_r);

		return xr_system_succeed;
	} };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

// This implements "ext/hand_interaction_ext" in XR_EXT_hand_interaction
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_EXT_hand_interaction

void xr_profile_ext_hand_interaction_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_EXT_HAND_INTERACTION_EXTENSION_NAME;
	sys.evt_initialize = { [](void*) {
		if (!backend_openxr_ext_enabled(XR_EXT_HAND_INTERACTION_EXTENSION_NAME))
			return xr_system_fail;

		bool explicit_request = ext_management_is_user_requested(XR_EXT_HAND_INTERACTION_EXTENSION_NAME);

		// SK's hand_interaction implementations use XR_EXT_hand_tracking for
		// some data, so we can't rely on these interaction profiles unless
		// XR_EXT_hand_tracking is available.
		if (explicit_request == false && !backend_openxr_ext_enabled(XR_EXT_HAND_TRACKING_EXTENSION_NAME)) {
			log_diag("XR_EXT_hand_interaction - Disabled - Dependant on XR_EXT_hand_tracking.");
			//xr_ext.EXT_hand_interaction = xr_ext_disabled;
			return xr_system_fail;
		}

		xr_interaction_profile_t profile_l = { "ext/hand_interaction_ext" };
		profile_l.top_level_path = "/user/hand/left";
		profile_l.is_hand        = true;
		profile_l.palm_offset    = pose_identity;
		profile_l.binding[profile_l.binding_ct++] = { xra_type_pose,  input_pose_l_grip,        "grip/pose"           };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_pose,  input_pose_l_aim,         "aim/pose"            };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_bool,  input_button_l_aim_ready, "pinch_ext/ready_ext" };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_float, input_float_l_trigger,    "pinch_ext/value"     };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_float, input_float_l_grip,       "grasp_ext/value"     };
		oxri_register_profile(profile_l);

		xr_interaction_profile_t profile_r = { "ext/hand_interaction_ext" };
		profile_r.top_level_path = "/user/hand/right";
		profile_r.is_hand        = true;
		profile_r.palm_offset    = pose_identity;
		profile_r.binding[profile_r.binding_ct++] = { xra_type_pose,  input_pose_r_grip,        "grip/pose"           };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_pose,  input_pose_r_aim,         "aim/pose"            };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_bool,  input_button_r_aim_ready, "pinch_ext/ready_ext" };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_float, input_float_r_trigger,    "pinch_ext/value"     };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_float, input_float_r_grip,       "grasp_ext/value"     };
		oxri_register_profile(profile_r);

		return xr_system_succeed;
	} };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

// This implements "microsoft/hand_interaction" in XR_MSFT_hand_interaction
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_MSFT_hand_interaction

void xr_profile_msft_hand_interaction_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_MSFT_HAND_INTERACTION_EXTENSION_NAME;
	sys.evt_initialize = { [](void*) {
		if (!backend_openxr_ext_enabled(XR_MSFT_HAND_INTERACTION_EXTENSION_NAME))
			return xr_system_fail;

		bool explicit_request = ext_management_is_user_requested(XR_MSFT_HAND_INTERACTION_EXTENSION_NAME);

		// Quest has a menu button that is always shown when hand tracking, but
		// the hand interaction EXTs don't support actions for it. This can
		// lead to a mismatch where users see the menu button, but SK _can't_
		// react to the button events. hand_interaction EXTs are disabled on
		// Quest so that input falls back to the simple_controller interaction
		// profile. We will only enable it if it's explicitly requested.
		//
		// Quest does not implement XR_EXT_hand_interaction, so we only need to
		// do this for the MSFT one.
		if (explicit_request == false && openxr_get_known_runtime() == xr_runtime_meta) {
			//xr_ext.MSFT_hand_interaction = xr_ext_rejected;
			log_diag("XR_MSFT_hand_interaction - Rejected - Hides menu button events on Quest.");
			return xr_system_fail;
		}

		// SK's hand_interaction implementations use XR_EXT_hand_tracking for
		// some data, so we can't rely on these interaction profiles unless
		// XR_EXT_hand_tracking is available.
		if (explicit_request == false && !backend_openxr_ext_enabled(XR_EXT_HAND_TRACKING_EXTENSION_NAME)) {
			log_diag("XR_MSFT_hand_interaction - Disabled - Dependant on XR_EXT_hand_tracking.");
			//xr_ext.MSFT_hand_interaction = xr_ext_disabled;
			return xr_system_fail;
		}

		xr_interaction_profile_t profile_l = { "microsoft/hand_interaction" };
		profile_l.top_level_path = "/user/hand/left";
		profile_l.is_hand        = true;
		profile_l.palm_offset    = pose_identity;
		profile_l.binding[profile_l.binding_ct++] = { xra_type_pose, input_pose_l_grip,      "grip/pose"     };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_pose, input_pose_l_aim,       "aim/pose"      };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_float, input_float_l_trigger, "select/value"  };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_float, input_float_l_grip,    "squeeze/value" };
		oxri_register_profile(profile_l);

		xr_interaction_profile_t profile_r = { "microsoft/hand_interaction" };
		profile_r.top_level_path = "/user/hand/right";
		profile_r.is_hand        = true;
		profile_r.palm_offset    = pose_identity;
		profile_r.binding[profile_r.binding_ct++] = { xra_type_pose, input_pose_r_grip,      "grip/pose"     };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_pose, input_pose_r_aim,       "aim/pose"      };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_float, input_float_r_trigger, "select/value"  };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_float, input_float_r_grip,    "squeeze/value" };
		oxri_register_profile(profile_r);

		return xr_system_succeed;
	} };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

// This implements "bytedance/pico_neo3_controller" and "bytedance/pico4_controller" in XR_BD_controller_interaction
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_BD_controller_interaction

void xr_profile_bd_controller_interaction_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_BD_CONTROLLER_INTERACTION_EXTENSION_NAME;
	sys.evt_initialize = { [](void*) {
		if (!backend_openxr_ext_enabled(XR_BD_CONTROLLER_INTERACTION_EXTENSION_NAME))
			return xr_system_fail;

		xr_interaction_profile_t profile3_l = { "bytedance/pico_neo3_controller" };
		profile3_l.top_level_path = "/user/hand/left";
		profile3_l.is_hand        = false;
		profile3_l.palm_offset    = pose_t{ {-0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		profile3_l.binding[profile3_l.binding_ct++]={ xra_type_pose,  input_pose_l_grip,      "grip/pose"        };
		profile3_l.binding[profile3_l.binding_ct++]={ xra_type_pose,  input_pose_l_aim,       "aim/pose"         };
		profile3_l.binding[profile3_l.binding_ct++]={ xra_type_float, input_float_l_trigger,  "trigger/value"    };
		profile3_l.binding[profile3_l.binding_ct++]={ xra_type_float, input_float_l_grip,     "squeeze/value"    };
		profile3_l.binding[profile3_l.binding_ct++]={ xra_type_xy,    input_xy_l_stick,       "thumbstick"       };
		profile3_l.binding[profile3_l.binding_ct++]={ xra_type_bool,  input_button_l_stick,   "thumbstick/click" };
		profile3_l.binding[profile3_l.binding_ct++]={ xra_type_bool,  input_button_l_menu,    "menu/click"       };
		profile3_l.binding[profile3_l.binding_ct++]={ xra_type_bool,  input_button_l_x1,      "x/click"          };
		profile3_l.binding[profile3_l.binding_ct++]={ xra_type_bool,  input_button_l_x2,      "y/click"          };
		oxri_register_profile(profile3_l);

		xr_interaction_profile_t profile3_r = { "bytedance/pico_neo3_controller" };
		profile3_r.top_level_path = "/user/hand/right";
		profile3_r.is_hand        = false;
		profile3_r.palm_offset    = pose_t{ { 0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		profile3_r.binding[profile3_r.binding_ct++]={ xra_type_pose,  input_pose_r_grip,      "grip/pose"        };
		profile3_r.binding[profile3_r.binding_ct++]={ xra_type_pose,  input_pose_r_aim,       "aim/pose"         };
		profile3_r.binding[profile3_r.binding_ct++]={ xra_type_float, input_float_r_trigger,  "trigger/value"    };
		profile3_r.binding[profile3_r.binding_ct++]={ xra_type_float, input_float_r_grip,     "squeeze/value"    };
		profile3_r.binding[profile3_r.binding_ct++]={ xra_type_xy,    input_xy_r_stick,       "thumbstick"       };
		profile3_r.binding[profile3_r.binding_ct++]={ xra_type_bool,  input_button_r_stick,   "thumbstick/click" };
		profile3_r.binding[profile3_r.binding_ct++]={ xra_type_bool,  input_button_r_menu,    "menu/click"       };
		profile3_r.binding[profile3_r.binding_ct++]={ xra_type_bool,  input_button_r_x1,      "a/click"          };
		profile3_r.binding[profile3_r.binding_ct++]={ xra_type_bool,  input_button_r_x2,      "b/click"          };
		oxri_register_profile(profile3_r);

		// Note that on the pico 4 OS 5.5 OpenXR SDK 2.2, the xrGetCurrentInteractionProfile will return '/interaction_profiles/bytedance/pico_neo3_controller'
		// instead of the expected '/interaction_profiles/bytedance/pico4_controller'
		xr_interaction_profile_t profile4_l = { "bytedance/pico4_controller" };
		profile4_l.top_level_path = "/user/hand/left";
		profile4_l.is_hand        = false;
		profile4_l.palm_offset    = pose_t{ {-0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		profile4_l.binding[profile4_l.binding_ct++]={ xra_type_pose,  input_pose_l_grip,      "grip/pose"        };
		profile4_l.binding[profile4_l.binding_ct++]={ xra_type_pose,  input_pose_l_aim,       "aim/pose"         };
		profile4_l.binding[profile4_l.binding_ct++]={ xra_type_float, input_float_l_trigger,  "trigger/value"    };
		profile4_l.binding[profile4_l.binding_ct++]={ xra_type_float, input_float_l_grip,     "squeeze/value"    };
		profile4_l.binding[profile4_l.binding_ct++]={ xra_type_xy,    input_xy_l_stick,       "thumbstick"       };
		profile4_l.binding[profile4_l.binding_ct++]={ xra_type_bool,  input_button_l_stick,   "thumbstick/click" };
		profile4_l.binding[profile4_l.binding_ct++]={ xra_type_bool,  input_button_l_menu,    "menu/click"       };
		profile4_l.binding[profile4_l.binding_ct++]={ xra_type_bool,  input_button_l_x1,      "x/click"          };
		profile4_l.binding[profile4_l.binding_ct++]={ xra_type_bool,  input_button_l_x2,      "y/click"          };
		oxri_register_profile(profile4_l);

		xr_interaction_profile_t profile4_r = { "bytedance/pico4_controller" };
		profile4_r.top_level_path = "/user/hand/right";
		profile4_r.is_hand        = false;
		profile4_r.palm_offset    = pose_t{ { 0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		profile4_r.binding[profile4_r.binding_ct++]={ xra_type_pose,  input_pose_r_grip,      "grip/pose"        };
		profile4_r.binding[profile4_r.binding_ct++]={ xra_type_pose,  input_pose_r_aim,       "aim/pose"         };
		profile4_r.binding[profile4_r.binding_ct++]={ xra_type_float, input_float_r_trigger,  "trigger/value"    };
		profile4_r.binding[profile4_r.binding_ct++]={ xra_type_float, input_float_r_grip,     "squeeze/value"    };
		profile4_r.binding[profile4_r.binding_ct++]={ xra_type_xy,    input_xy_r_stick,       "thumbstick"       };
		profile4_r.binding[profile4_r.binding_ct++]={ xra_type_bool,  input_button_r_stick,   "thumbstick/click" };

		profile4_r.binding[profile4_r.binding_ct++]={ xra_type_bool,  input_button_r_x1,      "a/click"          };
		profile4_r.binding[profile4_r.binding_ct++]={ xra_type_bool,  input_button_r_x2,      "b/click"          };
		oxri_register_profile(profile4_r);

		return xr_system_succeed;
	} };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

} // namespace sk