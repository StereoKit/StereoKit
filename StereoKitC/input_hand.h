#pragma once

#include "stereokit.h"

enum hand_finger_ {
	input_finger_thumb = 0,
	input_finger_index,
	input_finger_middle,
	input_finger_ring,
	input_finger_pinky,
};
enum hand_joint_ {
	input_joint_metacarpal = 0,
	input_joint_proximal,
	input_joint_intermediate,
	input_joint_distal,
	input_joint_tip,
};

struct hand_mesh_t {
	mesh_t    mesh;
	vert_t   *verts;
	int       vert_count;
	uint16_t *inds;
	int       ind_count;
};

extern hand_t hand_info[2];

void input_hand_init    ();
void input_hand_shutdown();
void input_hand_update  ();

void input_hand_sim(handed_ handedness, const vec3 &hand_pos, const quat &orientation, bool tracked, bool trigger_pressed, bool grip_pressed);
void input_hand_update_mesh(const hand_t &hand);