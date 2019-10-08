#ifndef SK_NO_FLATSCREEN

#include "win32_input.h"

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
		return;
	}
#endif

	pointer_t *pointer_cursor = input_get_pointer(win32_input_pointers[0]);
	pointer_t *pointer_head   = input_get_pointer(win32_input_pointers[1]);

	camera_t    *cam    = nullptr;
	transform_t *cam_tr = nullptr;
	render_get_cam(&cam, &cam_tr);

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
	pointer_head  ->state = pointer_state_none;	

	win32_hand_scroll = win32_hand_scroll + (input_mouse.scroll - win32_hand_scroll) * time_elapsedf();

	if (cam != nullptr) {
		pointer_head->state   = pointer_state_available;
		pointer_head->ray.pos = cam_tr->_position;
		pointer_head->ray.dir = transform_forward(*cam_tr);

		ray_t ray = ray_from_mouse(input_mouse.pos);
		hand_pos = ray.pos + ray.dir * (0.6f + win32_hand_scroll * 0.001f);
		hand_rot = quat_lookat(vec3_zero, ray.dir);
	}

	pointer_cursor->state |= pointer_state_available;
	pointer_cursor->ray.dir     = pointer_dir;
	pointer_cursor->ray.pos     = hand_pos;
	pointer_cursor->orientation = hand_rot;

	input_hand_sim(handed_right, hand_pos, hand_rot, hand_tracked, l_pressed, r_pressed);

	input_source_ src = input_source_hand | input_source_hand_right;
	if (was_tracked   != hand_tracked) input_fire_event( src, hand_tracked  ? input_state_justtracked : input_state_untracked, *pointer_cursor);
	if (was_l_pressed != l_pressed   ) input_fire_event( src, l_pressed     ? input_state_justpinch   : input_state_unpinch,   *pointer_cursor);
	if (was_r_pressed != r_pressed   ) input_fire_event( src, r_pressed     ? input_state_justgrip    : input_state_ungrip,    *pointer_cursor);
}

///////////////////////////////////////////

void win32_mouse_update() {
	// Mouse scroll
	input_mouse.scroll_change = win32_scroll - input_mouse.scroll;
	input_mouse.scroll        = win32_scroll;
	// Mouse position and on-screen
	POINT cursor_pos;
	if (GetCursorPos(&cursor_pos) && ScreenToClient(win32_window, &cursor_pos)) {
		vec2 new_pos = vec2{ cursor_pos.x, cursor_pos.y };
		input_mouse.pos_change = new_pos - input_mouse.pos;
		input_mouse.pos        = new_pos;
		input_mouse.available  = true;
	} else {
		input_mouse.available = false;
	}
	// Mouse buttons
	input_mouse.button_left   = button_make_state(input_mouse.button_left   & button_state_down, GetKeyState(VK_LBUTTON) < 0);
	input_mouse.button_right  = button_make_state(input_mouse.button_right  & button_state_down, GetKeyState(VK_RBUTTON) < 0);
	input_mouse.button_center = button_make_state(input_mouse.button_center & button_state_down, GetKeyState(VK_MBUTTON) < 0);
}

} // namespace sk

#endif // SK_NO_FLATSCREEN