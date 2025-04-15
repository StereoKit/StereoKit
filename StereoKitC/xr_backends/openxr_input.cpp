/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

#include "../platforms/platform.h"
#if defined(SK_XR_OPENXR)

#include "openxr.h"
#include "openxr_extensions.h"
#include "openxr_input.h"
#include "extensions/ext_management.h"
#include "../hands/hand_oxr_controller.h"
#include "../hands/input_hand.h"
#include "../systems/input.h"
#include "../systems/render.h"

#include "../libraries/array.h"
#include "../stereokit.h"
#include "../_stereokit.h"

#include <openxr/openxr.h>
#include <stdio.h>

namespace sk {

///////////////////////////////////////////

struct xrc_profile_info_t {
	const char* name;
	bool        is_hand;
	XrPath      profile;
	pose_t      offset[2];
};

union XrRLPath {
	struct {
		XrPath left;
		XrPath right;
	};
	struct {
		XrPath paths[2];
	};
};

struct xrc_state_t {
	XrActionSet                       action_set;
	XrAction                          actions[xr_action_max];

	XrRLPath                          hand_subaction;
	XrSpace                           space_aim [2];
	XrSpace                           space_grip[2];
	XrSpace                           space_palm[2];

	int32_t                           eyes_pointer;
	button_state_                     tracked_state;

	array_t<xr_interaction_profile_t> registered_profiles;
	bool                              registration_finished;

