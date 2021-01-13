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

quat xrc_offset_rot[2];
vec3 xrc_offset_pos[2];

///////////////////////////////////////////

bool hand_oxrc_available() {
	return sk_active_display_mode() == display_mode_mixedreality 
		&& xr_session          != XR_NULL_HANDLE;
}

///////////////////////////////////////////

void hand_oxrc_init() {
}

///////////////////////////////////////////

void hand_oxrc_shutdown() {
}

///////////////////////////////////////////

void hand_oxrc_update_frame() {
	if (xr_time == 0) return;

	oxri_update_frame();

	// Now we'll get the current states of our actions, and store them for later use
	matrix root = render_get_cam_root();
	for (uint32_t hand = 0; hand < handed_max; hand++) {
		XrActionStateGetInfo get_info = { XR_TYPE_ACTION_STATE_GET_INFO };
		get_info.subactionPath = xrc_hand_subaction_path[hand];

		XrActionStatePose point_state = { XR_TYPE_ACTION_STATE_POSE };
		get_info.action = xrc_point_action;
		xrGetActionStatePose(xr_session, &get_info, &point_state);

		XrActionStatePose pose_state = { XR_TYPE_ACTION_STATE_POSE };
		get_info.action = xrc_pose_action;
		xrGetActionStatePose(xr_session, &get_info, &pose_state);

		// Events come with a timestamp
		XrActionStateFloat select_state = { XR_TYPE_ACTION_STATE_FLOAT };
		get_info.action = xrc_select_action;
		xrGetActionStateFloat(xr_session, &get_info, &select_state);

		// Events come with a timestamp
		XrActionStateFloat grip_state = { XR_TYPE_ACTION_STATE_FLOAT };
		get_info.action = xrc_grip_action;
		xrGetActionStateFloat(xr_session, &get_info, &grip_state);

		// Update the hand point pose
		pose_t     point_pose = {};
		pointer_t* pointer    = input_get_pointer(input_hand_pointer_id[hand]);
		pointer->tracked = button_make_state(pointer->tracked & button_state_active, point_state.isActive);
		pointer->state   = button_make_state(pointer->state   & button_state_active, select_state.currentState > 0.5f);
		if (openxr_get_space(xrc_point_space[hand], &point_pose)) {
			point_pose.position    = matrix_mul_point   (root, point_pose.position);
			point_pose.orientation = matrix_mul_rotation(root, point_pose.orientation);

			pointer->ray.pos = point_pose.position;
			pointer->ray.dir = point_pose.orientation * vec3_forward;
			pointer->orientation = point_pose.orientation;
		}

		// Simulate the hand based on the state of the controller
		pose_t hand_pose = {};
		if (openxr_get_space(xr_hand_space[hand], &hand_pose)) {
			hand_pose.position    = matrix_mul_point   (root, hand_pose.position);
			hand_pose.orientation = matrix_mul_rotation(root, hand_pose.orientation);

			hand_pose.orientation = xrc_offset_rot[hand] * hand_pose.orientation;
			hand_pose.position   += hand_pose.orientation * xrc_offset_pos[hand];
			input_hand_sim((handed_)hand, false, hand_pose.position, hand_pose.orientation, pose_state.isActive, select_state.currentState > 0.5f, grip_state.currentState > 0.5f);
		}

		// Get event poses, and fire our own events for them
		const hand_t *curr_hand = input_hand((handed_)hand);
		pose_t pose = {};
		if (curr_hand->pinch_state & button_state_changed &&
			openxr_get_space(xrc_point_space[hand], &pose, select_state.lastChangeTime)) {
			pose.position    = matrix_mul_point   (root, pose.position);
			pose.orientation = matrix_mul_rotation(root, pose.orientation);

			pointer_t event_pointer = *pointer;
			event_pointer.ray.pos     = pose.position;
			event_pointer.ray.dir     = pose.orientation * vec3_forward;
			event_pointer.orientation = pose.orientation;

			input_fire_event(event_pointer.source, curr_hand->pinch_state & ~button_state_active, event_pointer);

		}
		if (curr_hand->grip_state & button_state_changed &&
			openxr_get_space(xrc_point_space[hand], &pose, grip_state.lastChangeTime)) {
			pose.position    = matrix_mul_point   (root, pose.position);
			pose.orientation = matrix_mul_rotation(root, pose.orientation);

			pointer_t event_pointer = *pointer;
			event_pointer.ray.pos = pose.position;
			event_pointer.ray.dir = pose.orientation * vec3_forward;
			event_pointer.orientation = pose.orientation;

			input_fire_event(event_pointer.source, curr_hand->grip_state & ~button_state_active, event_pointer);

		}
		if (curr_hand->tracked_state & button_state_changed) {
			input_fire_event(pointer->source, pointer->tracked & ~button_state_active, *pointer);
		}
	}

	input_hand_update_meshes();
}

///////////////////////////////////////////

void hand_oxrc_update_predicted() {
}

}
