#include "../../stereokit.h"
#include "../../_stereokit.h"
#include "../platform/openxr.h"
#include "../input.h"
#include "input_hand.h"

#include <string.h>
#include <stdlib.h>

namespace sk {

///////////////////////////////////////////

XrActionSet xrc_action_set;
XrAction    xrc_pose_action;
XrAction    xrc_point_action;
XrAction    xrc_select_action;
XrAction    xrc_grip_action;
XrPath      xrc_hand_subaction_path[2];
XrSpace     xrc_point_space[2];
quat        xrc_offset_rot[2];
vec3        xrc_offset_pos[2];
int         xrc_pointer_ids[3];
XrSpace     xr_hand_space[2] = {};

///////////////////////////////////////////

bool hand_oxrc_available() {
	return sk_active_runtime() == runtime_mixedreality && xr_session != XR_NULL_HANDLE && xr_hand_state == xr_hand_state_unavailable;
}

///////////////////////////////////////////

void hand_oxrc_init() {
	XrActionSetCreateInfo actionset_info = { XR_TYPE_ACTION_SET_CREATE_INFO };
	strcpy_s(actionset_info.actionSetName,          "input");
	strcpy_s(actionset_info.localizedActionSetName, "Input");
	XrResult result = xrCreateActionSet(xr_instance, &actionset_info, &xrc_action_set);
	xrStringToPath(xr_instance, "/user/hand/left",  &xrc_hand_subaction_path[0]);
	xrStringToPath(xr_instance, "/user/hand/right", &xrc_hand_subaction_path[1]);
	if (XR_FAILED(result)) {
		log_infof("xrCreateActionSet failed: [%s]", openxr_string(result));
		return;
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
		return;
	}

	// Create an action to track the pointing position and orientation!
	action_info.actionType = XR_ACTION_TYPE_POSE_INPUT;
	strcpy_s(action_info.actionName,          "hand_point");
	strcpy_s(action_info.localizedActionName, "Hand Point");
	result = xrCreateAction(xrc_action_set, &action_info, &xrc_point_action);
	if (XR_FAILED(result)) {
		log_infof("xrCreateAction failed: [%s]", openxr_string(result));
		return;
	}

	// Create an action for listening to the select action! This is primary trigger
	// on controllers, and an airtap on HoloLens
	action_info.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
	strcpy_s(action_info.actionName,          "select");
	strcpy_s(action_info.localizedActionName, "Select");
	result = xrCreateAction(xrc_action_set, &action_info, &xrc_select_action);
	if (XR_FAILED(result)) {
		log_infof("xrCreateAction failed: [%s]", openxr_string(result));
		return;
	}

	action_info.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
	strcpy_s(action_info.actionName,          "grip");
	strcpy_s(action_info.localizedActionName, "Grip");
	result = xrCreateAction(xrc_action_set, &action_info, &xrc_grip_action);
	if (XR_FAILED(result)) {
		log_infof("xrCreateAction failed: [%s]", openxr_string(result));
		return;
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
			return;
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
			return;
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
			return;
		}

		action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
		action_space_info.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
		action_space_info.subactionPath     = xrc_hand_subaction_path[i];
		action_space_info.action            = xrc_point_action;
		result = xrCreateActionSpace(xr_session, &action_space_info, &xrc_point_space[i]);
		if (XR_FAILED(result)) {
			log_infof("xrCreateActionSpace failed: [%s]", openxr_string(result));
			return;
		}
	}

