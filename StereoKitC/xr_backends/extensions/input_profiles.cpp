/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#include "ext_management.h"
#include "../openxr_input.h"

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

void xr_profile_ext_hp_mr_controller_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_EXT_HP_MIXED_REALITY_CONTROLLER_EXTENSION_NAME;
	sys.func_initialize = { [](void*) {
		if (!backend_openxr_ext_enabled(XR_EXT_HP_MIXED_REALITY_CONTROLLER_EXTENSION_NAME))
			return xr_system_fail;

		// hp/mixed_reality_controller
		// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_EXT_hp_mixed_reality_controller
		pose_t palm_offset = device_display_get_blend() == display_blend_opaque
			? pose_t{ {0.01f, -0.01f,  0.015f}, quat_from_angles(-45, 0, 0) }
			: pose_t{ {0,      0.005f, 0     }, quat_from_angles(-68, 0, 0) };

		xr_interaction_profile_t profile = { "hp/mixed_reality_controller" };
		profile.use_shorthand_names       = true;
		profile.is_hand                   = false;
		profile.palm_offset[handed_left ] = palm_offset;
		profile.palm_offset[handed_right] = palm_offset;
		profile.binding[profile.binding_ct++] = { xr_action_pose_grip,      "grip/pose",           "grip/pose"           };
		profile.binding[profile.binding_ct++] = { xr_action_pose_aim,       "aim/pose",            "aim/pose"            };
		profile.binding[profile.binding_ct++] = { xr_action_float_trigger,  "trigger/value",       "trigger/value"       };
		profile.binding[profile.binding_ct++] = { xr_action_float_grip,     "squeeze/value",       "squeeze/value"       };
		profile.binding[profile.binding_ct++] = { xr_action_xy_stick,       "thumbstick",          "thumbstick"          };
		profile.binding[profile.binding_ct++] = { xr_action_bool_stick,     "thumbstick/click",    "thumbstick/click"    };
		profile.binding[profile.binding_ct++] = { xr_action_bool_menu,      "menu/click",          "menu/click"          };
		profile.binding[profile.binding_ct++] = { xr_action_bool_x1,        "x/click",             "a/click"             };
		profile.binding[profile.binding_ct++] = { xr_action_bool_x2,        "y/click",             "b/click"             };
		oxri_register_profile(profile);

		return xr_system_succeed;
	} };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

void xr_profile_ext_hand_interaction_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_EXT_HAND_INTERACTION_EXTENSION_NAME;
	sys.func_initialize = { [](void*) {
		if (!backend_openxr_ext_enabled(XR_EXT_HAND_INTERACTION_EXTENSION_NAME))
			return xr_system_fail;

		// ext/hand_interaction_ext
		// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#ext_hand_interaction_profile
		xr_interaction_profile_t profile = { "ext/hand_interaction_ext" };
		profile.use_shorthand_names       = true;
		profile.is_hand                   = true;
		profile.palm_offset[handed_left ] = pose_identity;
		profile.palm_offset[handed_right] = pose_identity;
		profile.binding[profile.binding_ct++] = { xr_action_pose_grip,      "grip/pose",           "grip/pose"           };
		profile.binding[profile.binding_ct++] = { xr_action_pose_aim,       "aim/pose",            "aim/pose"            };
		profile.binding[profile.binding_ct++] = { xr_action_bool_aim_ready, "pinch_ext/ready_ext", "pinch_ext/ready_ext" };
		profile.binding[profile.binding_ct++] = { xr_action_float_trigger,  "pinch_ext/value",     "pinch_ext/value"     };
		profile.binding[profile.binding_ct++] = { xr_action_float_grip,     "grasp_ext/value",     "grasp_ext/value"     };
		oxri_register_profile(profile);

		return xr_system_succeed;
	} };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

