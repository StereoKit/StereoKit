/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

#include "../stereokit.h"
#include "../systems/input.h"
#include "input_hand.h"

#include <string.h>

namespace sk {

///////////////////////////////////////////

bool32_t     override_enabled = false;
bool32_t     override_hand[2] = {};
hand_joint_t override_joints[2][27] = {};

///////////////////////////////////////////

bool hand_override_available() {
	return override_enabled;
}

///////////////////////////////////////////

void hand_override_init() {
}

///////////////////////////////////////////

void hand_override_shutdown() {
}

///////////////////////////////////////////

void hand_override_update_frame() {
	for (int32_t h = 0; h < handed_max; h++) {
		hand_t *inp_hand = input_hand_ref((handed_)h);
		inp_hand->tracked_state = button_make_state(inp_hand->tracked_state & button_state_active, override_hand[h]);

		if (!override_hand[h]) continue;

		hand_joint_t *pose = input_hand_get_pose_buffer((handed_)h);
		memcpy(pose, &override_joints[h], sizeof(hand_joint_t) * 25);
		inp_hand->palm  = pose_t{ override_joints[h][25].position, override_joints[h][25].orientation };
		inp_hand->wrist = pose_t{ override_joints[h][26].position, override_joints[h][26].orientation };
		inp_hand->aim_ready = button_state_inactive;

		input_hand_sim_trigger(inp_hand->pinch_state, inp_hand->fingers[hand_finger_index][hand_joint_tip], inp_hand->fingers[hand_finger_thumb][hand_joint_tip],
			PINCH_ACTIVATION_DIST, PINCH_DEACTIVATION_DIST, PINCH_MAX_DIST,
			&inp_hand->pinch_state, &inp_hand->pinch_activation);
		input_hand_sim_trigger(inp_hand->grip_state, inp_hand->fingers[hand_finger_ring][hand_joint_tip], inp_hand->fingers[hand_finger_ring][hand_joint_root],
			GRIP_ACTIVATION_DIST, GRIP_DEACTIVATION_DIST, GRIP_MAX_DIST,
			&inp_hand->grip_state, &inp_hand->grip_activation);
	}
}

///////////////////////////////////////////

void input_hand_override(handed_ hand, const hand_joint_t *hand_joints) {
	override_hand[hand] = hand_joints != nullptr;
	if (hand_joints != nullptr)
		memcpy(override_joints[hand], hand_joints, sizeof(hand_joint_t) * 27);

	bool32_t enable = override_hand[0] || override_hand[1];
	if (enable != override_enabled) {
		override_enabled = enable;

		input_hand_refresh_system();
		input_hand_update();
	}
}

}