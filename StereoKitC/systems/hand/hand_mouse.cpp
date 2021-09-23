#include "../../stereokit.h"
#include "../input.h"
#include "../platform/flatscreen_input.h"
#include "input_hand.h"

namespace sk {

///////////////////////////////////////////

int   mouse_pointer_id;
float mouse_hand_scroll = 0;

///////////////////////////////////////////

bool hand_mouse_available() {
	return sk_active_display_mode() == display_mode_flatscreen;
}

///////////////////////////////////////////

void hand_mouse_init() {
	mouse_pointer_id = input_hand_pointer_id[handed_right];
}

///////////////////////////////////////////

void hand_mouse_shutdown() {
}

///////////////////////////////////////////

void hand_mouse_update_frame() {
	pointer_t *pointer_cursor = input_get_pointer(mouse_pointer_id);

	const hand_t *hand = input_hand(handed_right);
	vec3 hand_pos     = hand->palm.position;
	quat hand_rot     = hand->palm.orientation;
	bool l_pressed    = false;
	bool r_pressed    = false;
	bool hand_tracked = false;

	bool was_tracked   = hand->tracked_state & button_state_active;
	bool was_l_pressed = hand->pinch_state   & button_state_active;
	bool was_r_pressed = hand->grip_state    & button_state_active;

	mouse_hand_scroll = mouse_hand_scroll + (input_mouse_data.scroll - mouse_hand_scroll) * time_elapsedf_unscaled() * 8;

	ray_t ray = {};
	if (input_mouse_data.available && ray_from_mouse(input_mouse_data.pos, ray)) {
		quat pointer_rot = quat_lookat(vec3_zero, ray.dir);

		hand_pos     = ray.pos + ray.dir * (0.6f + mouse_hand_scroll * 0.00025f);
		hand_rot     = quat_from_angles(40,30,90) * pointer_rot;
		hand_tracked = true;
		l_pressed    = input_key(key_mouse_left ) & button_state_active;
		r_pressed    = flatscreen_is_simulating_movement() ? false : input_key(key_mouse_right) & button_state_active;

		pointer_cursor->ray.dir     = ray.dir;
		pointer_cursor->ray.pos     = hand_pos;
		pointer_cursor->orientation = pointer_rot;
	}
	pointer_cursor->tracked = button_make_state(was_tracked, hand_tracked);
	pointer_cursor->state   = button_make_state(was_l_pressed, l_pressed);

	input_hand_sim(handed_right, true, hand_pos, hand_rot, hand_tracked, l_pressed, r_pressed);
	input_hand_sim(handed_left,  true, vec3_zero, quat_identity, false, false, false);

	input_source_ src = input_source_hand | input_source_hand_right;
	if (was_tracked   != hand_tracked) input_fire_event( src, hand_tracked  ? button_state_just_active : button_state_just_inactive, *pointer_cursor);
	if (was_l_pressed != l_pressed   ) input_fire_event( src, l_pressed     ? button_state_just_active : button_state_just_inactive, *pointer_cursor);
	if (was_r_pressed != r_pressed   ) input_fire_event( src, r_pressed     ? button_state_just_active : button_state_just_inactive, *pointer_cursor);

	input_hand_update_meshes();
}

///////////////////////////////////////////

void hand_mouse_update_predicted() {
}

}