void xr_profile_msft_hand_interaction_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_MSFT_HAND_INTERACTION_EXTENSION_NAME;
	sys.func_initialize = { [](void*) {
		if (!backend_openxr_ext_enabled(XR_MSFT_HAND_INTERACTION_EXTENSION_NAME))
			return xr_system_fail;

		// microsoft/hand_interaction
		// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#ext_hand_interaction_profile
		xr_interaction_profile_t profile = { "microsoft/hand_interaction" };
		profile.use_shorthand_names       = true;
		profile.is_hand                   = true;
		profile.palm_offset[handed_left ] = pose_identity;
		profile.palm_offset[handed_right] = pose_identity;
		profile.binding[profile.binding_ct++] = { xr_action_pose_grip,      "grip/pose",           "grip/pose"           };
		profile.binding[profile.binding_ct++] = { xr_action_pose_aim,       "aim/pose",            "aim/pose"            };
		profile.binding[profile.binding_ct++] = { xr_action_float_trigger,  "select/value",        "select/value"        };
		profile.binding[profile.binding_ct++] = { xr_action_float_grip,     "squeeze/value",       "squeeze/value"       };
		oxri_register_profile(profile);

		return xr_system_succeed;
	} };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

void xr_profile_bd_controller_interaction_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_BD_CONTROLLER_INTERACTION_EXTENSION_NAME;
	sys.func_initialize = { [](void*) {
		if (!backend_openxr_ext_enabled(XR_BD_CONTROLLER_INTERACTION_EXTENSION_NAME))
			return xr_system_fail;

		// Bytedance PICO Neo3
		// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_BD_controller_interaction
		xr_interaction_profile_t profile3 = { "bytedance/pico_neo3_controller" };
		profile3.use_shorthand_names       = true;
		profile3.is_hand                   = false;
		profile3.palm_offset[handed_left ] = pose_t{ {-0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		profile3.palm_offset[handed_right] = pose_t{ { 0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		profile3.binding[profile3.binding_ct++]={ xr_action_pose_grip,      "grip/pose",           "grip/pose"           };
		profile3.binding[profile3.binding_ct++]={ xr_action_pose_aim,       "aim/pose",            "aim/pose"            };
		profile3.binding[profile3.binding_ct++]={ xr_action_float_trigger,  "trigger/value",       "trigger/value"       };
		profile3.binding[profile3.binding_ct++]={ xr_action_float_grip,     "squeeze/value",       "squeeze/value"       };
		profile3.binding[profile3.binding_ct++]={ xr_action_xy_stick,       "thumbstick",          "thumbstick"          };
		profile3.binding[profile3.binding_ct++]={ xr_action_bool_stick,     "thumbstick/click",    "thumbstick/click"    };
		profile3.binding[profile3.binding_ct++]={ xr_action_bool_menu,      "menu/click",          "menu/click"          };
		profile3.binding[profile3.binding_ct++]={ xr_action_bool_x1,        "x/click",             "a/click"             };
		profile3.binding[profile3.binding_ct++]={ xr_action_bool_x2,        "y/click",             "b/click"             };
		oxri_register_profile(profile3);

		// Bytedance Pico 4
		// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_BD_controller_interaction
		// Note that on the pico 4 OS 5.5 OpenXR SDK 2.2, the xrGetCurrentInteractionProfile will return '/interaction_profiles/bytedance/pico_neo3_controller'
		// instead of the expected '/interaction_profiles/bytedance/pico4_controller'
		xr_interaction_profile_t profile4 = { "bytedance/pico4_controller" };
		profile4.use_shorthand_names       = true;
		profile4.is_hand                   = false;
		profile4.palm_offset[handed_left ] = pose_t{ {-0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		profile4.palm_offset[handed_right] = pose_t{ { 0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		profile4.binding[profile4.binding_ct++]={ xr_action_pose_grip,      "grip/pose",           "grip/pose"           };
		profile4.binding[profile4.binding_ct++]={ xr_action_pose_aim,       "aim/pose",            "aim/pose"            };
		profile4.binding[profile4.binding_ct++]={ xr_action_float_trigger,  "trigger/value",       "trigger/value"       };
		profile4.binding[profile4.binding_ct++]={ xr_action_float_grip,     "squeeze/value",       "squeeze/value"       };
		profile4.binding[profile4.binding_ct++]={ xr_action_xy_stick,       "thumbstick",          "thumbstick"          };
		profile4.binding[profile4.binding_ct++]={ xr_action_bool_stick,     "thumbstick/click",    "thumbstick/click"    };
		profile4.binding[profile4.binding_ct++]={ xr_action_bool_menu,      "menu/click"                                 };
		profile4.binding[profile4.binding_ct++]={ xr_action_bool_x1,        "x/click",             "a/click"             };
		profile4.binding[profile4.binding_ct++]={ xr_action_bool_x2,        "y/click",             "b/click"             };
		oxri_register_profile(profile4);

		return xr_system_succeed;
	} };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

} // namespace sk