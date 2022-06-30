#include "../platforms/platform_utils.h"
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

struct xrc_actions_t {
};

XrActionSet xrc_action_set;
XrAction    xrc_action_eyes;
XrAction    xrc_action_pose_grip;
XrAction    xrc_action_pose_palm;
XrAction    xrc_action_pose_aim;
XrAction    xrc_action_trigger;
XrAction    xrc_action_grip;
XrAction    xrc_action_stick_x;
XrAction    xrc_action_stick_y;
XrAction    xrc_action_stick_click;
XrAction    xrc_action_x1;
XrAction    xrc_action_x2;
XrAction    xrc_action_menu;

XrPath      xrc_hand_subaction_path[2];
XrSpace     xrc_space_aim          [2];
XrSpace     xrc_space_grip         [2] = {};
XrSpace     xrc_space_palm         [2] = {};
XrSpace     xr_gaze_space              = {};

int32_t xr_eyes_pointer;

struct xrc_profile_info_t {
	const char *name;
	XrPath      profile;
	quat        offset_rot[2];
	vec3        offset_pos[2];
};
array_t<xrc_profile_info_t> xrc_profile_offsets = {};
XrPath                      xrc_active_profile[2] = { 0xFFFFFFFF, 0xFFFFFFFF };

void oxri_set_profile(handed_ hand, XrPath profile);

///////////////////////////////////////////

