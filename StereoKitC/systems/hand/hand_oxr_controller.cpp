#include "../../stereokit.h"
#include "../../_stereokit.h"
#include "../platform/openxr.h"
#include "../platform/openxr_input.h"
#include "../input.h"
#include "input_hand.h"

#include <string.h>
#include <stdlib.h>

namespace sk {

///////////////////////////////////////////

quat xrc_offset_rot [2];
vec3 xrc_offset_pos [2];
int  xrc_pointer_ids[3];

///////////////////////////////////////////

bool hand_oxrc_available() {
	return sk_active_runtime() == runtime_mixedreality 
		&& xr_session          != XR_NULL_HANDLE 
		&& xr_hand_state       == xr_hand_state_unavailable;
}

///////////////////////////////////////////

void hand_oxrc_init() {
	// Temporary orientation fix for WMR vs. HoloLens controllers
	if (sk_info.display_type == display_opaque) {
		xrc_offset_rot[handed_left ] = quat_from_angles(-45, 0, 0);
		xrc_offset_rot[handed_right] = quat_from_angles(-45, 0, 0);
		xrc_offset_pos[handed_left ] = { 0.01f, -0.01f, 0.015f };
		xrc_offset_pos[handed_right] = { 0.01f, -0.01f, 0.015f };
	} else {
		xrc_offset_rot[handed_left ] = quat_from_angles(-68, 0, 0);
		xrc_offset_rot[handed_right] = quat_from_angles(-68, 0, 0);
		xrc_offset_pos[handed_left ] = { 0, 0.005f, 0 };
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
}

///////////////////////////////////////////

void hand_oxrc_update_frame() {
	if (xr_time == 0) return;

	oxri_update_frame();

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
