/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2025 Nick Klingensmith
 * Copyright (c) 2024-2025 Qualcomm Technologies, Inc.
 */

#include "../stereokit.h"
#include "../_stereokit.h"
#include "../systems/input.h"
#include "../xr_backends/simulator.h"
#include "input_hand.h"

namespace sk {

///////////////////////////////////////////

float   mouse_hand_scroll  = 0;
handed_ mouse_active_hand  = handed_right;
int     mouse_active_state = 0;

///////////////////////////////////////////

bool hand_mouse_available() {
	return backend_xr_get_type() == backend_xr_type_simulator;
}

///////////////////////////////////////////

void hand_mouse_init() {
	input_hand_sim(handed_left,  true, vec3_zero, quat_identity, false);
	input_hand_sim(handed_right, true, vec3_zero, quat_identity, false);
}

///////////////////////////////////////////

void hand_mouse_shutdown() {
}

///////////////////////////////////////////

bool hand_mouse_pose(float scroll_distance, pose_t *out_pose) {
	const mouse_t* mouse = input_mouse();
	ray_t          ray   = {};
	if (mouse->available && ray_from_mouse(mouse->pos, ray)) {
		out_pose->orientation = quat_lookat(vec3_zero, ray.dir);
		out_pose->position    = ray.pos + ray.dir * (0.6f + scroll_distance * 0.00025f);
		return true;
	}
	return false;
}

///////////////////////////////////////////

void hand_mouse_update_frame(handed_ hand) {
	if ((input_key(key_shift) & button_state_just_active) && (input_key(key_ctrl) & button_state_active)) {
		mouse_active_state = (mouse_active_state + 1) % 4;
		switch (mouse_active_state) {
		case 0: mouse_active_hand = handed_right; break;
		case 1: mouse_active_hand = handed_max;   break;
		case 2: mouse_active_hand = handed_left;  break;
		case 3: mouse_active_hand = handed_max;   break;
		}
	}

	// Only update for the active hand or if this is the currently active hand
	if (mouse_active_hand == handed_max || hand != mouse_active_hand) return;

	hand_t *hand_ref = input_hand_ref(hand);
	bool l_pressed     = false;
	bool r_pressed     = false;
	bool was_tracked   = hand_ref->tracked_state & button_state_active;
	bool was_l_pressed = hand_ref->pinch_state   & button_state_active;
	bool was_r_pressed = hand_ref->grip_state    & button_state_active;

	mouse_hand_scroll = mouse_hand_scroll + (input_mouse()->scroll - mouse_hand_scroll) * fminf(1, time_stepf_unscaled()*8);

	pose_t hand_pose    = pose_identity;
	bool   hand_tracked = hand_mouse_pose(mouse_hand_scroll, &hand_pose);
	if (hand_tracked) {
		l_pressed = input_key(key_mouse_left ) & button_state_active;
		r_pressed = simulator_is_simulating_movement() ? false : input_key(key_mouse_right) & button_state_active;
	}
	button_state_ tracked = button_make_state(was_tracked, hand_tracked);

	quat hand_rot = (hand == handed_right
		? quat_from_angles(40, 30, 90)
		: quat_from_angles(40,-30,-90)) * hand_pose.orientation;
	input_hand_sim(hand, true, hand_pose.position, hand_rot, hand_tracked);

	hand_ref->pinch_state      = input_key(key_mouse_left );
	hand_ref->grip_state       = input_key(key_mouse_right);
	hand_ref->pinch_activation = (input_key(key_mouse_left ) & button_state_active) > 0;
	hand_ref->grip_activation  = (input_key(key_mouse_right) & button_state_active) > 0;

	hand_ref->aim       = hand_pose;
	hand_ref->aim_ready = tracked;
}

///////////////////////////////////////////

void hand_mouse_update_poses(handed_ hand) {
	// Only update poses for the active hand
	if (mouse_active_hand == handed_max || hand != mouse_active_hand) return;
	
	pose_t hand_pose = pose_identity;
	if (hand_mouse_pose(mouse_hand_scroll, &hand_pose)) {
		quat hand_rot = (hand == handed_right
			? quat_from_angles(40,  30,  90)
			: quat_from_angles(40, -30, -90)) * hand_pose.orientation;
		input_hand_sim_poses(hand, true, hand_pose.position, hand_rot);
	}
}

}