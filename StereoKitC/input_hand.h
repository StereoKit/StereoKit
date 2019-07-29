#pragma once

#include "stereokit.h"

enum input_finger_ {
	input_finger_thumb = 0,
	input_finger_index,
	input_finger_middle,
	input_finger_ring,
	input_finger_pinky,
};
enum input_joint_ {
	input_joint_metacarpal = 0,
	input_joint_proximal,
	input_joint_intermediate,
	input_joint_distal,
	input_joint_tip,
};
enum input_handed_ {
	input_handed_left = 0,
	input_handed_right,
};

struct input_hand_t {
	pose_t        fingers[5][5];
	pose_t        wrist;
	input_handed_ handedness;
	bool          visible;
};

struct input_hand_mesh_t {
	mesh_t mesh;
	vert_t *verts;
	int vert_count;
	uint16_t *inds;
	int ind_count;
};

void input_hand_init    ();
void input_hand_shutdown();
void input_hand_update  ();

void input_hand_sim(input_handed_ handedness, const vec3 &hand_pos, const quat &orientation, bool visible, bool trigger_pressed, bool grip_pressed);
void input_hand_update_mesh(const input_hand_t &hand);