	array_t<xrc_profile_info_t>       profiles;
	XrPath                            active_profile[2];
	pose_t                            active_offset [2];
};
static xrc_state_t local = {};

XrSpace xr_gaze_space = {};
bool    xrc_aim_ready[2];

///////////////////////////////////////////

bool oxri_init        ();
void oxri_shutdown    (void*);
void oxri_update_frame(void*);

void oxri_set_profile (handed_ hand, XrPath profile);

bool _make_action_rl  (const char* action_name, const char* display_name, XrActionType type, XrRLPath* in_subaction_path, XrAction *out_action);
bool _make_action     (const char* action_name, const char* display_name, XrActionType type, XrAction* out_action);

///////////////////////////////////////////

void oxri_register() {
	xr_system_t system = {};
	system.func_initialize = { [](void*) { return oxri_init() ? xr_system_succeed : xr_system_fail_critical; } };
	system.func_shutdown   = { oxri_shutdown };
	system.func_step_begin = { oxri_update_frame };
	ext_management_sys_register(system);
}

///////////////////////////////////////////

void oxri_register_profile(xr_interaction_profile_t profile) {
	if (local.registration_finished == true) {
		log_errf("Interaction profile '%s' was registered too late! Please add it before stereokit initialization.", profile.name);
		return;
	}
	// If we already have a registered profile with this name, we prefer the
	// earlier one.
	for (int32_t i = 0; i < local.registered_profiles.count; i++) {
		if (string_eq(profile.name, local.registered_profiles[i].name))
			return;
	}
	local.registered_profiles.add(profile);
}

///////////////////////////////////////////

bool oxri_bind_profile(xr_interaction_profile_t profile, xrc_profile_info_t *out_profile) {
	*out_profile = {};
	bool result = true;

	// EXT_palm_pose applies to all interaction profiles that use /user/hand/*
	// paths, so we need to check if any of the paths have such a string.
	bool has_palm = profile.use_shorthand_names;

	char                      buffer[256];
	XrActionSuggestedBinding* binds   = sk_malloc_t(XrActionSuggestedBinding, profile.binding_ct*2 + 2); // +2 for palm extension
	int32_t                   bind_ct = 0;
	for (int32_t i = 0; i < profile.binding_ct; i++) {
		if (profile.binding[i].action == xr_action_pose_palm && xr_ext.EXT_palm_pose != xr_ext_active)
			continue;

		// If we haven't determined it's a hand path yet, check if it is.
		if (!has_palm && (
				string_startswith(profile.binding[i].paths[handed_left],  "/user/hand/") ||
				string_startswith(profile.binding[i].paths[handed_right], "/user/hand/")))
			has_palm = true;


		if (profile.binding[i].paths[handed_left]) {
			binds[bind_ct].action = local.actions[profile.binding[i].action];

			if (profile.use_shorthand_names) snprintf(buffer, sizeof(buffer), "/user/hand/%s/input/%s", "left", profile.binding[i].paths[handed_left]);
			else                             snprintf(buffer, sizeof(buffer), "%s", profile.binding[i].paths[handed_left]);
			XrResult xr = xrStringToPath(xr_instance, buffer, &binds[bind_ct].binding);
			if (XR_FAILED(xr)) { log_errf("xrStringToPath failed for %s '%s': [%s]", profile.name, buffer, openxr_string(xr)); result = false; goto finish; }
			else               { bind_ct++; }
		}
		if (profile.binding[i].paths[handed_right]) {
			binds[bind_ct].action = local.actions[profile.binding[i].action];

			if (profile.use_shorthand_names) snprintf(buffer, sizeof(buffer), "/user/hand/%s/input/%s", "right", profile.binding[i].paths[handed_right]);
			else                             snprintf(buffer, sizeof(buffer), "%s", profile.binding[i].paths[handed_right]);
			XrResult xr = xrStringToPath(xr_instance, buffer, &binds[bind_ct].binding);
			if (XR_FAILED(xr)) { log_errf("xrStringToPath failed for %s '%s': [%s]", profile.name, buffer, openxr_string(xr)); result = false; goto finish; }
			else               { bind_ct++; }
		}
	}

	if (has_palm && xr_ext.EXT_palm_pose) {
		binds[bind_ct].action = local.actions[xr_action_pose_palm];

		snprintf(buffer, sizeof(buffer), "/user/hand/%s/input/%s", "left", "palm_ext/pose");
		XrResult xr = xrStringToPath(xr_instance, buffer, &binds[bind_ct].binding);
		if (XR_FAILED(xr)) { log_errf("xrStringToPath failed for %s '%s': [%s]", profile.name, buffer, openxr_string(xr)); result = false; goto finish; }
		else               { bind_ct++; }

		binds[bind_ct].action = local.actions[xr_action_pose_palm];

		snprintf(buffer, sizeof(buffer), "/user/hand/%s/input/%s", "right", "palm_ext/pose");
		xr = xrStringToPath(xr_instance, buffer, &binds[bind_ct].binding);
		if (XR_FAILED(xr)) { log_errf("xrStringToPath failed for %s '%s': [%s]", profile.name, buffer, openxr_string(xr)); result = false; goto finish; }
		else               { bind_ct++; }
	}

	XrPath interaction_path;
	snprintf(buffer, sizeof(buffer), "/interaction_profiles/%s", profile.name);
	xrStringToPath(xr_instance, buffer, &interaction_path);

	XrInteractionProfileSuggestedBinding suggested_binds = { XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
	suggested_binds.interactionProfile     = interaction_path;
	suggested_binds.suggestedBindings      = binds;
	suggested_binds.countSuggestedBindings = bind_ct;
	if (XR_FAILED(xrSuggestInteractionProfileBindings(xr_instance, &suggested_binds))) {
		result = false;
		goto finish;
	}

	out_profile->profile              = interaction_path;
	out_profile->name                 = profile.name;
	out_profile->is_hand              = profile.is_hand;
	out_profile->offset[handed_left ] = profile.palm_offset[handed_left];
	out_profile->offset[handed_right] = profile.palm_offset[handed_right];

finish:
	sk_free(binds);
	return result;
}

///////////////////////////////////////////

bool oxri_init() {
	local = {};
	local.active_offset [0] = pose_identity;
	local.active_offset [1] = pose_identity;
	local.active_profile[0] = 0xFFFFFFFF;
	local.active_profile[1] = 0xFFFFFFFF;
	local.eyes_pointer      = input_add_pointer(input_source_gaze | (device_has_eye_gaze() ? input_source_gaze_eyes : input_source_gaze_head));

	XrActionSetCreateInfo actionset_info = { XR_TYPE_ACTION_SET_CREATE_INFO };
	snprintf(actionset_info.actionSetName,          sizeof(actionset_info.actionSetName),          "input");
	snprintf(actionset_info.localizedActionSetName, sizeof(actionset_info.localizedActionSetName), "Input");
	XrResult result = xrCreateActionSet(xr_instance, &actionset_info, &local.action_set);
	if (XR_FAILED(result)) {
		log_infof("xrCreateActionSet failed: [%s]", openxr_string(result));
		return false;
	}

	xrStringToPath(xr_instance, "/user/hand/left",  &local.hand_subaction.left);
	xrStringToPath(xr_instance, "/user/hand/right", &local.hand_subaction.right);

	if (!_make_action_rl("grip_pose",   "Grip Pose",   XR_ACTION_TYPE_POSE_INPUT,     &local.hand_subaction, &local.actions[xr_action_pose_grip     ])) return false;
	if (!_make_action_rl("aim_pose",    "Aim Pose",    XR_ACTION_TYPE_POSE_INPUT,     &local.hand_subaction, &local.actions[xr_action_pose_aim      ])) return false;
	if (!_make_action_rl("aim_ready",   "Aim Ready",   XR_ACTION_TYPE_BOOLEAN_INPUT,  &local.hand_subaction, &local.actions[xr_action_bool_aim_ready])) return false;
	if (!_make_action_rl("trigger",     "Trigger",     XR_ACTION_TYPE_FLOAT_INPUT,    &local.hand_subaction, &local.actions[xr_action_float_trigger ])) return false;
	if (!_make_action_rl("grip",        "Grip",        XR_ACTION_TYPE_FLOAT_INPUT,    &local.hand_subaction, &local.actions[xr_action_float_grip    ])) return false;
	if (!_make_action_rl("stick_xy",    "Stick XY",    XR_ACTION_TYPE_VECTOR2F_INPUT, &local.hand_subaction, &local.actions[xr_action_xy_stick      ])) return false;
	if (!_make_action_rl("stick_click", "Stick Click", XR_ACTION_TYPE_BOOLEAN_INPUT,  &local.hand_subaction, &local.actions[xr_action_bool_stick    ])) return false;
	if (!_make_action_rl("menu",        "Menu",        XR_ACTION_TYPE_BOOLEAN_INPUT,  &local.hand_subaction, &local.actions[xr_action_bool_menu     ])) return false;
	if (!_make_action_rl("button_x1",   "Button X1",   XR_ACTION_TYPE_BOOLEAN_INPUT,  &local.hand_subaction, &local.actions[xr_action_bool_x1       ])) return false;
	if (!_make_action_rl("button_x2",   "Button X2",   XR_ACTION_TYPE_BOOLEAN_INPUT,  &local.hand_subaction, &local.actions[xr_action_bool_x2       ])) return false;

	if (xr_ext.EXT_palm_pose == xr_ext_active && !_make_action_rl("palm_pose", "Palm Pose", XR_ACTION_TYPE_POSE_INPUT, &local.hand_subaction, &local.actions[xr_action_pose_palm])) return false;
	if (device_has_eye_gaze()                 && !_make_action   ("eye_gaze",  "Eye Gaze",  XR_ACTION_TYPE_POSE_INPUT,                        &local.actions[xr_action_pose_eyes])) return false;

	// Bind the actions we just created to specific locations on the input
	// profiles! These are 'suggested' because they may be overridden by the
	// runtime preferences. For example, if the runtime allows you to remap
	// buttons, or provides input accessibility settings.

	// OpenXR's ideal situation is that you provide bindings for the
	// controllers that you know and use. Runtimes are then supposed to remap
	// those bindings to the controller that is actually present. But in
	// reality, the remapping doesn't always happen.

	// Here, we provide bindings for as many controllers as possible, so that
	// runtimes don't have the opportunity to drop the ball. These profiles are
	// all part of the core spec, but there are a few more specified as
	// extensions in /xr_backends/extensions/input_profiles.cpp.

	// microsoft/motion_controller
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_microsoft_mixed_reality_motion_controller_profile
	{
		pose_t palm_offset = device_display_get_blend() == display_blend_opaque
			? pose_t{ {0.01f, -0.01f,  0.015f}, quat_from_angles(-45, 0, 0) }
			: pose_t{ {0,      0.005f, 0     }, quat_from_angles(-68, 0, 0) };

		xr_interaction_profile_t profile = { "microsoft/motion_controller" };
		profile.use_shorthand_names       = true;
		profile.is_hand                   = false;
		profile.palm_offset[handed_left ] = palm_offset;
		profile.palm_offset[handed_right] = palm_offset;
		profile.binding[profile.binding_ct++] = { xr_action_pose_grip,      "grip/pose",           "grip/pose"           };
		profile.binding[profile.binding_ct++] = { xr_action_pose_aim,       "aim/pose",            "aim/pose"            };
		profile.binding[profile.binding_ct++] = { xr_action_float_trigger,  "trigger/value",       "trigger/value"       };
		profile.binding[profile.binding_ct++] = { xr_action_float_grip,     "squeeze/click",       "squeeze/click"       };
		profile.binding[profile.binding_ct++] = { xr_action_xy_stick,       "thumbstick",          "thumbstick"          };
		profile.binding[profile.binding_ct++] = { xr_action_bool_stick,     "thumbstick/click",    "thumbstick/click"    };
		profile.binding[profile.binding_ct++] = { xr_action_bool_menu,      "menu/click",          "menu/click"          };

		oxri_register_profile(profile);
	}

	// htc/vive_controller
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_htc_vive_controller_profile
	{
		xr_interaction_profile_t profile = { "htc/vive_controller" };
		profile.use_shorthand_names       = true;
		profile.is_hand                   = false;
		profile.palm_offset[handed_left ] = pose_t{ {-0.035f, 0, 0}, quat_from_angles(-40, 0, 0) };
		profile.palm_offset[handed_right] = pose_t{ { 0.035f, 0, 0}, quat_from_angles(-40, 0, 0) };
		profile.binding[profile.binding_ct++] = { xr_action_pose_grip,      "grip/pose",           "grip/pose"           };
		profile.binding[profile.binding_ct++] = { xr_action_pose_aim,       "aim/pose",            "aim/pose"            };
		profile.binding[profile.binding_ct++] = { xr_action_float_trigger,  "trigger/value",       "trigger/value"       };
		profile.binding[profile.binding_ct++] = { xr_action_float_grip,     "squeeze/click",       "squeeze/click"       };
		profile.binding[profile.binding_ct++] = { xr_action_bool_menu,      "menu/click",          "menu/click"          };

		oxri_register_profile(profile);
	}

	// valve/index_controller
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_valve_index_controller_profile
	{
		xr_interaction_profile_t profile = { "valve/index_controller" };
		profile.use_shorthand_names       = true;
		profile.is_hand                   = false;
		profile.palm_offset[handed_left ] = pose_t{ {-0.035f, 0, 0}, quat_from_angles(-40, 0, 0) };
		profile.palm_offset[handed_right] = pose_t{ { 0.035f, 0, 0}, quat_from_angles(-40, 0, 0) };
		profile.binding[profile.binding_ct++] = { xr_action_pose_grip,      "grip/pose",           "grip/pose"           };
		profile.binding[profile.binding_ct++] = { xr_action_pose_aim,       "aim/pose",            "aim/pose"            };
		profile.binding[profile.binding_ct++] = { xr_action_float_trigger,  "trigger/value",       "trigger/value"       };
		profile.binding[profile.binding_ct++] = { xr_action_float_grip,     "squeeze/value",       "squeeze/value"       };
		profile.binding[profile.binding_ct++] = { xr_action_xy_stick,       "thumbstick",          "thumbstick"          };
		profile.binding[profile.binding_ct++] = { xr_action_bool_stick,     "thumbstick/click",    "thumbstick/click"    };
		profile.binding[profile.binding_ct++] = { xr_action_bool_menu,      "system/click",        "system/click"        };
		profile.binding[profile.binding_ct++] = { xr_action_bool_x1,        "a/click",             "a/click"             };
		profile.binding[profile.binding_ct++] = { xr_action_bool_x2,        "b/click",             "b/click"             };

		oxri_register_profile(profile);
	}

	// oculus/touch_controller
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_oculus_touch_controller_profile
	{
		xr_interaction_profile_t profile = { "oculus/touch_controller" };
		profile.use_shorthand_names       = true;
		profile.is_hand                   = false;
		profile.palm_offset[handed_left ] = pose_t{ {-0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		profile.palm_offset[handed_right] = pose_t{ { 0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		profile.binding[profile.binding_ct++] = { xr_action_pose_grip,      "grip/pose",           "grip/pose"           };
		profile.binding[profile.binding_ct++] = { xr_action_pose_aim,       "aim/pose",            "aim/pose"            };
		profile.binding[profile.binding_ct++] = { xr_action_float_trigger,  "trigger/value",       "trigger/value"       };
		profile.binding[profile.binding_ct++] = { xr_action_float_grip,     "squeeze/value",       "squeeze/value"       };
		profile.binding[profile.binding_ct++] = { xr_action_xy_stick,       "thumbstick",          "thumbstick"          };
		profile.binding[profile.binding_ct++] = { xr_action_bool_stick,     "thumbstick/click",    "thumbstick/click"    };
		profile.binding[profile.binding_ct++] = { xr_action_bool_menu,      "menu/click",                                };
		profile.binding[profile.binding_ct++] = { xr_action_bool_x1,        "x/click",             "a/click"             };
		profile.binding[profile.binding_ct++] = { xr_action_bool_x2,        "y/click",             "b/click"             };

		oxri_register_profile(profile);
	}

	// khr/simple_controller
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_khronos_simple_controller_profile
	{
		xr_interaction_profile_t profile = { "khr/simple_controller" };
		profile.use_shorthand_names       = true;
		profile.is_hand                   = false;
		profile.palm_offset[handed_left ] = pose_identity;
		profile.palm_offset[handed_right] = pose_identity;
		profile.binding[profile.binding_ct++] = { xr_action_pose_grip,      "grip/pose",           "grip/pose"           };
		profile.binding[profile.binding_ct++] = { xr_action_pose_aim,       "aim/pose",            "aim/pose"            };
		profile.binding[profile.binding_ct++] = { xr_action_float_trigger,  "select/click",        "select/click"        };
		profile.binding[profile.binding_ct++] = { xr_action_bool_menu,      "menu/click",          "menu/click"          };

		oxri_register_profile(profile);
	}

	// Eye tracking
	if (device_has_eye_gaze()) {
		xr_interaction_profile_t profile = { "ext/eye_gaze_interaction" };
		profile.use_shorthand_names       = false;
		profile.is_hand                   = false;
		profile.palm_offset[handed_left ] = pose_identity;
		profile.palm_offset[handed_right] = pose_identity;
		profile.binding[profile.binding_ct++] = { xr_action_pose_eyes,      "/user/eyes_ext/input/gaze_ext/pose"         };

		xrc_profile_info_t bind_info = {};
		if (oxri_bind_profile(profile, &bind_info)) {
			XrActionSpaceCreateInfo create_space = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
			create_space.action            = local.actions[xr_action_pose_eyes];
			create_space.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
			result = xrCreateActionSpace(xr_session, &create_space, &xr_gaze_space);
			if (XR_FAILED(result)) {
				log_warnf("Gaze xrCreateActionSpace failed: [%s]", openxr_string(result));
			}
		}
	}

	// Suggest all our input profiles
	local.registration_finished = true;
	for (int32_t i = 0; i < local.registered_profiles.count; i++) {
		xrc_profile_info_t bind_info = {};
		if (oxri_bind_profile(local.registered_profiles[i], &bind_info)) {
			local.profiles.add(bind_info);
		}
	}
	local.registered_profiles.free();

	// Create frames of reference for the pose actions
	for (int32_t i = 0; i < 2; i++) {
		XrActionSpaceCreateInfo action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
		action_space_info.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
		action_space_info.subactionPath     = local.hand_subaction.paths[i];
		action_space_info.action            = local.actions[xr_action_pose_grip];
		result = xrCreateActionSpace(xr_session, &action_space_info, &local.space_grip[i]);
		if (XR_FAILED(result)) {
			log_errf("xrCreateActionSpace failed: [%s]", openxr_string(result));
			return false;
		}

		action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
		action_space_info.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
		action_space_info.subactionPath     = local.hand_subaction.paths[i];
		action_space_info.action            = local.actions[xr_action_pose_aim];
		result = xrCreateActionSpace(xr_session, &action_space_info, &local.space_aim[i]);
		if (XR_FAILED(result)) {
			log_errf("xrCreateActionSpace failed: [%s]", openxr_string(result));
			return false;
		}

		if (xr_ext.EXT_palm_pose == xr_ext_active) {
			action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
			action_space_info.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
			action_space_info.subactionPath     = local.hand_subaction.paths[i];
			action_space_info.action            = local.actions[xr_action_pose_palm];
			result = xrCreateActionSpace(xr_session, &action_space_info, &local.space_palm[i]);
			if (XR_FAILED(result)) {
				log_errf("xrCreateActionSpace failed: [%s]", openxr_string(result));
				return false;
			}
		}
	}

	// Attach the action set we just made to the session
	XrSessionActionSetsAttachInfo attach_info = { XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
	attach_info.countActionSets = 1;
	attach_info.actionSets      = &local.action_set;
	result = xrAttachSessionActionSets(xr_session, &attach_info);
	if (XR_FAILED(result)) {
		log_errf("xrAttachSessionActionSets failed: [%s]", openxr_string(result));
		return false;
	}

	oxri_set_profile(handed_left,  local.active_profile[handed_left ]);
	oxri_set_profile(handed_right, local.active_profile[handed_right]);
	return true;
}

///////////////////////////////////////////

void oxri_shutdown(void*) {
	local.profiles.free();

	if (xr_gaze_space) { xrDestroySpace(xr_gaze_space); xr_gaze_space = {}; }
	for (int32_t i = 0; i < 2; i++) {
		if (local.space_grip[i]) { xrDestroySpace(local.space_grip[i]); local.space_grip[i] = {}; }
		if (local.space_aim [i]) { xrDestroySpace(local.space_aim [i]); local.space_aim [i] = {}; }
		if (local.space_palm[i]) { xrDestroySpace(local.space_palm[i]); local.space_palm[i] = {}; }
	}
	if (local.action_set) { xrDestroyActionSet(local.action_set); local.action_set = {}; }
	local = {};
}


///////////////////////////////////////////

button_state_ openxr_space_tracked() {
	return local.tracked_state;
}

///////////////////////////////////////////

void oxri_update_poses() {
	// We occasionally need to update poses multiple times per frame, to
	// account for camera movement, or predicted time updates. This code is 
	// called separate from button press and tracking events to prevent
	// issues with hiding 'just_active/inactive' events.

	// Update our action set with up-to-date input data!
	XrActiveActionSet action_set = { };
	action_set.actionSet     = local.action_set;
	action_set.subactionPath = XR_NULL_PATH;
	XrActionsSyncInfo sync_info = { XR_TYPE_ACTIONS_SYNC_INFO };
	sync_info.countActiveActionSets = 1;
	sync_info.activeActionSets      = &action_set;
	xrSyncActions(xr_session, &sync_info);

	matrix root   = render_get_cam_final    ();
	quat   root_q = matrix_extract_rotation(root);

	// Track the head location, and use it to determine the tracking state of
	// the world.
	XrSpaceLocation head_location = { XR_TYPE_SPACE_LOCATION };
	XrResult        res           = xrLocateSpace(xr_head_space, xr_app_space, xr_time, &head_location);
	if (XR_UNQUALIFIED_SUCCESS(res) && 
		(head_location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT   ) != 0 &&
		(head_location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0) {

		memcpy(&input_head_pose_local.position,    &head_location.pose.position,    sizeof(vec3));
		memcpy(&input_head_pose_local.orientation, &head_location.pose.orientation, sizeof(quat));
	}
	// We report tracking of the device based on the positional tracking
	// reported here. Rotational tracking is pretty much always available to
	// devices containing an accelerometer/gyroscope, so positional is the best
	// metric for tracking quality.
	local.tracked_state = button_make_state(
		(local.tracked_state            & button_state_active)                    != 0,
		(head_location.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT) != 0);

	input_head_pose_world = matrix_transform_pose(root, input_head_pose_local);

	// Get input from whatever controllers may be present
	for (uint32_t hand = 0; hand < handed_max; hand++) {
		controller_t*        controller = input_controller_ref((handed_)hand);
		XrActionStateGetInfo get_info   = { XR_TYPE_ACTION_STATE_GET_INFO };
		get_info.subactionPath = local.hand_subaction.paths[hand];

		//// Pose actions

		// Get the grip pose the verbose way, since we want to know if
		// rotation or position are tracked independently of eachother.
		XrSpaceLocation space_location = { XR_TYPE_SPACE_LOCATION };
		res = xrLocateSpace(local.space_grip[hand], xr_app_space, xr_time, &space_location);
		bool tracked_pos = false;
		bool tracked_rot = false;
		bool valid_pos   = false;
		bool valid_rot   = false;
		if (XR_UNQUALIFIED_SUCCESS(res)) {
			tracked_pos = (space_location.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT)    != 0;
			tracked_rot = (space_location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT) != 0;
			valid_pos   = (space_location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)      != 0;
			valid_rot   = (space_location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)   != 0;
			if (valid_pos) {
				vec3 local_pos;
				memcpy(&local_pos, &space_location.pose.position, sizeof(vec3));
				controller->pose.position = root * local_pos;
			}
			if (valid_rot) {
				quat local_rot;
				memcpy(&local_rot, &space_location.pose.orientation, sizeof(quat));
				controller->pose.orientation = local_rot * root_q;
			}
			controller->tracked_pos = tracked_pos ? track_state_known : (valid_pos ? track_state_inferred : track_state_lost);
			controller->tracked_rot = tracked_rot ? track_state_known : (valid_rot ? track_state_inferred : track_state_lost);
		}

		// Get the palm position from either the extension, or our previous
		// fallback offsets from the grip pose.
		if (xr_ext.EXT_palm_pose == xr_ext_active) {
			space_location = { XR_TYPE_SPACE_LOCATION };
			res = xrLocateSpace(local.space_palm[hand], xr_app_space, xr_time, &space_location);
			if (XR_UNQUALIFIED_SUCCESS(res)) {
				pose_t local_palm;
				if (space_location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) {
					memcpy(&local_palm.position, &space_location.pose.position, sizeof(vec3));
					controller->palm.position = root * local_palm.position;
				}
				if (space_location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) {
					memcpy(&local_palm.orientation, &space_location.pose.orientation, sizeof(quat));
					controller->palm.orientation = local_palm.orientation * root_q;
				}
			}
		} else {
			controller->palm.orientation = local.active_offset[hand].orientation * controller->pose.orientation;
			controller->palm.position    = controller->pose.position + controller->palm.orientation * local.active_offset[hand].position;
		}

		// Controller aim pose
		space_location = { XR_TYPE_SPACE_LOCATION };
		res = xrLocateSpace(local.space_aim[hand], xr_app_space, xr_time, &space_location);
		if (XR_UNQUALIFIED_SUCCESS(res)) {
			pose_t local_aim;
			if (space_location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) {
				memcpy(&local_aim.position, &space_location.pose.position, sizeof(vec3));
				controller->aim.position = root * local_aim.position;
			}
			if (space_location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) {
				memcpy(&local_aim.orientation, &space_location.pose.orientation, sizeof(quat));
				controller->aim.orientation = local_aim.orientation * root_q;
			}
		}
	}

	// eye input
	if (device_has_eye_gaze()) {
		pointer_t           *pointer     = input_get_pointer(local.eyes_pointer);
		XrActionStatePose    action_pose = {XR_TYPE_ACTION_STATE_POSE};
		XrActionStateGetInfo action_info = {XR_TYPE_ACTION_STATE_GET_INFO};
		action_info.action = local.actions[xr_action_pose_eyes];
		xrGetActionStatePose(xr_session, &action_info, &action_pose);

		if (action_pose.isActive && openxr_get_gaze_space(&input_eyes_pose_local, xr_eyes_sample_time)) {
			input_eyes_pose_world.position    = root   * input_eyes_pose_local.position;
			input_eyes_pose_world.orientation = root_q * input_eyes_pose_local.orientation;

			pointer->ray.pos     = input_eyes_pose_world.position;
			pointer->ray.dir     = input_eyes_pose_world.orientation * vec3_forward;
			pointer->orientation = input_eyes_pose_world.orientation;
		}
	}
}

///////////////////////////////////////////

void oxri_update_frame(void*) {
	if (xr_session_state != XR_SESSION_STATE_FOCUSED)
		return;

	// This function call already syncs OpenXR actions
	oxri_update_poses();

	// Get input from whatever controllers may be present
	bool menu_button = false;
	for (uint32_t hand = 0; hand < handed_max; hand++) {
		controller_t*        controller = input_controller_ref((handed_)hand);
		XrActionStateGetInfo get_info   = { XR_TYPE_ACTION_STATE_GET_INFO };
		get_info.subactionPath = local.hand_subaction.paths[hand];

		//// Pose actions

		controller->tracked = button_make_state(
			controller->tracked & button_state_active,
			(controller->tracked_pos != track_state_lost ||
			 controller->tracked_rot != track_state_lost));

		//// Float actions

		// Trigger
		XrActionStateFloat state_trigger = { XR_TYPE_ACTION_STATE_FLOAT };
		get_info.action = local.actions[xr_action_float_trigger];
		xrGetActionStateFloat(xr_session, &get_info, &state_trigger);
		controller->trigger = state_trigger.currentState;

		// Grip button
		XrActionStateFloat state_grip_btn = { XR_TYPE_ACTION_STATE_FLOAT };
		get_info.action = local.actions[xr_action_float_grip];
		xrGetActionStateFloat(xr_session, &get_info, &state_grip_btn);
		controller->grip = state_grip_btn.currentState;

		// Analog stick X and Y
		XrActionStateVector2f grip_stick_xy = { XR_TYPE_ACTION_STATE_VECTOR2F };
		get_info.action = local.actions[xr_action_xy_stick];
		xrGetActionStateVector2f(xr_session, &get_info, &grip_stick_xy);
		controller->stick.x = -grip_stick_xy.currentState.x;
		controller->stick.y =  grip_stick_xy.currentState.y;

		//// Bool actions

		// Menu button
		XrActionStateBoolean state_menu = { XR_TYPE_ACTION_STATE_BOOLEAN };
		get_info.action = local.actions[xr_action_bool_menu];
		xrGetActionStateBoolean(xr_session, &get_info, &state_menu);
		menu_button = menu_button || state_menu.currentState;

		// Stick click
		XrActionStateBoolean state_stick_click = { XR_TYPE_ACTION_STATE_BOOLEAN };
		get_info.action = local.actions[xr_action_bool_stick];
		xrGetActionStateBoolean(xr_session, &get_info, &state_stick_click);
		controller->stick_click = button_make_state(controller->stick_click & button_state_active, state_stick_click.currentState);

		// Button x1 and x2
		XrActionStateBoolean state_x1 = { XR_TYPE_ACTION_STATE_BOOLEAN };
		XrActionStateBoolean state_x2 = { XR_TYPE_ACTION_STATE_BOOLEAN };
		get_info.action = local.actions[xr_action_bool_x1];
		xrGetActionStateBoolean(xr_session, &get_info, &state_x1);
		get_info.action = local.actions[xr_action_bool_x2];
		xrGetActionStateBoolean(xr_session, &get_info, &state_x2);
		controller->x1 = button_make_state(controller->x1 & button_state_active, state_x1.currentState);
		controller->x2 = button_make_state(controller->x2 & button_state_active, state_x2.currentState);

		// Aim ready
		if (local.actions[xr_action_bool_aim_ready] != XR_NULL_HANDLE) {
			XrActionStateBoolean state_aim_ready = { XR_TYPE_ACTION_STATE_BOOLEAN };
			get_info.action = local.actions[xr_action_bool_aim_ready];
			xrGetActionStateBoolean(xr_session, &get_info, &state_aim_ready);
			xrc_aim_ready[hand] = state_aim_ready.currentState;
		}
	}
	input_controller_menu_set(button_make_state(input_controller_menu() & button_state_active, menu_button));

	// eye input
	if (device_has_eye_gaze()) {
		pointer_t           *pointer     = input_get_pointer(local.eyes_pointer);
		XrActionStatePose    action_pose = {XR_TYPE_ACTION_STATE_POSE};
		XrActionStateGetInfo action_info = {XR_TYPE_ACTION_STATE_GET_INFO};
		action_info.action = local.actions[xr_action_pose_eyes];
		xrGetActionStatePose(xr_session, &action_info, &action_pose);

		input_eyes_tracked_set(button_make_state(input_eyes_tracked() & button_state_active, action_pose.isActive));
		pointer->tracked = input_eyes_tracked();
	}
}

///////////////////////////////////////////

void oxri_set_profile(handed_ hand, XrPath profile) {
	local.active_profile[hand] = profile;
	for (int32_t i = 0; i < local.profiles.count; i++) {
		if (local.profiles[i].profile == profile) {
			local.active_offset[hand] = local.profiles[i].offset[hand];
			input_controller_set_hand(hand, local.profiles[i].is_hand);
			log_diagf("Switched %s controller profile to %s", hand == handed_left ? "left" : "right", local.profiles[i].name);
			break;
		}
	}
}

///////////////////////////////////////////

void oxri_update_interaction_profile() {
	XrInteractionProfileState active_profile = { XR_TYPE_INTERACTION_PROFILE_STATE };
	for (int32_t h = 0; h < handed_max; h++) {
		if (XR_FAILED(xrGetCurrentInteractionProfile(xr_session, local.hand_subaction.paths[h], &active_profile)))
			continue;
		if (active_profile.interactionProfile != local.active_profile[h])
			oxri_set_profile((handed_)h, active_profile.interactionProfile);
	}
}

///////////////////////////////////////////

bool _make_action_arr(const char* action_name, const char* display_name, XrActionType type, XrPath *subaction_path, int32_t path_count, XrAction *out_action) {
	XrActionCreateInfo action_info = { XR_TYPE_ACTION_CREATE_INFO };
	action_info.countSubactionPaths = path_count;
	action_info.subactionPaths      = subaction_path;
	action_info.actionType          = type;
	snprintf(action_info.actionName,          sizeof(action_info.actionName         ), "%s", action_name);
	snprintf(action_info.localizedActionName, sizeof(action_info.localizedActionName), "%s", display_name);
	XrResult result = xrCreateAction(local.action_set, &action_info, out_action);
	if (XR_FAILED(result)) {
		log_infof("xrCreateAction failed: [%s]", openxr_string(result));
		return false;
	}
	return true;
}

///////////////////////////////////////////

bool _make_action_rl(const char* action_name, const char* display_name, XrActionType type, XrRLPath *in_subaction_path, XrAction *out_action) {
	return _make_action_arr(action_name, display_name, type, in_subaction_path->paths, 2, out_action);
}

///////////////////////////////////////////

bool _make_action(const char* action_name, const char* display_name, XrActionType type, XrAction *out_action) {
	return _make_action_arr(action_name, display_name, type, nullptr, 0, out_action);
}

} // namespace sk
#endif