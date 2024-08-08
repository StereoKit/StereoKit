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
	XrActionSet action_set;
	XrAction    action_eyes;
	XrAction    action_grip_pose;
	XrAction    action_palm_pose;
	XrAction    action_aim_pose;
	XrAction    action_aim_ready;
	XrAction    action_trigger;
	XrAction    action_grip;
	XrAction    action_stick_xy;
	XrAction    action_stick_click;
	XrAction    action_x1;
	XrAction    action_x2;
	XrAction    action_menu;

	XrRLPath    hand_subaction;
	XrSpace     space_aim[2];
	XrSpace     space_grip[2];
	XrSpace     space_palm[2];

	int32_t       eyes_pointer;
	button_state_ tracked_state;

	array_t<xrc_profile_info_t> profiles;
	XrPath                      active_profile[2];
	pose_t                      active_offset[2];
};
static xrc_state_t local = {};

XrSpace xr_gaze_space = {};
bool    xrc_aim_ready[2];

///////////////////////////////////////////

void oxri_set_profile(handed_ hand, XrPath profile);

XrRLPath _bind_paths    (const char* name);
void     _bind          (array_t<XrActionSuggestedBinding>* arr, XrAction action, XrPath   path);
void     _bind_rl       (array_t<XrActionSuggestedBinding>* arr, XrAction action, XrRLPath rlpath);
bool     _bind_suggest  (const char* profile_name, bool is_hand, const array_t<XrActionSuggestedBinding> binding_arr, pose_t palm_left_offset, pose_t palm_right_offset, xrc_profile_info_t* out_profile);
bool     _make_action_rl(const char* action_name, const char* display_name, XrActionType type, XrRLPath* in_subaction_path, XrAction *out_action);
bool     _make_action   (const char* action_name, const char* display_name, XrActionType type, XrAction* out_action);

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

	if (!_make_action_rl("grip_pose",   "Grip Pose",   XR_ACTION_TYPE_POSE_INPUT,     &local.hand_subaction, &local.action_grip_pose  )) return false;
	if (!_make_action_rl("aim_pose",    "Aim Pose",    XR_ACTION_TYPE_POSE_INPUT,     &local.hand_subaction, &local.action_aim_pose   )) return false;
	if (!_make_action_rl("aim_ready",   "Aim Ready",   XR_ACTION_TYPE_BOOLEAN_INPUT,  &local.hand_subaction, &local.action_aim_ready  )) return false;
	if (!_make_action_rl("trigger",     "Trigger",     XR_ACTION_TYPE_FLOAT_INPUT,    &local.hand_subaction, &local.action_trigger    )) return false;
	if (!_make_action_rl("grip",        "Grip",        XR_ACTION_TYPE_FLOAT_INPUT,    &local.hand_subaction, &local.action_grip       )) return false;
	if (!_make_action_rl("stick_xy",    "Stick XY",    XR_ACTION_TYPE_VECTOR2F_INPUT, &local.hand_subaction, &local.action_stick_xy   )) return false;
	if (!_make_action_rl("stick_click", "Stick Click", XR_ACTION_TYPE_BOOLEAN_INPUT,  &local.hand_subaction, &local.action_stick_click)) return false;
	if (!_make_action_rl("menu",        "Menu",        XR_ACTION_TYPE_BOOLEAN_INPUT,  &local.hand_subaction, &local.action_menu       )) return false;
	if (!_make_action_rl("button_x1",   "Button X1",   XR_ACTION_TYPE_BOOLEAN_INPUT,  &local.hand_subaction, &local.action_x1         )) return false;
	if (!_make_action_rl("button_x2",   "Button X2",   XR_ACTION_TYPE_BOOLEAN_INPUT,  &local.hand_subaction, &local.action_x2         )) return false;

	if (xr_ext.EXT_palm_pose == xr_ext_active && !_make_action_rl("palm_pose", "Palm Pose", XR_ACTION_TYPE_POSE_INPUT, &local.hand_subaction, &local.action_palm_pose)) return false;
	if (device_has_eye_gaze()                                 && !_make_action   ("eye_gaze",  "Eye Gaze",  XR_ACTION_TYPE_POSE_INPUT,                        &local.action_eyes     )) return false;

	// Bind the actions we just created to specific locations on the Khronos simple_controller
	// definition! These are labeled as 'suggested' because they may be overridden by the runtime
	// preferences. For example, if the runtime allows you to remap buttons, or provides input
	// accessibility settings.
	XrRLPath path_aim_pose      = _bind_paths("aim/pose");
	XrRLPath path_grip_pose     = _bind_paths("grip/pose");
	XrRLPath path_trigger_val   = _bind_paths("trigger/value");
	XrRLPath path_select_val    = _bind_paths("select/value");
	XrRLPath path_select_click  = _bind_paths("select/click");
	XrRLPath path_squeeze_val   = _bind_paths("squeeze/value");
	XrRLPath path_squeeze_click = _bind_paths("squeeze/click");
	XrRLPath path_stick_xy      = _bind_paths("thumbstick");
	XrRLPath path_stick_click   = _bind_paths("thumbstick/click");
	XrRLPath path_system_click  = _bind_paths("system/click");
	XrRLPath path_menu_click    = _bind_paths("menu/click");
	XrRLPath path_back_click    = _bind_paths("back/click");
	XrRLPath path_x_click       = _bind_paths("x/click");
	XrRLPath path_y_click       = _bind_paths("y/click");
	XrRLPath path_a_click       = _bind_paths("a/click");
	XrRLPath path_b_click       = _bind_paths("b/click");

	XrRLPath path_pinch_val     = xr_ext.EXT_hand_interaction == xr_ext_active ? _bind_paths("pinch_ext/value")     : XrRLPath{};
	XrRLPath path_pinch_ready   = xr_ext.EXT_hand_interaction == xr_ext_active ? _bind_paths("pinch_ext/ready_ext") : XrRLPath{};
	XrRLPath path_grasp_val     = xr_ext.EXT_hand_interaction == xr_ext_active ? _bind_paths("grasp_ext/value") : XrRLPath{};
	XrRLPath path_palm_pose     = xr_ext.EXT_palm_pose        == xr_ext_active ? _bind_paths("palm_ext/pose"  ) : XrRLPath{};

	// OpenXR's ideal situation is that you provide bindings for the
	// controllers that you know and use. Runtimes are then supposed to remap
	// those bindings to the controller that is actually present. But in
	// reality, the remapping doesn't always happen.
	//
	// Here, we provide bindings for as many controllers as possible, so that
	// runtimes don't have the opportunity to drop the ball.


	xrc_profile_info_t                bind_info = {};
	array_t<XrActionSuggestedBinding> bind_arr  = {};

	// microsoft/motion_controller
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_microsoft_mixed_reality_motion_controller_profile
	{
		bind_arr.clear();
		_bind_rl(&bind_arr, local.action_grip_pose,   path_grip_pose    );
		_bind_rl(&bind_arr, local.action_aim_pose,    path_aim_pose     );
		_bind_rl(&bind_arr, local.action_trigger,     path_trigger_val  );
		_bind_rl(&bind_arr, local.action_grip,        path_squeeze_click);
		_bind_rl(&bind_arr, local.action_stick_xy,    path_stick_xy     );
		_bind_rl(&bind_arr, local.action_stick_click, path_stick_click  );
		_bind_rl(&bind_arr, local.action_menu,        path_menu_click   );
		if (xr_ext.EXT_palm_pose == xr_ext_active) _bind_rl(&bind_arr, local.action_palm_pose, path_palm_pose);

		pose_t palm_offset = device_display_get_blend() == display_blend_opaque
			? pose_t{ {0.01f, -0.01f,  0.015f}, quat_from_angles(-45, 0, 0) }
			: pose_t{ {0,      0.005f, 0     }, quat_from_angles(-68, 0, 0) };
		if (_bind_suggest("microsoft/motion_controller", false, bind_arr, palm_offset, palm_offset, &bind_info))
			local.profiles.add(bind_info);
	}

	// hp/mixed_reality_controller
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_EXT_hp_mixed_reality_controller
	if (xr_ext.EXT_hp_mixed_reality_controller == xr_ext_active) {
		bind_arr.clear();
		_bind_rl(&bind_arr, local.action_grip_pose,   path_grip_pose  );
		_bind_rl(&bind_arr, local.action_aim_pose,    path_aim_pose   );
		_bind_rl(&bind_arr, local.action_trigger,     path_trigger_val);
		_bind_rl(&bind_arr, local.action_grip,        path_squeeze_val);
		_bind_rl(&bind_arr, local.action_stick_xy,    path_stick_xy   );
		_bind_rl(&bind_arr, local.action_stick_click, path_stick_click);
		_bind_rl(&bind_arr, local.action_menu,        path_menu_click );
		_bind_rl(&bind_arr, local.action_x1,          {path_x_click.left, path_a_click.right});
		_bind_rl(&bind_arr, local.action_x2,          {path_y_click.left, path_b_click.right});
		if (xr_ext.EXT_palm_pose == xr_ext_active) _bind_rl(&bind_arr, local.action_palm_pose, path_palm_pose);

		pose_t palm_offset = device_display_get_blend() == display_blend_opaque
			? pose_t{ {0.01f, -0.01f,  0.015f}, quat_from_angles(-45, 0, 0) }
			: pose_t{ {0,      0.005f, 0     }, quat_from_angles(-68, 0, 0) };
		if (_bind_suggest("hp/mixed_reality_controller", false, bind_arr, palm_offset, palm_offset, &bind_info))
			local.profiles.add(bind_info);
	}

	// ext/hand_interaction_ext
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#ext_hand_interaction_profile
	if (xr_ext.EXT_hand_interaction == xr_ext_active) {
		bind_arr.clear();
		_bind_rl(&bind_arr, local.action_grip_pose, path_grip_pose);
		_bind_rl(&bind_arr, local.action_aim_pose,  path_aim_pose );
		_bind_rl(&bind_arr, local.action_trigger,   path_pinch_val);
		_bind_rl(&bind_arr, local.action_aim_ready, path_pinch_ready);
		_bind_rl(&bind_arr, local.action_grip,      path_grasp_val);
		if (xr_ext.EXT_palm_pose == xr_ext_active) _bind_rl(&bind_arr, local.action_palm_pose, path_palm_pose);

		if (_bind_suggest("ext/hand_interaction_ext", true, bind_arr, pose_identity, pose_identity, &bind_info))
			local.profiles.add(bind_info);
	}

	// microsoft/hand_interaction
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#ext_hand_interaction_profile
	if (xr_ext.MSFT_hand_interaction == xr_ext_active) {
		bind_arr.clear();
		_bind_rl(&bind_arr, local.action_grip_pose, path_grip_pose);
		_bind_rl(&bind_arr, local.action_aim_pose,  path_aim_pose);
		_bind_rl(&bind_arr, local.action_trigger,   path_select_val);
		_bind_rl(&bind_arr, local.action_grip,      path_squeeze_val);
		if (xr_ext.EXT_palm_pose == xr_ext_active) _bind_rl(&bind_arr, local.action_palm_pose, path_palm_pose);

		if (_bind_suggest("microsoft/hand_interaction", true, bind_arr, pose_identity, pose_identity, &bind_info))
			local.profiles.add(bind_info);
	}

	// Bytedance PICO Neo3
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_BD_controller_interaction
	if (xr_ext.BD_controller_interaction == xr_ext_active) {
		bind_arr.clear();
		_bind_rl(&bind_arr, local.action_grip_pose,  path_grip_pose  );
		_bind_rl(&bind_arr, local.action_aim_pose,   path_aim_pose   );
		_bind_rl(&bind_arr, local.action_trigger,    path_trigger_val);
		_bind_rl(&bind_arr, local.action_grip,       path_squeeze_val);
		_bind_rl(&bind_arr, local.action_stick_xy,   path_stick_xy   );
		_bind_rl(&bind_arr, local.action_stick_click,path_stick_click);
		_bind_rl(&bind_arr, local.action_menu,       path_menu_click );
		_bind_rl(&bind_arr, local.action_x1,         {path_x_click.left, path_a_click.right});
		_bind_rl(&bind_arr, local.action_x2,         {path_y_click.left, path_b_click.right});
		if (xr_ext.EXT_palm_pose == xr_ext_active) _bind_rl(&bind_arr, local.action_palm_pose, path_palm_pose);

		pose_t palm_offset_left  = pose_t{ {-0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		pose_t palm_offset_right = pose_t{ { 0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		if (_bind_suggest("bytedance/pico_neo3_controller", false, bind_arr, palm_offset_left, palm_offset_right, &bind_info))
			local.profiles.add(bind_info);
	}

	// Bytedance Pico 4
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_BD_controller_interaction
	// Note that on the pico 4 OS 5.5 OpenXR SDK 2.2, the xrGetCurrentInteractionProfile will return '/interaction_profiles/bytedance/pico_neo3_controller'
	// instead of the expected '/interaction_profiles/bytedance/pico4_controller'
	if (xr_ext.BD_controller_interaction == xr_ext_active) {
		bind_arr.clear();
		_bind_rl(&bind_arr, local.action_grip_pose,  path_grip_pose  );
		_bind_rl(&bind_arr, local.action_aim_pose,   path_aim_pose   );
		_bind_rl(&bind_arr, local.action_trigger,    path_trigger_val);
		_bind_rl(&bind_arr, local.action_grip,       path_squeeze_val);
		_bind_rl(&bind_arr, local.action_stick_xy,   path_stick_xy   );
		_bind_rl(&bind_arr, local.action_stick_click,path_stick_click);
		_bind   (&bind_arr, local.action_menu,       path_menu_click.left);
		_bind_rl(&bind_arr, local.action_x1,         {path_x_click.left, path_a_click.right});
		_bind_rl(&bind_arr, local.action_x2,         {path_y_click.left, path_b_click.right});
		if (xr_ext.EXT_palm_pose == xr_ext_active) _bind_rl(&bind_arr, local.action_palm_pose, path_palm_pose);

		pose_t palm_offset_left  = pose_t{ {-0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		pose_t palm_offset_right = pose_t{ { 0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		if (_bind_suggest("bytedance/pico4_controller", false, bind_arr, palm_offset_left, palm_offset_right, &bind_info))
			local.profiles.add(bind_info);
	}

	// htc/vive_controller
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_htc_vive_controller_profile
	{
		bind_arr.clear();
		_bind_rl(&bind_arr, local.action_grip_pose,   path_grip_pose    );
		_bind_rl(&bind_arr, local.action_aim_pose,    path_aim_pose     );
		_bind_rl(&bind_arr, local.action_trigger,     path_trigger_val  );
		_bind_rl(&bind_arr, local.action_grip,        path_squeeze_click);
		_bind_rl(&bind_arr, local.action_menu,        path_menu_click   );
		if (xr_ext.EXT_palm_pose == xr_ext_active) _bind_rl(&bind_arr, local.action_palm_pose, path_palm_pose);

		pose_t palm_offset_left  = pose_t{ {-0.035f, 0, 0}, quat_from_angles(-40, 0, 0) };
		pose_t palm_offset_right = pose_t{ { 0.035f, 0, 0}, quat_from_angles(-40, 0, 0) };
		if (_bind_suggest("htc/vive_controller", false, bind_arr, palm_offset_left, palm_offset_right, &bind_info))
			local.profiles.add(bind_info);
	}

	// valve/index_controller
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_valve_index_controller_profile
	{
		bind_arr.clear();
		_bind_rl(&bind_arr, local.action_grip_pose,   path_grip_pose   );
		_bind_rl(&bind_arr, local.action_aim_pose,    path_aim_pose    );
		_bind_rl(&bind_arr, local.action_trigger,     path_trigger_val );
		_bind_rl(&bind_arr, local.action_grip,        path_squeeze_val );
		_bind_rl(&bind_arr, local.action_stick_xy,    path_stick_xy    );
		_bind_rl(&bind_arr, local.action_stick_click, path_stick_click );
		_bind_rl(&bind_arr, local.action_menu,        path_system_click);
		_bind_rl(&bind_arr, local.action_x1,          path_a_click     );
		_bind_rl(&bind_arr, local.action_x2,          path_b_click     );
		if (xr_ext.EXT_palm_pose == xr_ext_active) _bind_rl(&bind_arr, local.action_palm_pose, path_palm_pose);

		pose_t palm_offset_left  = pose_t{ {-0.035f, 0, 0}, quat_from_angles(-40, 0, 0) };
		pose_t palm_offset_right = pose_t{ { 0.035f, 0, 0}, quat_from_angles(-40, 0, 0) };
		if (_bind_suggest("valve/index_controller", false, bind_arr, palm_offset_left, palm_offset_right, &bind_info))
			local.profiles.add(bind_info);
	}

	// oculus/touch_controller
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_oculus_touch_controller_profile
	{
		bind_arr.clear();
		_bind_rl(&bind_arr, local.action_grip_pose,   path_grip_pose  );
		_bind_rl(&bind_arr, local.action_aim_pose,    path_aim_pose   );
		_bind_rl(&bind_arr, local.action_trigger,     path_trigger_val);
		_bind_rl(&bind_arr, local.action_grip,        path_squeeze_val);
		_bind_rl(&bind_arr, local.action_stick_xy,    path_stick_xy   );
		_bind_rl(&bind_arr, local.action_stick_click, path_stick_click);
		_bind   (&bind_arr, local.action_menu,        path_menu_click.left);
		_bind_rl(&bind_arr, local.action_x1,          {path_x_click.left, path_a_click.right} );
		_bind_rl(&bind_arr, local.action_x2,          {path_y_click.left, path_b_click.right} );
		if (xr_ext.EXT_palm_pose == xr_ext_active) _bind_rl(&bind_arr, local.action_palm_pose, path_palm_pose);

		pose_t palm_offset_left  = pose_t{ {-0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		pose_t palm_offset_right = pose_t{ { 0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		if (_bind_suggest("oculus/touch_controller", false, bind_arr, palm_offset_left, palm_offset_right, &bind_info))
			local.profiles.add(bind_info);
	}

	// khr/simple_controller
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_khronos_simple_controller_profile
	{
		bind_arr.clear();
		_bind_rl(&bind_arr, local.action_grip_pose,   path_grip_pose    );
		_bind_rl(&bind_arr, local.action_aim_pose,    path_aim_pose     );
		_bind_rl(&bind_arr, local.action_trigger,     path_select_click );
		_bind_rl(&bind_arr, local.action_menu,        path_menu_click   );
		if (xr_ext.EXT_palm_pose == xr_ext_active) _bind_rl(&bind_arr, local.action_palm_pose, path_palm_pose);

		if (_bind_suggest("khr/simple_controller", false, bind_arr, pose_identity, pose_identity, &bind_info))
			local.profiles.add(bind_info);
	}

	// Eye tracking
	if (device_has_eye_gaze()) {
		XrPath gaze_path;
		xrStringToPath(xr_instance, "/user/eyes_ext/input/gaze_ext/pose", &gaze_path);

		bind_arr.clear();
		_bind(&bind_arr, local.action_eyes, gaze_path);

		if (_bind_suggest("ext/eye_gaze_interaction", false, bind_arr, pose_identity, pose_identity, &bind_info)) {
			XrActionSpaceCreateInfo create_space = {XR_TYPE_ACTION_SPACE_CREATE_INFO};
			create_space.action            = local.action_eyes;
			create_space.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
			result = xrCreateActionSpace(xr_session, &create_space, &xr_gaze_space);
			if (XR_FAILED(result)) {
				log_warnf("Gaze xrCreateActionSpace failed: [%s]", openxr_string(result));
			}
		}
	}

	bind_arr.free();

	// Create frames of reference for the pose actions
	for (int32_t i = 0; i < 2; i++) {
		XrActionSpaceCreateInfo action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
		action_space_info.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
		action_space_info.subactionPath     = local.hand_subaction.paths[i];
		action_space_info.action            = local.action_grip_pose;
		result = xrCreateActionSpace(xr_session, &action_space_info, &local.space_grip[i]);
		if (XR_FAILED(result)) {
			log_errf("xrCreateActionSpace failed: [%s]", openxr_string(result));
			return false;
		}

		action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
		action_space_info.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
		action_space_info.subactionPath     = local.hand_subaction.paths[i];
		action_space_info.action            = local.action_aim_pose;
		result = xrCreateActionSpace(xr_session, &action_space_info, &local.space_aim[i]);
		if (XR_FAILED(result)) {
			log_errf("xrCreateActionSpace failed: [%s]", openxr_string(result));
			return false;
		}

		if (xr_ext.EXT_palm_pose == xr_ext_active) {
			action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
			action_space_info.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
			action_space_info.subactionPath     = local.hand_subaction.paths[i];
			action_space_info.action            = local.action_palm_pose;
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

void oxri_shutdown() {
	local.profiles.free();
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
		action_info.action = local.action_eyes;
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

void oxri_update_frame() {
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
		get_info.action = local.action_trigger;
		xrGetActionStateFloat(xr_session, &get_info, &state_trigger);
		controller->trigger = state_trigger.currentState;

		// Grip button
		XrActionStateFloat state_grip_btn = { XR_TYPE_ACTION_STATE_FLOAT };
		get_info.action = local.action_grip;
		xrGetActionStateFloat(xr_session, &get_info, &state_grip_btn);
		controller->grip = state_grip_btn.currentState;

		// Analog stick X and Y
		XrActionStateVector2f grip_stick_xy = { XR_TYPE_ACTION_STATE_VECTOR2F };
		get_info.action = local.action_stick_xy;
		xrGetActionStateVector2f(xr_session, &get_info, &grip_stick_xy);
		controller->stick.x = -grip_stick_xy.currentState.x;
		controller->stick.y =  grip_stick_xy.currentState.y;

		//// Bool actions

		// Menu button
		XrActionStateBoolean state_menu = { XR_TYPE_ACTION_STATE_BOOLEAN };
		get_info.action = local.action_menu;
		xrGetActionStateBoolean(xr_session, &get_info, &state_menu);
		menu_button = menu_button || state_menu.currentState;

		// Stick click
		XrActionStateBoolean state_stick_click = { XR_TYPE_ACTION_STATE_BOOLEAN };
		get_info.action = local.action_stick_click;
		xrGetActionStateBoolean(xr_session, &get_info, &state_stick_click);
		controller->stick_click = button_make_state(controller->stick_click & button_state_active, state_stick_click.currentState);

		// Button x1 and x2
		XrActionStateBoolean state_x1 = { XR_TYPE_ACTION_STATE_BOOLEAN };
		XrActionStateBoolean state_x2 = { XR_TYPE_ACTION_STATE_BOOLEAN };
		get_info.action = local.action_x1;
		xrGetActionStateBoolean(xr_session, &get_info, &state_x1);
		get_info.action = local.action_x2;
		xrGetActionStateBoolean(xr_session, &get_info, &state_x2);
		controller->x1 = button_make_state(controller->x1 & button_state_active, state_x1.currentState);
		controller->x2 = button_make_state(controller->x2 & button_state_active, state_x2.currentState);

		// Aim ready
		if (local.action_aim_ready != XR_NULL_HANDLE) {
			XrActionStateBoolean state_aim_ready = { XR_TYPE_ACTION_STATE_BOOLEAN };
			get_info.action = local.action_aim_ready;
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
		action_info.action = local.action_eyes;
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

XrRLPath _bind_paths(const char* name)
{
	XrRLPath result = {};
	char     buffer[256];
	snprintf(buffer, sizeof(buffer), "/user/hand/%s/input/%s", "left", name);
	XrResult xr = xrStringToPath(xr_instance, buffer, &result.left);
	if (XR_FAILED(xr)) {
		log_infof("xrStringToPath failed for '%s': [%s]", buffer, openxr_string(xr));
	}
	snprintf(buffer, sizeof(buffer), "/user/hand/%s/input/%s", "right", name);
	xr = xrStringToPath(xr_instance, buffer, &result.right);
	if (XR_FAILED(xr)) {
		log_infof("xrStringToPath failed for '%s': [%s]", buffer, openxr_string(xr));
	}
	return result;
}

///////////////////////////////////////////

void _bind_rl(array_t<XrActionSuggestedBinding>* arr, XrAction action, XrRLPath rlpath) {
	arr->add({ action, rlpath.left });
	arr->add({ action, rlpath.right });
}

///////////////////////////////////////////

void _bind(array_t<XrActionSuggestedBinding>* arr, XrAction action, XrPath path) {
	arr->add({ action, path });
}

///////////////////////////////////////////

bool _bind_suggest(const char *profile_name, bool is_hand, const array_t<XrActionSuggestedBinding> binding_arr, pose_t palm_left_offset, pose_t palm_right_offset, xrc_profile_info_t *out_profile) {
	*out_profile = {};

	XrPath interaction_path;
	char   buffer[256];
	snprintf(buffer, sizeof(buffer), "/interaction_profiles/%s", profile_name);
	xrStringToPath(xr_instance, buffer, &interaction_path);

	XrInteractionProfileSuggestedBinding suggested_binds = { XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
	suggested_binds.interactionProfile     = interaction_path;
	suggested_binds.suggestedBindings      = binding_arr.data;
	suggested_binds.countSuggestedBindings = binding_arr.count;
	if (XR_FAILED(xrSuggestInteractionProfileBindings(xr_instance, &suggested_binds)))
		return false;

	out_profile->profile              = interaction_path;
	out_profile->name                 = profile_name;
	out_profile->is_hand              = is_hand;
	out_profile->offset[handed_left ] = palm_left_offset;
	out_profile->offset[handed_right] = palm_right_offset;
	return true;
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