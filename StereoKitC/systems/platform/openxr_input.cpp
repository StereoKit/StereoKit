
#include "openxr.h"
#include "openxr_input.h"

#include "../../stereokit.h"
#include "../../_stereokit.h"

#include <openxr/openxr.h>

namespace sk {

///////////////////////////////////////////

XrActionSet xrc_action_set;
XrAction    xrc_pose_action;
XrAction    xrc_point_action;
XrAction    xrc_select_action;
XrAction    xrc_grip_action;
XrPath      xrc_hand_subaction_path[2];
XrSpace     xrc_point_space[2];
XrSpace     xr_hand_space[2] = {};

///////////////////////////////////////////

bool oxri_init() {
	XrActionSetCreateInfo actionset_info = { XR_TYPE_ACTION_SET_CREATE_INFO };
	strcpy_s(actionset_info.actionSetName,          "input");
	strcpy_s(actionset_info.localizedActionSetName, "Input");
	XrResult result = xrCreateActionSet(xr_instance, &actionset_info, &xrc_action_set);
	xrStringToPath(xr_instance, "/user/hand/left",  &xrc_hand_subaction_path[0]);
	xrStringToPath(xr_instance, "/user/hand/right", &xrc_hand_subaction_path[1]);
	if (XR_FAILED(result)) {
		log_infof("xrCreateActionSet failed: [%s]", openxr_string(result));
		return false;
	}

	// Create an action to track the position and orientation of the hands! This is
	// the controller location, or the center of the palms for actual hands.
	XrActionCreateInfo action_info = { XR_TYPE_ACTION_CREATE_INFO };
	action_info.countSubactionPaths = _countof(xrc_hand_subaction_path);
	action_info.subactionPaths      = xrc_hand_subaction_path;
	action_info.actionType          = XR_ACTION_TYPE_POSE_INPUT;
	strcpy_s(action_info.actionName,          "hand_pose");
	strcpy_s(action_info.localizedActionName, "Hand Pose");
	result = xrCreateAction(xrc_action_set, &action_info, &xrc_pose_action);
	if (XR_FAILED(result)) {
		log_infof("xrCreateAction failed: [%s]", openxr_string(result));
		return false;
	}

	// Create an action to track the pointing position and orientation!
	action_info.actionType = XR_ACTION_TYPE_POSE_INPUT;
	strcpy_s(action_info.actionName,          "hand_point");
	strcpy_s(action_info.localizedActionName, "Hand Point");
	result = xrCreateAction(xrc_action_set, &action_info, &xrc_point_action);
	if (XR_FAILED(result)) {
		log_infof("xrCreateAction failed: [%s]", openxr_string(result));
		return false;
	}

	// Create an action for listening to the select action! This is primary trigger
	// on controllers, and an airtap on HoloLens
	action_info.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
	strcpy_s(action_info.actionName,          "select");
	strcpy_s(action_info.localizedActionName, "Select");
	result = xrCreateAction(xrc_action_set, &action_info, &xrc_select_action);
	if (XR_FAILED(result)) {
		log_infof("xrCreateAction failed: [%s]", openxr_string(result));
		return false;
	}

	action_info.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
	strcpy_s(action_info.actionName,          "grip");
	strcpy_s(action_info.localizedActionName, "Grip");
	result = xrCreateAction(xrc_action_set, &action_info, &xrc_grip_action);
	if (XR_FAILED(result)) {
		log_infof("xrCreateAction failed: [%s]", openxr_string(result));
		return false;
	}

	// Bind the actions we just created to specific locations on the Khronos simple_controller
	// definition! These are labeled as 'suggested' because they may be overridden by the runtime
	// preferences. For example, if the runtime allows you to remap buttons, or provides input
	// accessibility settings.
	XrPath profile_path;
	XrPath pose_path  [2];
	XrPath point_path [2];
	XrPath select_path[2];
	XrPath grip_path  [2];
	xrStringToPath(xr_instance, "/user/hand/left/input/grip/pose",  &pose_path[0]);
	xrStringToPath(xr_instance, "/user/hand/right/input/grip/pose", &pose_path[1]);
	xrStringToPath(xr_instance, "/user/hand/left/input/aim/pose",   &point_path[0]);
	xrStringToPath(xr_instance, "/user/hand/right/input/aim/pose",  &point_path[1]);
	XrInteractionProfileSuggestedBinding suggested_binds = { XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };

	// microsoft / motion_controller
	{
		xrStringToPath(xr_instance, "/user/hand/left/input/trigger/value",  &select_path[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/trigger/value", &select_path[1]);
		xrStringToPath(xr_instance, "/user/hand/left/input/squeeze/click",  &grip_path[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/squeeze/click", &grip_path[1]);
		XrActionSuggestedBinding bindings[] = {
			{ xrc_pose_action,   pose_path  [0] }, { xrc_pose_action,   pose_path  [1] },
			{ xrc_point_action,  point_path [0] }, { xrc_point_action,  point_path [1] },
			{ xrc_select_action, select_path[0] }, { xrc_select_action, select_path[1] },
			{ xrc_grip_action,   grip_path  [0] }, { xrc_grip_action,   grip_path  [1] },
		};

		xrStringToPath(xr_instance, "/interaction_profiles/microsoft/motion_controller", &profile_path);
		suggested_binds.interactionProfile     = profile_path;
		suggested_binds.suggestedBindings      = &bindings[0];
		suggested_binds.countSuggestedBindings = _countof(bindings);
		result = xrSuggestInteractionProfileBindings(xr_instance, &suggested_binds);
		if (XR_FAILED(result)) {
			log_infof("xrSuggestInteractionProfileBindings failed: [%s]", openxr_string(result));
			return false;
		}
	}

	// khr / simple_controller
	{
		xrStringToPath(xr_instance, "/user/hand/left/input/select/click",  &select_path[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/select/click", &select_path[1]);
		XrActionSuggestedBinding bindings[] = {
			{ xrc_pose_action,   pose_path  [0] }, { xrc_pose_action,   pose_path  [1] },
			{ xrc_point_action,  point_path [0] }, { xrc_point_action,  point_path [1] },
			{ xrc_select_action, select_path[0] }, { xrc_select_action, select_path[1] },
		};

		xrStringToPath(xr_instance, "/interaction_profiles/khr/simple_controller", &profile_path);
		suggested_binds.interactionProfile     = profile_path;
		suggested_binds.suggestedBindings      = &bindings[0];
		suggested_binds.countSuggestedBindings = _countof(bindings);
		result = xrSuggestInteractionProfileBindings(xr_instance, &suggested_binds);
		if (XR_FAILED(result)) {
			log_infof("xrSuggestInteractionProfileBindings failed: [%s]", openxr_string(result));
			return false;
		}
	}

	// Create frames of reference for the pose actions
	for (int32_t i = 0; i < 2; i++) {
		XrActionSpaceCreateInfo action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
		action_space_info.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
		action_space_info.subactionPath     = xrc_hand_subaction_path[i];
		action_space_info.action            = xrc_pose_action;
		result = xrCreateActionSpace(xr_session, &action_space_info, &xr_hand_space[i]);
		if (XR_FAILED(result)) {
			log_infof("xrCreateActionSpace failed: [%s]", openxr_string(result));
			return false;
		}

		action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
		action_space_info.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
		action_space_info.subactionPath     = xrc_hand_subaction_path[i];
		action_space_info.action            = xrc_point_action;
		result = xrCreateActionSpace(xr_session, &action_space_info, &xrc_point_space[i]);
		if (XR_FAILED(result)) {
			log_infof("xrCreateActionSpace failed: [%s]", openxr_string(result));
			return false;
		}
	}

	// Attach the action set we just made to the session
	XrSessionActionSetsAttachInfo attach_info = { XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
	attach_info.countActionSets = 1;
	attach_info.actionSets      = &xrc_action_set;
	result = xrAttachSessionActionSets(xr_session, &attach_info);
	if (XR_FAILED(result)) {
		log_infof("xrAttachSessionActionSets failed: [%s]", openxr_string(result));
		return false;
	}

	return true;
}

///////////////////////////////////////////

void oxri_shutdown() {
	xrDestroySpace(xr_hand_space[0]);
	xrDestroySpace(xr_hand_space[1]);
	xrDestroyActionSet(xrc_action_set);
}

///////////////////////////////////////////

void oxri_update_frame() {
	// Update our action set with up-to-date input data!
	XrActiveActionSet action_set = { };
	action_set.actionSet     = xrc_action_set;
	action_set.subactionPath = XR_NULL_PATH;
	XrActionsSyncInfo sync_info = { XR_TYPE_ACTIONS_SYNC_INFO };
	sync_info.countActiveActionSets = 1;
	sync_info.activeActionSets      = &action_set;
	xrSyncActions(xr_session, &sync_info);
}

} // namespace sk