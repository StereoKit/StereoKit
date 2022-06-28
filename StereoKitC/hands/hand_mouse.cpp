#include "../stereokit.h"
#include "../_stereokit.h"
#include "../systems/input.h"
#include "../platforms/flatscreen_input.h"
#include "input_hand.h"

namespace sk {

///////////////////////////////////////////

int     mouse_pointer_id;
float   mouse_hand_scroll  = 0;
handed_ mouse_active_hand  = handed_right;
int     mouse_active_state = 0;

///////////////////////////////////////////

bool hand_mouse_available() {
	return sk_active_display_mode() == display_mode_flatscreen && !sk_settings.disable_flatscreen_mr_sim;
}

///////////////////////////////////////////

void hand_mouse_init() {
	mouse_pointer_id = input_hand_pointer_id[handed_right];

	input_hand_sim(handed_left,  true, vec3_zero, quat_identity, false, false, false);
	input_hand_sim(handed_right, true, vec3_zero, quat_identity, false, false, false);
}

///////////////////////////////////////////

void hand_mouse_shutdown() {
}

///////////////////////////////////////////

bool hand_mouse_update_position() {
	ray_t ray = {};
	if (input_mouse_data.available && ray_from_mouse(input_mouse_data.pos, ray)) {
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

	pointer_t    *pointer_cursor = input_get_pointer(mouse_pointer_id);
	const hand_t *hand           = input_hand       (mouse_active_hand);
	bool l_pressed     = false;
	bool r_pressed     = false;
	bool was_tracked   = hand->tracked_state & button_state_active;
	bool was_l_pressed = hand->pinch_state   & button_state_active;
	bool was_r_pressed = hand->grip_state    & button_state_active;

	mouse_hand_scroll = mouse_hand_scroll + (input_mouse_data.scroll - mouse_hand_scroll) * fminf(1, time_elapsedf_unscaled()*8);

	bool hand_tracked = hand_mouse_update_position();
	if (hand_tracked) {
		l_pressed = input_key(key_mouse_left ) & button_state_active;
		r_pressed = flatscreen_is_simulating_movement() ? false : input_key(key_mouse_right) & button_state_active;
	}
	pointer_cursor->tracked = button_make_state(was_tracked, hand_tracked);
	pointer_cursor->state   = button_make_state(was_l_pressed, l_pressed);

	quat hand_rot = (mouse_active_hand == handed_right
		? quat_from_angles(40, 30, 90)
		: quat_from_angles(40,-30,-90)) * pointer_cursor->orientation;
	input_hand_sim(mouse_active_hand, true, pointer_cursor->ray.pos, hand_rot, hand_tracked, l_pressed, r_pressed);

	input_source_ src = input_source_hand | input_source_hand_right;
	if (was_tracked   != hand_tracked) input_fire_event( src, hand_tracked  ? button_state_just_active : button_state_just_inactive, *pointer_cursor);
	if (was_l_pressed != l_pressed   ) input_fire_event( src, l_pressed     ? button_state_just_active : button_state_just_inactive, *pointer_cursor);
	if (was_r_pressed != r_pressed   ) input_fire_event( src, r_pressed     ? button_state_just_active : button_state_just_inactive, *pointer_cursor);
}

///////////////////////////////////////////

void hand_mouse_update_poses(bool update_visuals) {
	hand_mouse_update_position();
	pointer_t *pointer_cursor = input_get_pointer(mouse_pointer_id);
	quat       hand_rot       = (mouse_active_hand == handed_right
		? quat_from_angles(40, 30, 90)
		: quat_from_angles(40,-30,-90)) * pointer_cursor->orientation;
	input_hand_sim_poses(mouse_active_hand, true, pointer_cursor->ray.pos, hand_rot);

	if (update_visuals)
		input_hand_update_meshes();
}

}