/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2025 Nick Klingensmith
 * Copyright (c) 2024-2025 Qualcomm Technologies, Inc.
 */

#include "../platforms/platform.h"
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
		hand_t             *hand       = input_hand_ref   ((handed_)hand_id);
		// Update the hand point pose
		if ((controller->tracked & button_state_active) > 0) {
			hand->aim = controller->aim;
		}

		// Simulate the hand based on the state of the controller
		bool   tracked   = controller->tracked & button_state_active;
		pose_t hand_pose = tracked
			? controller->palm
			: hand->palm;

		if (animate) input_hand_sim      ((handed_)hand_id, false, hand_pose.position, hand_pose.orientation, tracked);
		else         input_hand_sim_poses((handed_)hand_id, false, hand_pose.position, hand_pose.orientation);
	}
}

///////////////////////////////////////////

void hand_oxrc_update_frame() {
	hand_oxrc_update_pose(true);

	// Now we'll get the current states of our actions, and store them for later use
	for (uint32_t hand_id = 0; hand_id < handed_max; hand_id++) {
		const controller_t *controller = input_controller((handed_)hand_id);
		hand_t             *hand       = input_hand_ref  ((handed_)hand_id);
		bool                tracked    = controller->tracked & button_state_active;

		hand->pinch_state      = button_make_state((hand->pinch_state & button_state_active) > 0, controller->trigger >= 0.5f);
		hand->grip_state       = button_make_state((hand->grip_state  & button_state_active) > 0, controller->grip    >= 0.5f);
		hand->pinch_activation = fminf(1,controller->trigger/0.5f);
		hand->grip_activation  = fminf(1,controller->grip   /0.5f);
		hand->aim_ready        = controller->tracked;
	}
}

///////////////////////////////////////////

void hand_oxrc_update_poses() {
	hand_oxrc_update_pose(false);
}

}

#endif