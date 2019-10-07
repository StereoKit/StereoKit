#pragma once

#include "../stereokit.h"

namespace sk {

enum hand_finger_ {
	hand_finger_thumb = 0,
	hand_finger_index,
	hand_finger_middle,
	hand_finger_ring,
	hand_finger_pinky,
};
enum hand_joint_ {
	hand_joint_metacarpal = 0,
	hand_joint_proximal,
	hand_joint_intermediate,
	hand_joint_distal,
	hand_joint_tip,
};

void input_hand_init    ();
void input_hand_shutdown();
void input_hand_update  ();

pose_t *input_hand_get_pose_buffer(handed_ hand);
void input_hand_sim(handed_ handedness, const vec3 &hand_pos, const quat &orientation, bool tracked, bool trigger_pressed, bool grip_pressed);
void input_hand_update_mesh(handed_ hand);
void input_hand_state_update(handed_ handedness);
void input_hand_make_solid();

} // namespace sk