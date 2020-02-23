#ifndef SK_NO_FLATSCREEN

#include "win32_input.h"

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

int win32_gaze_pointer;

///////////////////////////////////////////

void win32_mouse_update();

///////////////////////////////////////////

void win32_input_init() {
	win32_gaze_pointer = input_add_pointer(input_source_gaze | input_source_gaze_head);

	render_set_view(matrix_trs(vec3{ 0,0.2f,0.4f }, quat_lookat({ 0,0.2f,0.4f }, vec3_zero)));
}

///////////////////////////////////////////

void win32_input_shutdown() {
}

///////////////////////////////////////////

void win32_input_update() {
#if WINDOWS_UWP
	for (int32_t i = 0; i < key_MAX; i++) {
		input_key_data.keys[i] = (uint8_t)button_make_state(
			input_key_data.keys[i] & button_state_active,
			uwp_key_down(i));
	}
#else
	for (int32_t i = 0; i < key_MAX; i++) {
		input_key_data.keys[i] = (uint8_t)button_make_state(
			input_key_data.keys[i] & button_state_active,
			GetKeyState(i) & 0x8000);
	}
#endif

	win32_mouse_update();

	pointer_t   *pointer_head = input_get_pointer(win32_gaze_pointer);
	pose_t       head         = input_head();

	pointer_head->tracked = button_state_active;
	pointer_head->ray.pos = head.position;
	pointer_head->ray.dir = head.orientation * vec3_forward;
}

///////////////////////////////////////////

void win32_mouse_update() {
	input_mouse_data.available = false;
	vec2  mouse_pos = {};
	float mouse_scroll = 0;

#if WINDOWS_UWP
	input_mouse_data.available = true;
	uwp_get_mouse(mouse_pos.x, mouse_pos.y, mouse_scroll);
#else
	mouse_scroll = win32_scroll;
	
	POINT cursor_pos;
	input_mouse_data.available = GetCursorPos(&cursor_pos) && ScreenToClient(win32_window, &cursor_pos);
	mouse_pos.x = (float)cursor_pos.x;
	mouse_pos.y = (float)cursor_pos.y;
#endif
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

} // namespace sk

#endif // SK_NO_FLATSCREEN