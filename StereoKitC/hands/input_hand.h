/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

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
	hand_joint_root = 0,
	hand_joint_knuckle_major,
	hand_joint_knuckle_mid,
	hand_joint_knuckle_minor,
	hand_joint_tip,
};
enum hand_system_ {
	hand_system_none = 0,
	hand_system_mouse,
	hand_system_oxr_controllers,
	hand_system_oxr_articulated,
	hand_system_override
};

struct hand_mesh_t {
	matrix   root_transform;
	mesh_t   mesh;
	vert_t  *verts;
	uint32_t vert_count;
	vind_t  *inds;
	uint32_t ind_count;
};

extern int32_t input_hand_pointer_id[handed_max];

void input_hand_init    ();
void input_hand_shutdown();
void input_hand_update  ();

hand_system_  input_hand_get_system        ();
hand_t*       input_hand_ref               (handed_ hand);
void          input_hand_refresh_system    ();
void          input_hand_update_poses      ();
bool          input_hand_get_visible       (handed_ hand);
hand_joint_t* input_hand_get_pose_buffer   (handed_ hand);
void          input_hand_sim               (handed_ handedness, bool center_on_finger,  vec3 hand_pos, quat orientation, bool tracked);
void          input_hand_sim_poses         (handed_ handedness, bool mouse_adjustments, vec3 hand_pos, quat orientation);
void          input_hand_sim_trigger       (button_state_ prev_state, hand_joint_t a, hand_joint_t b, float activation_dist, float deactivation_dist, float max_dist, button_state_* out_state, float* out_activation);
void          input_hand_state_update      (handed_ handedness);
void          input_gen_fallback_mesh      (const hand_joint_t fingers[][5], mesh_t mesh, vert_t** ref_verts, vind_t** ref_inds);

#define PINCH_ACTIVATION_DIST   0.01f
#define PINCH_DEACTIVATION_DIST 0.02f
#define PINCH_MAX_DIST          0.08f

#define GRIP_ACTIVATION_DIST    0.05f
#define GRIP_DEACTIVATION_DIST  0.06f
#define GRIP_MAX_DIST           0.11f

} // namespace sk