	// Attach the action set we just made to the session
	XrSessionActionSetsAttachInfo attach_info = { XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
	attach_info.countActionSets = 1;
	attach_info.actionSets      = &xrc_action_set;
	result = xrAttachSessionActionSets(xr_session, &attach_info);
	if (XR_FAILED(result)) {
		log_infof("xrAttachSessionActionSets failed: [%s]", openxr_string(result));
		return;
	}

	// Temporary orientation fix for WMR vs. HoloLens controllers
	if (sk_info.display_type == display_opaque) {
		xrc_offset_rot[handed_left ] = quat_from_angles(-45, 0, 0);
		xrc_offset_rot[handed_right] = quat_from_angles(-45, 0, 0);
		xrc_offset_pos[handed_left]  = { 0.01f, -0.01f, 0.015f };
		xrc_offset_pos[handed_right] = { 0.01f, -0.01f, 0.015f };
	} else {
		xrc_offset_rot[handed_left ] = quat_from_angles(-68, 0, 0);
		xrc_offset_rot[handed_right] = quat_from_angles(-68, 0, 0);
		xrc_offset_pos[handed_left]  = { 0, 0.005f, 0 };
		xrc_offset_pos[handed_right] = { 0, 0.005f, 0 };
	}

	for (int32_t i = 1; i >= 0; i--) {
		input_source_ hand = i == 0 ? input_source_hand_left : input_source_hand_right;
		xrc_pointer_ids[i] = input_add_pointer(input_source_can_press | input_source_hand | hand);
	}
	xrc_pointer_ids[2] = input_add_pointer(input_source_gaze | input_source_gaze_head);
}

///////////////////////////////////////////

void hand_oxrc_shutdown() {
	xrDestroySpace(xr_hand_space[0]);
	xrDestroySpace(xr_hand_space[1]);
	xrDestroyActionSet(xrc_action_set);
}

///////////////////////////////////////////

void hand_oxrc_update_frame() {
	if (xr_time == 0) return;

	// Update our action set with up-to-date input data!
	XrActiveActionSet action_set = { };
	action_set.actionSet     = xrc_action_set;
	action_set.subactionPath = XR_NULL_PATH;
	XrActionsSyncInfo sync_info = { XR_TYPE_ACTIONS_SYNC_INFO };
	sync_info.countActiveActionSets = 1;
	sync_info.activeActionSets      = &action_set;
	xrSyncActions(xr_session, &sync_info);

	// Now we'll get the current states of our actions, and store them for later use
	for (uint32_t hand = 0; hand < handed_max; hand++) {
		XrActionStateGetInfo get_info = { XR_TYPE_ACTION_STATE_GET_INFO };
		get_info.subactionPath = xrc_hand_subaction_path[hand];

		XrActionStatePose point_state = { XR_TYPE_ACTION_STATE_POSE };
		get_info.action = xrc_pose_action;
		xrGetActionStatePose(xr_session, &get_info, &point_state);

		XrActionStatePose pose_state = { XR_TYPE_ACTION_STATE_POSE };
		get_info.action = xrc_pose_action;
		xrGetActionStatePose(xr_session, &get_info, &pose_state);

		// Events come with a timestamp
		XrActionStateBoolean select_state = { XR_TYPE_ACTION_STATE_BOOLEAN };
		get_info.action = xrc_select_action;
		xrGetActionStateBoolean(xr_session, &get_info, &select_state);

		// Events come with a timestamp
		XrActionStateBoolean grip_state = { XR_TYPE_ACTION_STATE_BOOLEAN };
		get_info.action = xrc_grip_action;
		xrGetActionStateBoolean(xr_session, &get_info, &grip_state);

		// Update the hand point pose
		pose_t     point_pose = {};
		pointer_t* pointer    = input_get_pointer(xrc_pointer_ids[hand]);
		pointer->tracked = button_make_state(pointer->tracked & button_state_active, point_state.isActive);
		pointer->state   = button_make_state(pointer->state   & button_state_active, select_state.currentState);
		if (openxr_get_space(xrc_point_space[hand], point_pose)) {
			pointer->ray.pos = point_pose.position;
			pointer->ray.dir = point_pose.orientation * vec3_forward;
			pointer->orientation = point_pose.orientation;
		}

		// If we have a select event, update the hand pose to match the event's timestamp
		pose_t hand_pose = {};
		if (openxr_get_space(xr_hand_space[hand], hand_pose)) {
			// TODO: make this into a oxr hand!
			hand_pose.orientation = xrc_offset_rot[hand] * hand_pose.orientation;
			hand_pose.position   += hand_pose.orientation * xrc_offset_pos[hand];
			input_hand_sim((handed_)hand, hand_pose.position, hand_pose.orientation, pose_state.isActive, select_state.currentState, grip_state.currentState);
		}

		// Get event poses, and fire our own events for them
		const hand_t& curr_hand = input_hand((handed_)hand);
		pose_t pose = {};
		if (curr_hand.pinch_state & button_state_changed &&
			openxr_get_space(xrc_point_space[hand], pose, select_state.lastChangeTime)) {

			pointer_t event_pointer = *pointer;
			event_pointer.ray.pos     = pose.position;
			event_pointer.ray.dir     = pose.orientation * vec3_forward;
			event_pointer.orientation = pose.orientation;

			input_fire_event(event_pointer.source, curr_hand.pinch_state & ~button_state_active, event_pointer);

		}
		if (curr_hand.grip_state & button_state_changed &&
			openxr_get_space(xrc_point_space[hand], pose, grip_state.lastChangeTime)) {

			pointer_t event_pointer = *pointer;
			event_pointer.ray.pos = pose.position;
			event_pointer.ray.dir = pose.orientation * vec3_forward;
			event_pointer.orientation = pose.orientation;

			input_fire_event(event_pointer.source, curr_hand.grip_state & ~button_state_active, event_pointer);

		}
		if (curr_hand.tracked_state & button_state_changed) {
			input_fire_event(pointer->source, pointer->tracked & ~button_state_active, *pointer);
		}
	}

	input_hand_update_meshes();
}

///////////////////////////////////////////

void hand_oxrc_update_predicted() {
}

}