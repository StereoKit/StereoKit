#ifndef SK_NO_FLATSCREEN

#include "../../_stereokit.h"
#include "platform_utils.h"
#include "flatscreen_input.h"

#if WINDOWS_UWP
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "win32.h"
#include "uwp.h"
#include "../input.h"
#include "../hand/input_hand.h"
#include "../render.h"
#include "../d3d.h"

#include <directxmath.h> // Matrix math functions and objects
using namespace DirectX;

namespace sk {

///////////////////////////////////////////

int   fltscr_gaze_pointer;
float fltscr_move_speed = 1.4f; // average human walk speed, see: https://en.wikipedia.org/wiki/Preferred_walking_speed
vec2  fltscr_rot_speed  = {10.f, 5.f}; // converting mouse pixel movement to rotation
vec3  fltscr_head_rot   = {};

///////////////////////////////////////////

void flatscreen_mouse_update();

///////////////////////////////////////////

void flatscreen_input_init() {
	fltscr_gaze_pointer = input_add_pointer(input_source_gaze | input_source_gaze_head);

	render_set_view(matrix_trs(vec3{ 0,0.2f,0.4f }, quat_lookat({ 0,0.2f,0.4f }, vec3_zero)));
}

///////////////////////////////////////////

void flatscreen_input_shutdown() {
}

///////////////////////////////////////////

void flatscreen_input_update() {
#if WINDOWS_UWP
	for (int32_t i = 0; i < key_MAX; i++) {
		input_key_data.keys[i] = (uint8_t)button_make_state(
			input_key_data.keys[i] & button_state_active,
			uwp_key_down(i));
	}
#else
	uint8_t caps_lock = input_key_data.keys[key_caps_lock];
	for (int32_t i = 0; i < key_MAX; i++) {
		input_key_data.keys[i] = (uint8_t)button_make_state(
			input_key_data.keys[i] & button_state_active,
			GetKeyState(i)         & 0x8000);
	}
	input_key_data.keys[key_caps_lock] = (uint8_t)button_make_state(
		caps_lock                  & button_state_active,
		GetKeyState(key_caps_lock) & 0x1);
#endif

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

		// Apply movement to the camera
		pose_t head = input_head();
		head.position += head.orientation * movement * time_elapsedf() * fltscr_move_speed;

		// head rotation
		if (input_key(key_mouse_right) & button_state_active) {
			mouse_t mouse = input_mouse();
			fltscr_head_rot.y -= mouse.pos_change.x * fltscr_rot_speed.x * time_elapsedf();
			fltscr_head_rot.x -= mouse.pos_change.y * fltscr_rot_speed.y * time_elapsedf();
			head.orientation = quat_from_angles(fltscr_head_rot.x, fltscr_head_rot.y, fltscr_head_rot.z);

			vec2 prev_pt = mouse.pos - mouse.pos_change;
			input_mouse_data.pos = prev_pt;
			platform_set_cursor(prev_pt);
		}

		render_set_view(pose_matrix(head));
	}

	pointer_t   *pointer_head = input_get_pointer(fltscr_gaze_pointer);
	pose_t       head         = input_head();

	pointer_head->tracked = button_state_active;
	pointer_head->ray.pos = head.position;
	pointer_head->ray.dir = head.orientation * vec3_forward;
}

///////////////////////////////////////////

void flatscreen_mouse_update() {
	vec2  mouse_pos            = {};
	float mouse_scroll         = platform_get_scroll();
	input_mouse_data.available = platform_get_cursor(mouse_pos);

	// Mouse scroll
	input_mouse_data.scroll_change = mouse_scroll - input_mouse_data.scroll;
	input_mouse_data.scroll        = mouse_scroll;

	// Mouse position and on-screen
	if (input_mouse_data.available) {
		input_mouse_data.pos_change = mouse_pos - input_mouse_data.pos;
		input_mouse_data.pos        = mouse_pos;
		input_mouse_data.available  = true;
	}
}

///////////////////////////////////////////

bool flatscreen_is_simulating_movement() {
	return sk_runtime == runtime_flatscreen
		&& sk_settings.disable_flatscreen_mr_sim == false
		&& (   input_key(key_caps_lock) & button_state_active
			|| input_key(key_shift)     & button_state_active);
}

} // namespace sk

#endif // SK_NO_FLATSCREEN