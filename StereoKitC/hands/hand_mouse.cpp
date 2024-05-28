/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

#include "../stereokit.h"
#include "../_stereokit.h"
#include "../systems/input.h"
#include "../xr_backends/simulator.h"
#include "input_hand.h"

namespace sk {

///////////////////////////////////////////

int     mouse_pointer_id;
float   mouse_hand_scroll  = 0;
handed_ mouse_active_hand  = handed_right;
int     mouse_active_state = 0;

///////////////////////////////////////////

bool hand_mouse_available() {
	return backend_xr_get_type() == backend_xr_type_simulator;
}

///////////////////////////////////////////

void hand_mouse_init() {
	mouse_pointer_id = input_hand_pointer_id[handed_right];

	input_hand_sim(handed_left,  true, vec3_zero, quat_identity, false);
	input_hand_sim(handed_right, true, vec3_zero, quat_identity, false);
}

///////////////////////////////////////////

void hand_mouse_shutdown() {
}

///////////////////////////////////////////

bool hand_mouse_update_position() {
	ray_t ray = {};
	const mouse_t *mouse = input_mouse();
	if (mouse->available && ray_from_mouse(mouse->pos, ray)) {
		quat pointer_rot = quat_lookat(vec3_zero, ray.dir);

		vec3 hand_pos     = ray.pos + ray.dir * (0.6f + mouse_hand_scroll * 0.00025f);
		quat hand_rot     = (mouse_active_hand == handed_right
			? quat_from_angles(40, 30, 90)
			: quat_from_angles(40,-30,-90)) * pointer_rot;

		pointer_t *pointer_cursor = input_get_pointer(mouse_pointer_id);
		pointer_cursor->ray.dir     = ray.dir;
		pointer_cursor->ray.pos     = hand_pos;
		pointer_cursor->orientation = pointer_rot;

		return true;
	}
	return false;
}

///////////////////////////////////////////

void hand_mouse_update_frame() {
	if ((input_key(key_shift) & button_state_just_active) && (input_key(key_ctrl) & button_state_active)) {
		mouse_active_state = (mouse_active_state + 1) % 4;
		switch (mouse_active_state) {
		case 0: mouse_active_hand = handed_right; break;
		case 1: mouse_active_hand = handed_max;   break;
		case 2: mouse_active_hand = handed_left;  break;
		case 3: mouse_active_hand = handed_max;   break;
		}
	}

	if (mouse_active_hand == handed_max) return;

	mouse_pointer_id = input_hand_pointer_id[mouse_active_hand];

	pointer_t *pointer_cursor = input_get_pointer(mouse_pointer_id);
	hand_t    *hand           = (hand_t*)input_hand(mouse_active_hand);
	bool l_pressed     = false;
	bool r_pressed     = false;
	bool was_tracked   = hand->tracked_state & button_state_active;
	bool was_l_pressed = hand->pinch_state   & button_state_active;
	bool was_r_pressed = hand->grip_state    & button_state_active;

	mouse_hand_scroll = mouse_hand_scroll + (input_mouse()->scroll - mouse_hand_scroll) * fminf(1, time_stepf_unscaled()*8);

	bool hand_tracked = hand_mouse_update_position();
	if (hand_tracked) {
		l_pressed = input_key(key_mouse_left ) & button_state_active;
		r_pressed = simulator_is_simulating_movement() ? false : input_key(key_mouse_right) & button_state_active;
	}
	pointer_cursor->tracked = button_make_state(was_tracked, hand_tracked);
	pointer_cursor->state   = input_key(key_mouse_left);

	quat hand_rot = (mouse_active_hand == handed_right
		? quat_from_angles(40, 30, 90)
		: quat_from_angles(40,-30,-90)) * pointer_cursor->orientation;
	input_hand_sim(mouse_active_hand, true, pointer_cursor->ray.pos, hand_rot, hand_tracked);

	input_source_ src = input_source_hand | input_source_hand_right;
	if (was_tracked   != hand_tracked) input_fire_event( src, hand_tracked  ? button_state_just_active : button_state_just_inactive, *pointer_cursor);
	if (was_l_pressed != l_pressed   ) input_fire_event( src, l_pressed     ? button_state_just_active : button_state_just_inactive, *pointer_cursor);
	if (was_r_pressed != r_pressed   ) input_fire_event( src, r_pressed     ? button_state_just_active : button_state_just_inactive, *pointer_cursor);

	hand->pinch_state      = input_key(key_mouse_left );
	hand->grip_state       = input_key(key_mouse_right);
	hand->pinch_activation = (input_key(key_mouse_left ) & button_state_active) > 0;
	hand->grip_activation  = (input_key(key_mouse_right) & button_state_active) > 0;

	hand->aim       = { pointer_cursor->ray.pos, pointer_cursor->orientation };
	hand->aim_ready = pointer_cursor->tracked;
}

///////////////////////////////////////////

void hand_mouse_update_poses() {
	hand_mouse_update_position();
	pointer_t *pointer_cursor = input_get_pointer(mouse_pointer_id);
	quat       hand_rot       = (mouse_active_hand == handed_right
		? quat_from_angles(40, 30, 90)
		: quat_from_angles(40,-30,-90)) * pointer_cursor->orientation;
	input_hand_sim_poses(mouse_active_hand, true, pointer_cursor->ray.pos, hand_rot);
}

}