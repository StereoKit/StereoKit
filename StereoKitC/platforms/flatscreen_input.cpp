#include "../_stereokit.h"
#include "platform_utils.h"
#include "flatscreen_input.h"

#include "../systems/input.h"
#include "../systems/render.h"

namespace sk {

///////////////////////////////////////////

int    fltscr_gaze_pointer;
float  fltscr_move_speed = 1.4f; // average human walk speed, see: https://en.wikipedia.org/wiki/Preferred_walking_speed
vec2   fltscr_rot_speed  = {10.f, 5.f}; // converting mouse pixel movement to rotation
matrix fltscr_transform  = matrix_identity;
vec3   fltscr_head_rot   = { -21, 0.0001f, 0 };
vec3   fltscr_head_pos   = {  0, 0.2f, 0.0f };
bool   fltscr_mouse_look = false;

///////////////////////////////////////////

void flatscreen_mouse_update();

///////////////////////////////////////////

void flatscreen_input_init() {
	fltscr_gaze_pointer = input_add_pointer(input_source_gaze | input_source_gaze_head);
	fltscr_transform    = sk_settings.disable_flatscreen_mr_sim
		? matrix_identity
		: matrix_trs( fltscr_head_pos, quat_from_angles( fltscr_head_rot.x, fltscr_head_rot.y, fltscr_head_rot.z ));
	render_set_cam_root(render_get_cam_root());
}

///////////////////////////////////////////

void flatscreen_input_shutdown() {
}

///////////////////////////////////////////

void flatscreen_input_update() {
	flatscreen_mouse_update();

	if (flatscreen_is_simulating_movement()) {

		// Get key based movement
		vec3 movement = {};
		if (input_key(key_w) & button_state_active) movement += vec3_forward;
		if (input_key(key_s) & button_state_active) movement -= vec3_forward;
		if (input_key(key_d) & button_state_active) movement += vec3_right;
		if (input_key(key_a) & button_state_active) movement -= vec3_right;
		if (input_key(key_e) & button_state_active) movement += vec3_up;
		if (input_key(key_q) & button_state_active) movement -= vec3_up;
		if (vec3_magnitude_sq( movement ) != 0)
			movement = vec3_normalize(movement);

		// head rotation
		quat orientation;
		if (input_key(key_mouse_right) & button_state_just_active) {
			fltscr_mouse_look = true;
		}
		if (fltscr_mouse_look) {
			const mouse_t *mouse = input_mouse();
			fltscr_head_rot.y -= mouse->pos_change.x * fltscr_rot_speed.x * time_elapsedf();
			fltscr_head_rot.x -= mouse->pos_change.y * fltscr_rot_speed.y * time_elapsedf();
			fltscr_head_rot.x = fmaxf(-89.9f, fminf(fltscr_head_rot.x, 89.9f));
			orientation = quat_from_angles(fltscr_head_rot.x, fltscr_head_rot.y, fltscr_head_rot.z);

			vec2 prev_pt = mouse->pos - mouse->pos_change;
			input_mouse_data.pos = prev_pt;
			platform_set_cursor(prev_pt);
		} else {
			orientation = quat_from_angles(fltscr_head_rot.x, fltscr_head_rot.y, fltscr_head_rot.z);
		}
		// Apply movement to the camera
		fltscr_head_pos += orientation * movement * time_elapsedf() * fltscr_move_speed;
		fltscr_transform = matrix_trs(fltscr_head_pos, orientation);
		render_set_cam_root(render_get_cam_root());
	} else {
		fltscr_mouse_look = false;
	}
	if (input_key(key_mouse_right) & button_state_just_inactive) {
		fltscr_mouse_look = false;
	}

	if (sk_settings.disable_flatscreen_mr_sim) {
		input_eyes_track_state = button_make_state(input_eyes_track_state & button_state_active, false);
	} else {
		bool sim_tracked = (input_key(key_alt) & button_state_active) > 0 ? true : false;
		input_eyes_track_state = button_make_state(input_eyes_track_state & button_state_active, sim_tracked);
		ray_t ray = {};
		if (sim_tracked && ray_from_mouse(input_mouse_data.pos, ray)) {
			input_eyes_pose_world.position    = ray.pos;
			input_eyes_pose_world.orientation = quat_lookat(vec3_zero, ray.dir);
			input_eyes_pose_local.position    = matrix_transform_pt(render_get_cam_final_inv(), ray.pos);
			input_eyes_pose_local.orientation = quat_lookat(vec3_zero, matrix_transform_dir(render_get_cam_final_inv(), ray.dir));
		}
	}

	pointer_t *pointer_head = input_get_pointer(fltscr_gaze_pointer);
	pointer_head->tracked = button_state_active;
	pointer_head->ray.pos = input_eyes_pose_world.position;
	pointer_head->ray.dir = input_eyes_pose_world.orientation * vec3_forward;
}

///////////////////////////////////////////

void flatscreen_mouse_update() {
	vec2  mouse_pos            = {};
	float mouse_scroll         = platform_get_scroll();
	input_mouse_data.available = platform_get_cursor(mouse_pos) && sk_focus == app_focus_active;

	// Mouse scroll
	if (sk_focus == app_focus_active) {
		input_mouse_data.scroll_change = mouse_scroll - input_mouse_data.scroll;
		input_mouse_data.scroll        = mouse_scroll;
	}

	// Mouse position and on-screen
	if (input_mouse_data.available) {
		input_mouse_data.pos_change = mouse_pos - input_mouse_data.pos;
		input_mouse_data.pos        = mouse_pos;
	}
}

///////////////////////////////////////////

bool flatscreen_is_simulating_movement() {
	return sk_display_mode == display_mode_flatscreen
		&& sk_settings.disable_flatscreen_mr_sim == false
		&& (   input_key(key_caps_lock) & button_state_active
			|| input_key(key_shift)     & button_state_active);
}

} // namespace sk