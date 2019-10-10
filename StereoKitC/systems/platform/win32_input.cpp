#ifndef SK_NO_FLATSCREEN

#include "win32_input.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "win32.h"
#include "../input.h"
#include "../input_hand.h"
#include "../input_leap.h"
#include "../render.h"
#include "../d3d.h"

#include <directxmath.h> // Matrix math functions and objects
using namespace DirectX;

namespace sk {

///////////////////////////////////////////

int   win32_input_pointers[2];
bool  win32_use_leap    = true;
float win32_hand_scroll = 0;

///////////////////////////////////////////

void win32_mouse_update();
void win32_mouse_hand();

///////////////////////////////////////////

void win32_input_init() {
	win32_input_pointers[0] = input_add_pointer(input_source_hand | input_source_hand_right | input_source_gaze | input_source_gaze_cursor | input_source_can_press);
	win32_input_pointers[1] = input_add_pointer(input_source_gaze | input_source_gaze_head);

#ifndef SK_NO_LEAP_MOTION
	win32_use_leap = input_leap_init();
#endif
}

///////////////////////////////////////////

void win32_input_shutdown() {
#ifndef SK_NO_LEAP_MOTION
	if (win32_use_leap)
		input_leap_shutdown();
#endif
}

///////////////////////////////////////////

void win32_input_update() {
	win32_mouse_update();

#ifndef SK_NO_LEAP_MOTION
	if (win32_use_leap && leap_has_device) {
		input_leap_update();
	} else {
		win32_mouse_hand();
	}
#else
	win32_mouse_hand();
#endif

	pointer_t   *pointer_head = input_get_pointer(win32_input_pointers[1]);
	camera_t    *cam    = nullptr;
	transform_t *cam_tr = nullptr;
	render_get_cam(&cam, &cam_tr);

	if (cam != nullptr) {
		pointer_head->state   = pointer_state_available;
		pointer_head->ray.pos = cam_tr->_position;
		pointer_head->ray.dir = transform_forward(*cam_tr);
	}
}

///////////////////////////////////////////

void win32_mouse_update() {
	// Mouse scroll
	input_mouse_data.scroll_change = win32_scroll - input_mouse_data.scroll;
	input_mouse_data.scroll        = win32_scroll;
	// Mouse position and on-screen
	POINT cursor_pos;
	if (GetCursorPos(&cursor_pos) && ScreenToClient(win32_window, &cursor_pos)) {
		vec2 new_pos = vec2{ (float)cursor_pos.x, (float)cursor_pos.y };
		input_mouse_data.pos_change = new_pos - input_mouse_data.pos;
		input_mouse_data.pos        = new_pos;
		input_mouse_data.available  = true;
	} else {
		input_mouse_data.available = false;
	}
}

///////////////////////////////////////////

void win32_mouse_hand() {
	pointer_t *pointer_cursor = input_get_pointer(win32_input_pointers[0]);

	const hand_t &hand = input_hand(handed_right);
	vec3 hand_pos     = hand.root.position;
	quat hand_rot     = hand.root.orientation;
	bool l_pressed    = false;
	bool r_pressed    = false;
	bool hand_tracked = false;
	vec3 pointer_dir  = pointer_cursor->ray.dir;

	bool was_tracked   = hand.state & input_state_tracked;
	bool was_l_pressed = hand.state & input_state_pinch;
	bool was_r_pressed = hand.state & input_state_grip;

	pointer_cursor->state = pointer_state_none;

	win32_hand_scroll = win32_hand_scroll + (input_mouse_data.scroll - win32_hand_scroll) * time_elapsedf() * 8;

	ray_t ray = {};
	if (ray_from_mouse(input_mouse_data.pos, ray)) {
		hand_pos     = ray.pos + ray.dir * (0.6f + win32_hand_scroll * 0.001f);
		hand_rot     = quat_lookat(vec3_zero, ray.dir);
		hand_tracked = true;
		l_pressed    = input_key(key_mouse_left ) & button_state_down;
		r_pressed    = input_key(key_mouse_right) & button_state_down;

		pointer_cursor->state |= pointer_state_available;
		pointer_cursor->ray.dir     = ray.dir;
		pointer_cursor->ray.pos     = hand_pos;
		pointer_cursor->orientation = hand_rot;
	}

	input_hand_sim(handed_right, hand_pos, hand_rot, hand_tracked, l_pressed, r_pressed);

	input_source_ src = input_source_hand | input_source_hand_right;
	if (was_tracked   != hand_tracked) input_fire_event( src, hand_tracked  ? input_state_justtracked : input_state_untracked, *pointer_cursor);
	if (was_l_pressed != l_pressed   ) input_fire_event( src, l_pressed     ? input_state_justpinch   : input_state_unpinch,   *pointer_cursor);
	if (was_r_pressed != r_pressed   ) input_fire_event( src, r_pressed     ? input_state_justgrip    : input_state_ungrip,    *pointer_cursor);
}

} // namespace sk

#endif // SK_NO_FLATSCREEN