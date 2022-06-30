#include "../platforms/platform_utils.h"
#if defined(SK_XR_OPENXR)

#include "../stereokit.h"
#include "../_stereokit.h"
#include "../xr_backends/openxr.h"
#include "../xr_backends/openxr_input.h"
#include "../systems/input.h"
#include "../systems/render.h"
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

void hand_oxrc_update_pose(bool animate) {
	for (uint32_t hand_id = 0; hand_id < handed_max; hand_id++) {
		const controller_t *controller = input_controller ((handed_)hand_id);
		const hand_t       *hand       = input_hand       ((handed_)hand_id);
		pointer_t          *pointer    = input_get_pointer(input_hand_pointer_id[hand_id]);
		// Update the hand point pose
		pointer->tracked = controller->tracked;
		if (pointer->tracked > 0) {
			pointer->ray.pos     = controller->aim.position;
			pointer->ray.dir     = controller->aim.orientation * vec3_forward;
			pointer->orientation = controller->aim.orientation;
		}

		// Simulate the hand based on the state of the controller
		bool   tracked   = controller->tracked & button_state_active;
		pose_t hand_pose = tracked
			? controller->palm
			: hand->palm;

		if (animate) input_hand_sim      ((handed_)hand_id, false, hand_pose.position, hand_pose.orientation, tracked, controller->trigger > 0.5f, controller->grip > 0.5f);
		else         input_hand_sim_poses((handed_)hand_id, false, hand_pose.position, hand_pose.orientation);
	}
}

///////////////////////////////////////////

void hand_oxrc_update_frame() {
	hand_oxrc_update_pose(true);

	// Now we'll get the current states of our actions, and store them for later use
	for (uint32_t hand_id = 0; hand_id < handed_max; hand_id++) {
		const controller_t *controller = input_controller ((handed_)hand_id);
		const hand_t       *hand       = input_hand       ((handed_)hand_id);
		pointer_t          *pointer    = input_get_pointer(input_hand_pointer_id[hand_id]);
		bool                tracked    = controller->tracked & button_state_active;

		// Get event poses, and fire our own events for them
		pointer->state = button_make_state(pointer->state & button_state_active, controller->trigger > 0.5f);

		if (hand->pinch_state & button_state_changed && tracked) {
			pointer_t event_pointer = *pointer;
			event_pointer.ray.pos     = controller->aim.position;
			event_pointer.ray.dir     = controller->aim.orientation * vec3_forward;
			event_pointer.orientation = controller->aim.orientation;

			input_fire_event(event_pointer.source, hand->pinch_state & ~button_state_active, event_pointer);
		}
		if (hand->grip_state & button_state_changed && tracked) {
			pointer_t event_pointer = *pointer;
			event_pointer.ray.pos     = controller->aim.position;
			event_pointer.ray.dir     = controller->aim.orientation * vec3_forward;
			event_pointer.orientation = controller->aim.orientation;

			input_fire_event(event_pointer.source, hand->grip_state & ~button_state_active, event_pointer);
		}
		if (hand->tracked_state & button_state_changed) {
			input_fire_event(pointer->source, pointer->tracked & ~button_state_active, *pointer);
		}
	}
}

///////////////////////////////////////////

void hand_oxrc_update_poses(bool update_visuals) {
	hand_oxrc_update_pose(false);
	if (update_visuals)
		input_hand_update_meshes();
}

}

#endif