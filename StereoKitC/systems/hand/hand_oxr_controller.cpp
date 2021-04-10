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
		&& xr_session != XR_NULL_HANDLE;
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

	// Now we'll get the current states of our actions, and store them for later use
	matrix root   = render_get_cam_root();
	quat   root_q = matrix_extract_rotation(root);
	for (uint32_t hand = 0; hand < handed_max; hand++) {

		// Update the hand point pose
		pointer_t* pointer = input_get_pointer(input_hand_pointer_id[hand]);
		pointer->tracked = input_controllers[hand].tracked;

		if (pointer->tracked > 0) {
			pose_t point_pose = {
				matrix_mul_point   (root, input_controllers[hand].aim.position),
				matrix_mul_rotation(root, input_controllers[hand].aim.orientation) };

			pointer->ray.pos     = point_pose.position;
			pointer->ray.dir     = point_pose.orientation * vec3_forward;
			pointer->orientation = point_pose.orientation;
		}

		// Simulate the hand based on the state of the controller
		bool tracked = input_controllers[hand].tracked & button_state_active;
		
		pose_t hand_pose = input_hand((handed_)hand)->palm;
		if (tracked) {
			
			hand_pose.position    = matrix_mul_point   (root, input_controllers[hand].pose.position);
			hand_pose.orientation = root_q * input_controllers[hand].pose.orientation;

			hand_pose.orientation = xrc_offset_rot[hand] * hand_pose.orientation;
			hand_pose.position   += hand_pose.orientation * xrc_offset_pos[hand];	
		}
		input_hand_sim((handed_)hand, false, hand_pose.position, hand_pose.orientation, tracked, input_controllers[hand].trigger > 0.5f, input_controllers[hand].grip > 0.5f);

		// Get event poses, and fire our own events for them
		pointer->state = button_make_state(pointer->state & button_state_active, input_controllers[hand].trigger > 0.5f);

		const hand_t *curr_hand = input_hand((handed_)hand);
		
		if (curr_hand->pinch_state & button_state_changed && tracked) {
			pose_t pose      = {};
			pose.position    = matrix_mul_point(root, input_controllers[hand].aim.position);
			pose.orientation = root_q * input_controllers[hand].aim.orientation;

			pointer_t event_pointer = *pointer;
			event_pointer.ray.pos     = pose.position;
			event_pointer.ray.dir     = pose.orientation * vec3_forward;
			event_pointer.orientation = pose.orientation;

			input_fire_event(event_pointer.source, curr_hand->pinch_state & ~button_state_active, event_pointer);

		}
		if (curr_hand->grip_state & button_state_changed && tracked) {
			pose_t pose      = {};
			pose.position    = matrix_mul_point(root, input_controllers[hand].aim.position);
			pose.orientation = root_q * input_controllers[hand].aim.orientation;

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