bool oxri_init() {
	XrPath                               profile_path;
	XrActionCreateInfo                   action_info     = { XR_TYPE_ACTION_CREATE_INFO };
	XrInteractionProfileSuggestedBinding suggested_binds = { XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };

	xrc_offset_pos[0] = vec3_zero;
	xrc_offset_pos[1] = vec3_zero;
	xrc_offset_rot[0] = quat_identity;
	xrc_offset_rot[1] = quat_identity;

	xr_eyes_pointer = input_add_pointer(input_source_gaze | (sk_info.eye_tracking_present ? input_source_gaze_eyes : input_source_gaze_head));

	XrActionSetCreateInfo actionset_info = { XR_TYPE_ACTION_SET_CREATE_INFO };
	snprintf(actionset_info.actionSetName,          sizeof(actionset_info.actionSetName),          "input");
	snprintf(actionset_info.localizedActionSetName, sizeof(actionset_info.localizedActionSetName), "Input");
	XrResult result = xrCreateActionSet(xr_instance, &actionset_info, &xrc_action_set);
	if (XR_FAILED(result)) {
		log_infof("xrCreateActionSet failed: [%s]", openxr_string(result));
		return false;
	}

	xrStringToPath(xr_instance, "/user/hand/left",  &xrc_hand_subaction_path[0]);
	xrStringToPath(xr_instance, "/user/hand/right", &xrc_hand_subaction_path[1]);

	// Create an action to track the position and orientation of the hands! This is
	// the controller location, or the center of the palms for actual hands.
	action_info.countSubactionPaths = _countof(xrc_hand_subaction_path);
	action_info.subactionPaths      = xrc_hand_subaction_path;
	action_info.actionType          = XR_ACTION_TYPE_POSE_INPUT;
	snprintf(action_info.actionName,          sizeof(action_info.actionName),          "grip_pose");
	snprintf(action_info.localizedActionName, sizeof(action_info.localizedActionName), "Grip Pose");
	result = xrCreateAction(xrc_action_set, &action_info, &xrc_action_pose_grip);
	if (XR_FAILED(result)) {
		log_infof("xrCreateAction failed: [%s]", openxr_string(result));
		return false;
	}

	// Create an action to track the pointing position and orientation!
	action_info.actionType = XR_ACTION_TYPE_POSE_INPUT;
	snprintf(action_info.actionName,          sizeof(action_info.actionName),          "aim_pose");
	snprintf(action_info.localizedActionName, sizeof(action_info.localizedActionName), "Aim Pose");
	result = xrCreateAction(xrc_action_set, &action_info, &xrc_action_pose_aim);
	if (XR_FAILED(result)) {
		log_infof("xrCreateAction failed: [%s]", openxr_string(result));
		return false;
	}

	// Create an action for listening to the select action! This is primary trigger
	// on controllers, and an airtap on HoloLens
	action_info.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
	snprintf(action_info.actionName,          sizeof(action_info.actionName),          "trigger");
	snprintf(action_info.localizedActionName, sizeof(action_info.localizedActionName), "Trigger");
	result = xrCreateAction(xrc_action_set, &action_info, &xrc_action_trigger);
	if (XR_FAILED(result)) {
		log_infof("xrCreateAction failed: [%s]", openxr_string(result));
		return false;
	}

	action_info.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
	snprintf(action_info.actionName,          sizeof(action_info.actionName),          "grip");
	snprintf(action_info.localizedActionName, sizeof(action_info.localizedActionName), "Grip");
	result = xrCreateAction(xrc_action_set, &action_info, &xrc_action_grip);
	if (XR_FAILED(result)) {
		log_infof("xrCreateAction failed: [%s]", openxr_string(result));
		return false;
	}

	// Stick axes for X and Y
	action_info.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
	snprintf(action_info.actionName,          sizeof(action_info.actionName),          "stick_x");
	snprintf(action_info.localizedActionName, sizeof(action_info.localizedActionName), "Stick X");
	result = xrCreateAction(xrc_action_set, &action_info, &xrc_action_stick_x);
	if (XR_FAILED(result)) {
		log_infof("xrCreateAction failed: [%s]", openxr_string(result));
		return false;
	}
	action_info.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
	snprintf(action_info.actionName,          sizeof(action_info.actionName),          "stick_y");
	snprintf(action_info.localizedActionName, sizeof(action_info.localizedActionName), "Stick Y");
	result = xrCreateAction(xrc_action_set, &action_info, &xrc_action_stick_y);
	if (XR_FAILED(result)) {
		log_infof("xrCreateAction failed: [%s]", openxr_string(result));
		return false;
	}

	// Button actions

	action_info.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
	snprintf(action_info.actionName,          sizeof(action_info.actionName),          "stick_click");
	snprintf(action_info.localizedActionName, sizeof(action_info.localizedActionName), "Stick Click");
	result = xrCreateAction(xrc_action_set, &action_info, &xrc_action_stick_click);
	if (XR_FAILED(result)) {
		log_infof("xrCreateAction failed: [%s]", openxr_string(result));
		return false;
	}

	action_info.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
	snprintf(action_info.actionName,          sizeof(action_info.actionName),          "menu");
	snprintf(action_info.localizedActionName, sizeof(action_info.localizedActionName), "Menu");
	result = xrCreateAction(xrc_action_set, &action_info, &xrc_action_menu);
	if (XR_FAILED(result)) {
		log_infof("xrCreateAction failed: [%s]", openxr_string(result));
		return false;
	}

	action_info.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
	snprintf(action_info.actionName,          sizeof(action_info.actionName),          "button_x1");
	snprintf(action_info.localizedActionName, sizeof(action_info.localizedActionName), "Button X1");
	result = xrCreateAction(xrc_action_set, &action_info, &xrc_action_x1);
	if (XR_FAILED(result)) {
		log_infof("xrCreateAction failed: [%s]", openxr_string(result));
		return false;
	}

	action_info.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
	snprintf(action_info.actionName,          sizeof(action_info.actionName),          "button_x2");
	snprintf(action_info.localizedActionName, sizeof(action_info.localizedActionName), "Button X2");
	result = xrCreateAction(xrc_action_set, &action_info, &xrc_action_x2);
	if (XR_FAILED(result)) {
		log_infof("xrCreateAction failed: [%s]", openxr_string(result));
		return false;
	}

	// Palm pose extension
	if (xr_ext_available.EXT_palm_pose) {
		action_info.actionType = XR_ACTION_TYPE_POSE_INPUT;
		snprintf(action_info.actionName,          sizeof(action_info.actionName),          "palm_pose");
		snprintf(action_info.localizedActionName, sizeof(action_info.localizedActionName), "Palm Pose");
		result = xrCreateAction(xrc_action_set, &action_info, &xrc_action_pose_palm);
		if (XR_FAILED(result)) {
			log_infof("xrCreateAction failed: [%s]", openxr_string(result));
			return false;
		}
	}

	// Eye tracking
	if (sk_info.eye_tracking_present) {
		action_info = { XR_TYPE_ACTION_CREATE_INFO };
		action_info.actionType = XR_ACTION_TYPE_POSE_INPUT;
		snprintf(action_info.actionName,          sizeof(action_info.actionName),          "eye_gaze");
		snprintf(action_info.localizedActionName, sizeof(action_info.localizedActionName), "Eye Gaze");
		result = xrCreateAction(xrc_action_set, &action_info, &xrc_action_eyes);
		if (XR_FAILED(result)) {
			log_warnf("xrCreateAction failed: [%s]", openxr_string(result));
			return false;
		}

		XrPath gaze_path;
		xrStringToPath(xr_instance, "/user/eyes_ext/input/gaze_ext/pose", &gaze_path);
		XrActionSuggestedBinding bindings = {xrc_action_eyes, gaze_path};

		xrStringToPath(xr_instance, "/interaction_profiles/ext/eye_gaze_interaction", &profile_path);
		suggested_binds.interactionProfile     = profile_path;
		suggested_binds.suggestedBindings      = &bindings;
		suggested_binds.countSuggestedBindings = 1;
		result = xrSuggestInteractionProfileBindings(xr_instance, &suggested_binds);
		if (XR_FAILED(result)) {
			log_warnf("Gaze xrSuggestInteractionProfileBindings failed: [%s]", openxr_string(result));
		}

		XrActionSpaceCreateInfo create_space = {XR_TYPE_ACTION_SPACE_CREATE_INFO};
		create_space.action            = xrc_action_eyes;
		create_space.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
		result = xrCreateActionSpace(xr_session, &create_space, &xr_gaze_space);
		if (XR_FAILED(result)) {
			log_warnf("Gaze xrCreateActionSpace failed: [%s]", openxr_string(result));
		}
	}

	// Bind the actions we just created to specific locations on the Khronos simple_controller
	// definition! These are labeled as 'suggested' because they may be overridden by the runtime
	// preferences. For example, if the runtime allows you to remap buttons, or provides input
	// accessibility settings.
	XrPath path_pose_palm  [2];
	XrPath path_pose_aim   [2];
	XrPath path_pose_grip  [2];
	XrPath path_trigger    [2];
	XrPath path_grip       [2];
	XrPath path_stick_x    [2];
	XrPath path_stick_y    [2];
	XrPath path_stick_click[2];
	XrPath path_x1         [2];
	XrPath path_x2         [2];
	XrPath path_menu       [2];
	xrStringToPath(xr_instance, "/user/hand/left/input/grip/pose",  &path_pose_grip[0]);
	xrStringToPath(xr_instance, "/user/hand/right/input/grip/pose", &path_pose_grip[1]);
	xrStringToPath(xr_instance, "/user/hand/left/input/aim/pose",   &path_pose_aim[0]);
	xrStringToPath(xr_instance, "/user/hand/right/input/aim/pose",  &path_pose_aim[1]);
	xrStringToPath(xr_instance, "/user/hand/left/input/squeeze/value",  &path_grip[0]);
	xrStringToPath(xr_instance, "/user/hand/right/input/squeeze/value", &path_grip[1]);

	xrStringToPath(xr_instance, "/user/hand/left/input/thumbstick/x",      &path_stick_x[0]);
	xrStringToPath(xr_instance, "/user/hand/right/input/thumbstick/x",     &path_stick_x[1]);
	xrStringToPath(xr_instance, "/user/hand/left/input/thumbstick/y",      &path_stick_y[0]);
	xrStringToPath(xr_instance, "/user/hand/right/input/thumbstick/y",     &path_stick_y[1]);
	xrStringToPath(xr_instance, "/user/hand/left/input/thumbstick/click",  &path_stick_click[0]);
	xrStringToPath(xr_instance, "/user/hand/right/input/thumbstick/click", &path_stick_click[1]);
	xrStringToPath(xr_instance, "/user/hand/left/input/menu/click",        &path_menu[0]);
	xrStringToPath(xr_instance, "/user/hand/right/input/menu/click",       &path_menu[1]);

	if (xr_ext_available.EXT_palm_pose) {
		xrStringToPath(xr_instance, "/user/hand/left/input/palm_ext/pose",  &path_pose_palm[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/palm_ext/pose", &path_pose_palm[1]);
	}

	array_t<XrActionSuggestedBinding> binding_arr = {};

	{ // microsoft/motion_controller
		xrStringToPath(xr_instance, "/user/hand/left/input/trigger/value",  &path_trigger[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/trigger/value", &path_trigger[1]);

		XrPath wmr_grip[2];
		xrStringToPath(xr_instance, "/user/hand/left/input/squeeze/click",  &wmr_grip[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/squeeze/click", &wmr_grip[1]);
		XrActionSuggestedBinding bindings[] = {
			{ xrc_action_pose_grip,  path_pose_grip  [0] }, { xrc_action_pose_grip,   path_pose_grip  [1] },
			{ xrc_action_pose_aim,   path_pose_aim   [0] }, { xrc_action_pose_aim,    path_pose_aim   [1] },
			{ xrc_action_trigger,    path_trigger    [0] }, { xrc_action_trigger,     path_trigger    [1] },
			{ xrc_action_grip,       wmr_grip        [0] }, { xrc_action_grip,        wmr_grip        [1] },
			{ xrc_action_stick_x,    path_stick_x    [0] }, { xrc_action_stick_x,     path_stick_x    [1] },
			{ xrc_action_stick_y,    path_stick_y    [0] }, { xrc_action_stick_y,     path_stick_y    [1] },
			{ xrc_action_stick_click,path_stick_click[0] }, { xrc_action_stick_click, path_stick_click[1] },
			{ xrc_action_menu,       path_menu       [0] }, { xrc_action_menu,        path_menu       [1] },
		};
		binding_arr.add_range(bindings, _countof(bindings));
		if (xr_ext_available.EXT_palm_pose) {
			binding_arr.add({ xrc_action_pose_palm, path_pose_palm[0] });
			binding_arr.add({ xrc_action_pose_palm, path_pose_palm[1] });
		}

		xrStringToPath(xr_instance, "/interaction_profiles/microsoft/motion_controller", &profile_path);
		suggested_binds.interactionProfile     = profile_path;
		suggested_binds.suggestedBindings      = binding_arr.data;
		suggested_binds.countSuggestedBindings = binding_arr.count;
		if (XR_SUCCEEDED(xrSuggestInteractionProfileBindings(xr_instance, &suggested_binds))) {
			// Orientation fix for WMR vs. HoloLens controllers
			xrc_profile_info_t info;
			info.profile = profile_path;
			info.name    = "microsoft/motion_controller";
			if (sk_info.display_type == display_opaque) {
				info.offset_rot[handed_left ] = quat_from_angles(-45, 0, 0);
				info.offset_rot[handed_right] = quat_from_angles(-45, 0, 0);
				info.offset_pos[handed_left ] = { 0.01f, -0.01f, 0.015f };
				info.offset_pos[handed_right] = { 0.01f, -0.01f, 0.015f };
			} else {
				info.offset_rot[handed_left ] = quat_from_angles(-68, 0, 0);
				info.offset_rot[handed_right] = quat_from_angles(-68, 0, 0);
				info.offset_pos[handed_left ] = { 0, 0.005f, 0 };
				info.offset_pos[handed_right] = { 0, 0.005f, 0 };
			}
			xrc_profile_offsets.add(info);
		}
		binding_arr.clear();
	}

	if (xr_ext_available.EXT_hp_mixed_reality_controller)
	{ // hp/mixed_reality_controller
		xrStringToPath(xr_instance, "/user/hand/left/input/x/click",        &path_x1[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/a/click",       &path_x1[1]);
		xrStringToPath(xr_instance, "/user/hand/left/input/y/click",        &path_x2[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/b/click",       &path_x2[1]);

		xrStringToPath(xr_instance, "/user/hand/left/input/trigger/value",  &path_trigger[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/trigger/value", &path_trigger[1]);
		XrActionSuggestedBinding bindings[] = {
			{ xrc_action_pose_grip,  path_pose_grip  [0] }, { xrc_action_pose_grip,   path_pose_grip  [1] },
			{ xrc_action_pose_aim,   path_pose_aim   [0] }, { xrc_action_pose_aim,    path_pose_aim   [1] },
			{ xrc_action_trigger,    path_trigger    [0] }, { xrc_action_trigger,     path_trigger    [1] },
			{ xrc_action_grip,       path_grip       [0] }, { xrc_action_grip,        path_grip       [1] },
			{ xrc_action_stick_x,    path_stick_x    [0] }, { xrc_action_stick_x,     path_stick_x    [1] },
			{ xrc_action_stick_y,    path_stick_y    [0] }, { xrc_action_stick_y,     path_stick_y    [1] },
			{ xrc_action_stick_click,path_stick_click[0] }, { xrc_action_stick_click, path_stick_click[1] },
			{ xrc_action_menu,       path_menu       [0] }, { xrc_action_menu,        path_menu       [1] },
			{ xrc_action_x1,         path_x1         [0] }, { xrc_action_x1,          path_x1         [1] },
			{ xrc_action_x2,         path_x2         [0] }, { xrc_action_x2,          path_x2         [1] },
		};
		binding_arr.add_range(bindings, _countof(bindings));
		if (xr_ext_available.EXT_palm_pose) {
			binding_arr.add({ xrc_action_pose_palm, path_pose_palm[0] });
			binding_arr.add({ xrc_action_pose_palm, path_pose_palm[1] });
		}

		xrStringToPath(xr_instance, "/interaction_profiles/hp/mixed_reality_controller", &profile_path);
		suggested_binds.interactionProfile     = profile_path;
		suggested_binds.suggestedBindings      = binding_arr.data;
		suggested_binds.countSuggestedBindings = binding_arr.count;
		if (XR_SUCCEEDED(xrSuggestInteractionProfileBindings(xr_instance, &suggested_binds))) {
			// Orientation fix for WMR vs. HoloLens controllers
			xrc_profile_info_t info;
			info.profile = profile_path;
			info.name    = "hp/mixed_reality_controller";
			if (sk_info.display_type == display_opaque) {
				info.offset_rot[handed_left ] = quat_from_angles(-45, 0, 0);
				info.offset_rot[handed_right] = quat_from_angles(-45, 0, 0);
				info.offset_pos[handed_left ] = { 0.01f, -0.01f, 0.015f };
				info.offset_pos[handed_right] = { 0.01f, -0.01f, 0.015f };
			} else {
				info.offset_rot[handed_left ] = quat_from_angles(-68, 0, 0);
				info.offset_rot[handed_right] = quat_from_angles(-68, 0, 0);
				info.offset_pos[handed_left ] = { 0, 0.005f, 0 };
				info.offset_pos[handed_right] = { 0, 0.005f, 0 };
			}
			xrc_profile_offsets.add(info);
		}
		binding_arr.clear();
	}

	{ // htc/vive_controller
		xrStringToPath(xr_instance, "/user/hand/left/input/trigger/value",  &path_trigger[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/trigger/value", &path_trigger[1]);

		XrPath vive_grip[2];
		xrStringToPath(xr_instance, "/user/hand/left/input/squeeze/click",  &vive_grip[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/squeeze/click", &vive_grip[1]);
		XrActionSuggestedBinding bindings[] = {
			{ xrc_action_pose_grip,  path_pose_grip  [0] }, { xrc_action_pose_grip,   path_pose_grip  [1] },
			{ xrc_action_pose_aim,   path_pose_aim   [0] }, { xrc_action_pose_aim,    path_pose_aim   [1] },
			{ xrc_action_trigger,    path_trigger    [0] }, { xrc_action_trigger,     path_trigger    [1] },
			{ xrc_action_grip,       vive_grip       [0] }, { xrc_action_grip,        vive_grip       [1] },
			{ xrc_action_menu,       path_menu       [0] }, { xrc_action_menu,        path_menu       [1] },
		};
		binding_arr.add_range(bindings, _countof(bindings));
		if (xr_ext_available.EXT_palm_pose) {
			binding_arr.add({ xrc_action_pose_palm, path_pose_palm[0] });
			binding_arr.add({ xrc_action_pose_palm, path_pose_palm[1] });
		}

		xrStringToPath(xr_instance, "/interaction_profiles/htc/vive_controller", &profile_path);
		suggested_binds.interactionProfile     = profile_path;
		suggested_binds.suggestedBindings      = binding_arr.data;
		suggested_binds.countSuggestedBindings = binding_arr.count;
		if (XR_SUCCEEDED(xrSuggestInteractionProfileBindings(xr_instance, &suggested_binds))) {
			// Orientation fix for HTC Vive controllers
			xrc_profile_info_t info;
			info.profile = profile_path;
			info.name    = "htc/vive_controller";
			info.offset_rot[handed_left ] = quat_from_angles(-40, 0, 0);
			info.offset_rot[handed_right] = quat_from_angles(-40, 0, 0);
			info.offset_pos[handed_left ] = {-0.035f, -0.00f, 0.00f };
			info.offset_pos[handed_right] = {0.035f, -0.00f, 0.00f };
			xrc_profile_offsets.add(info);
		}
		binding_arr.clear();
	}

	{ // valve/index_controller
		xrStringToPath(xr_instance, "/user/hand/left/input/a/click",        &path_x1[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/a/click",       &path_x1[1]);
		xrStringToPath(xr_instance, "/user/hand/left/input/b/click",        &path_x2[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/b/click",       &path_x2[1]);

		xrStringToPath(xr_instance, "/user/hand/left/input/trigger/value",  &path_trigger[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/trigger/value", &path_trigger[1]);

		XrPath index_menu[2];
		xrStringToPath(xr_instance, "/user/hand/left/input/system/click",  &index_menu[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/system/click", &index_menu[1]);
		XrActionSuggestedBinding bindings[] = {
			{ xrc_action_pose_grip,  path_pose_grip  [0] }, { xrc_action_pose_grip,   path_pose_grip  [1] },
			{ xrc_action_pose_aim,   path_pose_aim   [0] }, { xrc_action_pose_aim,    path_pose_aim   [1] },
			{ xrc_action_trigger,    path_trigger    [0] }, { xrc_action_trigger,     path_trigger    [1] },
			{ xrc_action_grip,       path_grip       [0] }, { xrc_action_grip,        path_grip       [1] },
			{ xrc_action_stick_x,    path_stick_x    [0] }, { xrc_action_stick_x,     path_stick_x    [1] },
			{ xrc_action_stick_y,    path_stick_y    [0] }, { xrc_action_stick_y,     path_stick_y    [1] },
			{ xrc_action_stick_click,path_stick_click[0] }, { xrc_action_stick_click, path_stick_click[1] },
			{ xrc_action_menu,       index_menu      [0] }, { xrc_action_menu,        index_menu      [1] },
			{ xrc_action_x1,         path_x1         [0] }, { xrc_action_x1,          path_x1         [1] },
			{ xrc_action_x2,         path_x2         [0] }, { xrc_action_x2,          path_x2         [1] },
		};
		binding_arr.add_range(bindings, _countof(bindings));
		if (xr_ext_available.EXT_palm_pose) {
			binding_arr.add({ xrc_action_pose_palm, path_pose_palm[0] });
			binding_arr.add({ xrc_action_pose_palm, path_pose_palm[1] });
		}

		xrStringToPath(xr_instance, "/interaction_profiles/valve/index_controller", &profile_path);
		suggested_binds.interactionProfile     = profile_path;
		suggested_binds.suggestedBindings      = binding_arr.data;
		suggested_binds.countSuggestedBindings = binding_arr.count;
		if (XR_SUCCEEDED(xrSuggestInteractionProfileBindings(xr_instance, &suggested_binds))) {
			// Orientation fix for Valve Index controllers
			xrc_profile_info_t info;
			info.profile = profile_path;
			info.name    = "valve/index_controller";
			info.offset_rot[handed_left ] = quat_from_angles(-40, 0, 0);
			info.offset_rot[handed_right] = quat_from_angles(-40, 0, 0);
			info.offset_pos[handed_left ] = {-0.035f, -0.00f, 0.00f };
			info.offset_pos[handed_right] = { 0.035f, -0.00f, 0.00f };
			xrc_profile_offsets.add(info);
		}
		binding_arr.clear();
	}

	{ // oculus/touch_controller
		xrStringToPath(xr_instance, "/user/hand/left/input/x/click",        &path_x1[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/a/click",       &path_x1[1]);
		xrStringToPath(xr_instance, "/user/hand/left/input/y/click",        &path_x2[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/b/click",       &path_x2[1]);

		xrStringToPath(xr_instance, "/user/hand/left/input/trigger/value",  &path_trigger[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/trigger/value", &path_trigger[1]);
		XrActionSuggestedBinding bindings[] = {
			{ xrc_action_pose_grip,  path_pose_grip  [0] }, { xrc_action_pose_grip,   path_pose_grip  [1] },
			{ xrc_action_pose_aim,   path_pose_aim   [0] }, { xrc_action_pose_aim,    path_pose_aim   [1] },
			{ xrc_action_trigger,    path_trigger    [0] }, { xrc_action_trigger,     path_trigger    [1] },
			{ xrc_action_grip,       path_grip       [0] }, { xrc_action_grip,        path_grip       [1] },
			{ xrc_action_stick_x,    path_stick_x    [0] }, { xrc_action_stick_x,     path_stick_x    [1] },
			{ xrc_action_stick_y,    path_stick_y    [0] }, { xrc_action_stick_y,     path_stick_y    [1] },
			{ xrc_action_stick_click,path_stick_click[0] }, { xrc_action_stick_click, path_stick_click[1] },
			{ xrc_action_menu,       path_menu       [0] },
			{ xrc_action_x1,         path_x1         [0] }, { xrc_action_x1,          path_x1         [1] },
			{ xrc_action_x2,         path_x2         [0] }, { xrc_action_x2,          path_x2         [1] },
		};
		binding_arr.add_range(bindings, _countof(bindings));
		if (xr_ext_available.EXT_palm_pose) {
			binding_arr.add({ xrc_action_pose_palm, path_pose_palm[0] });
			binding_arr.add({ xrc_action_pose_palm, path_pose_palm[1] });
		}

		xrStringToPath(xr_instance, "/interaction_profiles/oculus/touch_controller", &profile_path);
		suggested_binds.interactionProfile     = profile_path;
		suggested_binds.suggestedBindings      = binding_arr.data;
		suggested_binds.countSuggestedBindings = binding_arr.count;
		if (XR_SUCCEEDED(xrSuggestInteractionProfileBindings(xr_instance, &suggested_binds))) {
			// Orientation fix for oculus touch controllers
			xrc_profile_info_t info;
			info.profile = profile_path;
			info.name    = "oculus/touch_controller";
			info.offset_rot[handed_left ] = quat_from_angles(-80, 0, 0);
			info.offset_rot[handed_right] = quat_from_angles(-80, 0, 0);
			info.offset_pos[handed_left ] = {-0.03f, 0.01f, 0.00f };
			info.offset_pos[handed_right] = { 0.03f, 0.01f, 0.00f };
			xrc_profile_offsets.add(info);
		}
		binding_arr.clear();
	}

#if !defined(SK_OS_ANDROID)
	{ // khr/simple_controller
		xrStringToPath(xr_instance, "/user/hand/left/input/select/click",  &path_trigger[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/select/click", &path_trigger[1]);
		XrActionSuggestedBinding bindings[] = {
			{ xrc_action_pose_grip, path_pose_grip[0] }, { xrc_action_pose_grip, path_pose_grip[1] },
			{ xrc_action_pose_aim,  path_pose_aim [0] }, { xrc_action_pose_aim,  path_pose_aim [1] },
			{ xrc_action_trigger,   path_trigger  [0] }, { xrc_action_trigger,   path_trigger  [1] },
			{ xrc_action_menu,      path_menu     [0] }, { xrc_action_menu,      path_menu     [1] },
		};
		binding_arr.add_range(bindings, _countof(bindings));
		if (xr_ext_available.EXT_palm_pose) {
			binding_arr.add({ xrc_action_pose_palm, path_pose_palm[0] });
			binding_arr.add({ xrc_action_pose_palm, path_pose_palm[1] });
		}

		xrStringToPath(xr_instance, "/interaction_profiles/khr/simple_controller", &profile_path);
		suggested_binds.interactionProfile     = profile_path;
		suggested_binds.suggestedBindings      = binding_arr.data;
		suggested_binds.countSuggestedBindings = binding_arr.count;
		if (XR_SUCCEEDED(xrSuggestInteractionProfileBindings(xr_instance, &suggested_binds))) {
			xrc_profile_info_t info;
			info.profile = profile_path;
			info.name    = "khr/simple_controller";
			info.offset_rot[handed_left ] = quat_identity;
			info.offset_rot[handed_right] = quat_identity;
			info.offset_pos[handed_left ] = vec3_zero;
			info.offset_pos[handed_right] = vec3_zero;
			xrc_profile_offsets.add(info);
		}
		binding_arr.clear();
	}
#endif

	binding_arr.free();

	// Create frames of reference for the pose actions
	for (int32_t i = 0; i < 2; i++) {
		XrActionSpaceCreateInfo action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
		action_space_info.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
		action_space_info.subactionPath     = xrc_hand_subaction_path[i];
		action_space_info.action            = xrc_action_pose_grip;
		result = xrCreateActionSpace(xr_session, &action_space_info, &xrc_space_grip[i]);
		if (XR_FAILED(result)) {
			log_errf("xrCreateActionSpace failed: [%s]", openxr_string(result));
			return false;
		}

		action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
		action_space_info.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
		action_space_info.subactionPath     = xrc_hand_subaction_path[i];
		action_space_info.action            = xrc_action_pose_aim;
		result = xrCreateActionSpace(xr_session, &action_space_info, &xrc_space_aim[i]);
		if (XR_FAILED(result)) {
			log_errf("xrCreateActionSpace failed: [%s]", openxr_string(result));
			return false;
		}

		if (xr_ext_available.EXT_palm_pose) {
			action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
			action_space_info.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
			action_space_info.subactionPath     = xrc_hand_subaction_path[i];
			action_space_info.action            = xrc_action_pose_palm;
			result = xrCreateActionSpace(xr_session, &action_space_info, &xrc_space_palm[i]);
			if (XR_FAILED(result)) {
				log_errf("xrCreateActionSpace failed: [%s]", openxr_string(result));
				return false;
			}
		}
	}

	// Attach the action set we just made to the session
	XrSessionActionSetsAttachInfo attach_info = { XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
	attach_info.countActionSets = 1;
	attach_info.actionSets      = &xrc_action_set;
	result = xrAttachSessionActionSets(xr_session, &attach_info);
	if (XR_FAILED(result)) {
		log_errf("xrAttachSessionActionSets failed: [%s]", openxr_string(result));
		return false;
	}

	oxri_set_profile(handed_left,  xrc_active_profile[handed_left ]);
	oxri_set_profile(handed_right, xrc_active_profile[handed_right]);
	return true;
}

///////////////////////////////////////////

void oxri_shutdown() {
	xrc_profile_offsets.free();
	if (xrc_space_grip[0]) { xrDestroySpace    (xrc_space_grip[0]); xrc_space_grip[0] = {}; }
	if (xrc_space_grip[1]) { xrDestroySpace    (xrc_space_grip[1]); xrc_space_grip[1] = {}; }
	if (xrc_action_set   ) { xrDestroyActionSet(xrc_action_set   ); xrc_action_set    = {}; }
}

///////////////////////////////////////////

void oxri_update_poses() {
	// We occasionally need to update poses multiple times per frame, to
	// account for camera movement, or predicted time updates. This code is 
	// called separate from button press and tracking events to prevent
	// issues with hiding 'just_active/inactive' events.

	// Update our action set with up-to-date input data!
	XrActiveActionSet action_set = { };
	action_set.actionSet     = xrc_action_set;
	action_set.subactionPath = XR_NULL_PATH;
	XrActionsSyncInfo sync_info = { XR_TYPE_ACTIONS_SYNC_INFO };
	sync_info.countActiveActionSets = 1;
	sync_info.activeActionSets      = &action_set;
	xrSyncActions(xr_session, &sync_info);

	matrix root   = render_get_cam_final    ();
	quat   root_q = matrix_extract_rotation(root);

	// Track the head location
	openxr_get_space(xr_head_space, &input_head_pose_local);
	input_head_pose_world = matrix_transform_pose(root, input_head_pose_local);

	// Get input from whatever controllers may be present
	for (uint32_t hand = 0; hand < handed_max; hand++) {
		XrActionStateGetInfo get_info = { XR_TYPE_ACTION_STATE_GET_INFO };
		get_info.subactionPath = xrc_hand_subaction_path[hand];

		//// Pose actions

		// Controller grip pose
		XrActionStatePose state_grip = { XR_TYPE_ACTION_STATE_POSE };
		get_info.action = xrc_action_pose_grip;
		xrGetActionStatePose(xr_session, &get_info, &state_grip);

		// Get the grip pose the verbose way, since we want to know if
		// rotation or position are tracked independently of eachother.
		XrSpaceLocation space_location = { XR_TYPE_SPACE_LOCATION };
		XrResult        res            = xrLocateSpace(xrc_space_grip[hand], xr_app_space, xr_time, &space_location);
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
				input_controllers[hand].pose.position = root * local_pos;
			}
			if (valid_rot) {
				quat local_rot;
				memcpy(&local_rot, &space_location.pose.orientation, sizeof(quat));
				input_controllers[hand].pose.orientation = local_rot * root_q;
			}
			input_controllers[hand].tracked_pos = tracked_pos ? track_state_known : (valid_pos ? track_state_inferred : track_state_lost);
			input_controllers[hand].tracked_rot = tracked_rot ? track_state_known : (valid_rot ? track_state_inferred : track_state_lost);
		}

		// Get the palm position from either the extension, or our previous
		// fallback offsets from the grip pose.
		if (xr_ext_available.EXT_palm_pose) {
			space_location = { XR_TYPE_SPACE_LOCATION };
			XrResult res = xrLocateSpace(xrc_space_palm[hand], xr_app_space, xr_time, &space_location);
			if (XR_UNQUALIFIED_SUCCESS(res)) {
				pose_t local_palm;
				if (space_location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) {
					memcpy(&local_palm.position, &space_location.pose.position, sizeof(vec3));
					input_controllers[hand].palm.position = root * local_palm.position;
				}
				if (space_location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) {
					memcpy(&local_palm.orientation, &space_location.pose.orientation, sizeof(quat));
					input_controllers[hand].palm.orientation = local_palm.orientation * root_q;
				}
			}
		} else {
			input_controllers[hand].palm.orientation = xrc_offset_rot[hand] * input_controllers[hand].pose.orientation;
			input_controllers[hand].palm.position    = input_controllers[hand].pose.position + input_controllers[hand].palm.orientation * xrc_offset_pos[hand];
		}

		// Controller aim pose
		XrActionStatePose state_aim = { XR_TYPE_ACTION_STATE_POSE };
		get_info.action = xrc_action_pose_aim;
		xrGetActionStatePose(xr_session, &get_info, &state_aim);
		pose_t local_aim;
		openxr_get_space(xrc_space_aim[hand], &local_aim);
		input_controllers[hand].aim = { root * local_aim.position, local_aim.orientation * root_q };
	}

	// eye input
	if (sk_info.eye_tracking_present) {
		pointer_t           *pointer     = input_get_pointer(xr_eyes_pointer);
		XrActionStatePose    action_pose = {XR_TYPE_ACTION_STATE_POSE};
		XrActionStateGetInfo action_info = {XR_TYPE_ACTION_STATE_GET_INFO};
		action_info.action = xrc_action_eyes;
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
		XrActionStateGetInfo get_info = { XR_TYPE_ACTION_STATE_GET_INFO };
		get_info.subactionPath = xrc_hand_subaction_path[hand];

		//// Pose actions

		input_controllers[hand].tracked = button_make_state(
			input_controllers[hand].tracked & button_state_active,
			(input_controllers[hand].tracked_pos != track_state_lost ||
			 input_controllers[hand].tracked_rot != track_state_lost));

		//// Float actions

		// Trigger
		XrActionStateFloat state_trigger = { XR_TYPE_ACTION_STATE_FLOAT };
		get_info.action = xrc_action_trigger;
		xrGetActionStateFloat(xr_session, &get_info, &state_trigger);
		input_controllers[hand].trigger = state_trigger.currentState;

		// Grip button
		XrActionStateFloat state_grip_btn = { XR_TYPE_ACTION_STATE_FLOAT };
		get_info.action = xrc_action_grip;
		xrGetActionStateFloat(xr_session, &get_info, &state_grip_btn);
		input_controllers[hand].grip = state_grip_btn.currentState;

		// Analog stick X and Y
		XrActionStateFloat grip_stick_x = { XR_TYPE_ACTION_STATE_FLOAT };
		XrActionStateFloat grip_stick_y = { XR_TYPE_ACTION_STATE_FLOAT };
		get_info.action = xrc_action_stick_x;
		xrGetActionStateFloat(xr_session, &get_info, &grip_stick_x);
		get_info.action = xrc_action_stick_y;
		xrGetActionStateFloat(xr_session, &get_info, &grip_stick_y);
		input_controllers[hand].stick.x = -grip_stick_x.currentState;
		input_controllers[hand].stick.y =  grip_stick_y.currentState;

		//// Bool actions

		// Menu button
		XrActionStateBoolean state_menu = { XR_TYPE_ACTION_STATE_BOOLEAN };
		get_info.action = xrc_action_menu;
		xrGetActionStateBoolean(xr_session, &get_info, &state_menu);
		menu_button = menu_button || state_menu.currentState;

		// Stick click
		XrActionStateBoolean state_stick_click = { XR_TYPE_ACTION_STATE_BOOLEAN };
		get_info.action = xrc_action_stick_click;
		xrGetActionStateBoolean(xr_session, &get_info, &state_stick_click);
		input_controllers[hand].stick_click = button_make_state(input_controllers[hand].stick_click & button_state_active, state_stick_click.currentState);

		// Button x1 and x2
		XrActionStateBoolean state_x1 = { XR_TYPE_ACTION_STATE_BOOLEAN };
		XrActionStateBoolean state_x2 = { XR_TYPE_ACTION_STATE_BOOLEAN };
		get_info.action = xrc_action_x1;
		xrGetActionStateBoolean(xr_session, &get_info, &state_x1);
		get_info.action = xrc_action_x2;
		xrGetActionStateBoolean(xr_session, &get_info, &state_x2);
		input_controllers[hand].x1 = button_make_state(input_controllers[hand].x1 & button_state_active, state_x1.currentState);
		input_controllers[hand].x2 = button_make_state(input_controllers[hand].x2 & button_state_active, state_x2.currentState);
	}
	input_controller_menubtn = button_make_state(input_controller_menubtn & button_state_active, menu_button);

	// eye input
	if (sk_info.eye_tracking_present) {
		pointer_t           *pointer     = input_get_pointer(xr_eyes_pointer);
		XrActionStatePose    action_pose = {XR_TYPE_ACTION_STATE_POSE};
		XrActionStateGetInfo action_info = {XR_TYPE_ACTION_STATE_GET_INFO};
		action_info.action = xrc_action_eyes;
		xrGetActionStatePose(xr_session, &action_info, &action_pose);

		input_eyes_track_state = button_make_state(input_eyes_track_state & button_state_active, action_pose.isActive);
		pointer->tracked = input_eyes_track_state;
	}
}

///////////////////////////////////////////

void oxri_set_profile(handed_ hand, XrPath profile) {
	xrc_active_profile[hand] = profile;
	for (int32_t i = 0; i < xrc_profile_offsets.count; i++) {
		if (xrc_profile_offsets[i].profile == profile) {
			xrc_offset_pos[hand] = xrc_profile_offsets[i].offset_pos[hand];
			xrc_offset_rot[hand] = xrc_profile_offsets[i].offset_rot[hand];
			log_diagf("Switched %s controller profile to %s", hand == handed_left ? "left" : "right", xrc_profile_offsets[i].name);
			break;
		}
	}
}

///////////////////////////////////////////

void oxri_update_interaction_profile() {
	XrPath path[2];
	xrStringToPath(xr_instance, "/user/hand/left",  &path[handed_left]);
	xrStringToPath(xr_instance, "/user/hand/right", &path[handed_right]);

	XrInteractionProfileState active_profile = { XR_TYPE_INTERACTION_PROFILE_STATE };
	for (int32_t h = 0; h < handed_max; h++) {
		if (XR_FAILED(xrGetCurrentInteractionProfile(xr_session, path[h], &active_profile)))
			continue;
		if (active_profile.interactionProfile != xrc_active_profile[h])
			oxri_set_profile((handed_)h, active_profile.interactionProfile);
	}
}

} // namespace sk
#endif