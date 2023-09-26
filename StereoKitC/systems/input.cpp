#include "../stereokit.h"
#include "input.h"
#include "input_keyboard.h"
#include "../hands/input_hand.h"
#include "../libraries/array.h"
#include "../xr_backends/openxr.h"
#include "../xr_backends/openxr_input.h"

namespace sk {

///////////////////////////////////////////

struct input_event_t {
	input_source_ source;
	button_state_ event;
	void (*event_callback)(input_source_ source, button_state_ evt, const pointer_t &pointer);
};

array_t<input_event_t>input_listeners      = {};
array_t<pointer_t>    input_pointers       = {};
mouse_t               input_mouse_data     = {};
controller_t          input_controllers[2] = {};
button_state_         input_controller_menubtn = {};

pose_t                input_head_pose_local  = { vec3_zero, quat_identity };
pose_t                input_head_pose_world  = { vec3_zero, quat_identity };
pose_t                input_eyes_pose_local  = { vec3_zero, quat_identity };
pose_t                input_eyes_pose_world  = { vec3_zero, quat_identity };
button_state_         input_eyes_track_state = button_state_inactive;

///////////////////////////////////////////

void input_mouse_update();

///////////////////////////////////////////

int32_t input_add_pointer(input_source_ source) {
	return input_pointers.add({ source, button_state_inactive });
}

///////////////////////////////////////////

pointer_t *input_get_pointer(int32_t id) {
	return &input_pointers[id];
}

///////////////////////////////////////////

int32_t input_pointer_count(input_source_ filter) {
	int32_t result = 0;
	for (int32_t i = 0; i < input_pointers.count; i++) {
		if (input_pointers[i].source & filter)
			result += 1;
	}
	return result;
}

///////////////////////////////////////////

pointer_t input_pointer(int32_t index, input_source_ filter) {
	int32_t curr = 0;
	for (int32_t i = 0; i < input_pointers.count; i++) {
		if (input_pointers[i].source & filter) {
			if (curr == index)
				return input_pointers[i];
			curr += 1;
		}
	}
	return {};
}

///////////////////////////////////////////

void input_subscribe(input_source_ source, button_state_ event, void (*event_callback)(input_source_ source, button_state_ event, const pointer_t &pointer)) {
	input_listeners.add({ source, event, event_callback });
}

///////////////////////////////////////////

void input_unsubscribe(input_source_ source, button_state_ event, void (*event_callback)(input_source_ source, button_state_ event, const pointer_t &pointer)) {
	for (int32_t i = input_listeners.count-1; i >= 0; i--) {
		if (input_listeners[i].source         == source && 
			input_listeners[i].event          == event  && 
			input_listeners[i].event_callback == event_callback) {
			input_listeners.remove(i);
		}
	}
}

///////////////////////////////////////////

void input_fire_event(input_source_ source, button_state_ event, const pointer_t &pointer) {
	for (int32_t i = 0; i < input_listeners.count; i++) {
		if (input_listeners[i].source & source && input_listeners[i].event & event) {
			input_listeners[i].event_callback(source, event, pointer);
		}
	}
}

///////////////////////////////////////////

bool input_init() {
	input_keyboard_initialize();
	input_hand_init();
	input_mouse_update();
	return true;
}

///////////////////////////////////////////

void input_shutdown() {
	input_keyboard_shutdown();
	input_pointers .free();
	input_listeners.free();
	input_hand_shutdown();
}

///////////////////////////////////////////

void input_step() {
	input_mouse_update();
	input_keyboard_update();
	input_hand_update();
}

///////////////////////////////////////////

void input_update_poses(bool update_visuals) {
#if defined(SK_XR_OPENXR)
	if (backend_xr_get_type() == backend_xr_type_openxr)
		oxri_update_poses();
#endif
	input_hand_update_poses(update_visuals);
}

///////////////////////////////////////////

const mouse_t *input_mouse() {
	return &input_mouse_data;
}

///////////////////////////////////////////

button_state_ input_key(key_ key) {
	return input_keyboard_get(key);
}

///////////////////////////////////////////

const pose_t *input_head() {
	return &input_head_pose_world;
}

///////////////////////////////////////////

const pose_t *input_eyes() {
	return &input_eyes_pose_world;
}

///////////////////////////////////////////

button_state_ input_eyes_tracked() {
	return input_eyes_track_state;
}

///////////////////////////////////////////

void input_mouse_update() {
	vec2  mouse_pos    = {};
	float mouse_scroll = platform_get_scroll();
	input_mouse_data.available = platform_get_cursor(mouse_pos) && sk_app_focus() == app_focus_active;

	// Mouse scroll
	if (sk_app_focus() == app_focus_active) {
		input_mouse_data.scroll_change = mouse_scroll - input_mouse_data.scroll;
		input_mouse_data.scroll        = mouse_scroll;
	}

	// Mouse position and on-screen
	if (input_mouse_data.available) {
		input_mouse_data.pos_change = mouse_pos - input_mouse_data.pos;
		input_mouse_data.pos        = mouse_pos;
	}
}

} // namespace sk {