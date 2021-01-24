#include "../stereokit.h"
#include "input.h"
#include "hand/input_hand.h"
#include "../libraries/array.h"
#include "platform/openxr.h"

namespace sk {

///////////////////////////////////////////

struct input_event_t {
	input_source_ source;
	button_state_ event;
	void (*event_callback)(input_source_ source, button_state_ evt, const pointer_t &pointer);
};

array_t<input_event_t>input_listeners  = {};
array_t<pointer_t>    input_pointers   = {};
mouse_t               input_mouse_data = {};
keyboard_t            input_key_data   = {};
pose_t                input_head_pose  = { vec3_zero, quat_identity };
pose_t                input_gaze_pose  = { vec3_zero, quat_identity };
button_state_         input_gaze_track_state = button_state_inactive;

///////////////////////////////////////////

int input_add_pointer(input_source_ source) {
	return (int)input_pointers.add({ source, button_state_inactive });
}

///////////////////////////////////////////

pointer_t *input_get_pointer(int32_t id) {
	return &input_pointers[id];
}

///////////////////////////////////////////

int input_pointer_count(input_source_ filter) {
	int result = 0;
	for (size_t i = 0; i < input_pointers.count; i++) {
		if (input_pointers[i].source & filter)
			result += 1;
	}
	return result;
}

///////////////////////////////////////////

pointer_t input_pointer(int32_t index, input_source_ filter) {
	int curr = 0;
	for (size_t i = 0; i < input_pointers.count; i++) {
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
	for (int64_t i = (int32_t)(input_listeners.count-1); i >= 0; i--) {
		if (input_listeners[i].source         == source && 
			input_listeners[i].event          == event  && 
			input_listeners[i].event_callback == event_callback) {
			input_listeners.remove(i);
		}
	}
}

///////////////////////////////////////////

void input_fire_event(input_source_ source, button_state_ event, const pointer_t &pointer) {
	for (size_t i = 0; i < input_listeners.count; i++) {
		if (input_listeners[i].source & source && input_listeners[i].event & event) {
			input_listeners[i].event_callback(source, event, pointer);
		}
	}
}

///////////////////////////////////////////

bool input_init() {
	input_hand_init();
	return true;
}

///////////////////////////////////////////

void input_shutdown() {
	input_pointers .free();
	input_listeners.free();
	input_hand_shutdown();
}

///////////////////////////////////////////

void input_update() {
	input_hand_update();
}

///////////////////////////////////////////

void input_update_predicted() {
	input_hand_update_predicted();
}

///////////////////////////////////////////

const mouse_t *input_mouse() {
	return &input_mouse_data;
}

///////////////////////////////////////////

button_state_ input_key(key_ key) {
	return (button_state_)input_key_data.keys[key];
}

///////////////////////////////////////////

const pose_t *input_head() {
	return &input_head_pose;
}

///////////////////////////////////////////

const pose_t *input_gaze() {
	return &input_gaze_pose;
}

///////////////////////////////////////////

button_state_ input_gaze_tracked() {
	return input_gaze_track_state;
}

///////////////////////////////////////////

bool32_t input_has_gaze() {
	return xr_has_gaze;
}

} // namespace sk {