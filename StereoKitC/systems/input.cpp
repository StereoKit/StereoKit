/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

#include "../stereokit.h"
#include "input.h"
#include "input_keyboard.h"
#include "input_render.h"
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

struct input_state_t {
	array_t<input_event_t>listeners;
	array_t<pointer_t>    pointers;
	mouse_t               mouse_data;
	controller_t          controllers[2];
	bool                  controller_hand[2];
	button_state_         controller_menubtn;
	button_state_         eyes_track_state;
};
static input_state_t local = {};

// TODO: these should be moved to local state
pose_t input_head_pose_local;
pose_t input_head_pose_world;
pose_t input_eyes_pose_local;
pose_t input_eyes_pose_world;

///////////////////////////////////////////

void input_mouse_update();
void input_hand_update_fallback_meshes(hand_mesh_t hand_mesh_2[]);

///////////////////////////////////////////

bool input_init() {
	local = {};
	input_head_pose_local = pose_identity;
	input_head_pose_world = pose_identity;
	input_eyes_pose_local = pose_identity;
	input_eyes_pose_world = pose_identity;

	input_keyboard_initialize();
	input_hand_init();
	input_mouse_update();
	input_render_init();
	return true;
}

///////////////////////////////////////////

void input_shutdown() {
	input_render_shutdown();
	input_keyboard_shutdown();
	local.pointers .free();
	local.listeners.free();
	input_hand_shutdown();
	local = {};
}

///////////////////////////////////////////

void input_step() {
	input_mouse_update();
	input_keyboard_update();
	input_hand_update();
	input_render_step();
}

///////////////////////////////////////////

void input_step_late() {
	input_update_poses();
	input_render_step_late();
}

///////////////////////////////////////////

int32_t input_add_pointer(input_source_ source) {
	return local.pointers.add({ source, button_state_inactive });
}

///////////////////////////////////////////

pointer_t *input_get_pointer(int32_t id) {
	return &local.pointers[id];
}

///////////////////////////////////////////

int32_t input_pointer_count(input_source_ filter) {
	int32_t result = 0;
	for (int32_t i = 0; i < local.pointers.count; i++) {
		if (local.pointers[i].source & filter)
			result += 1;
	}
	return result;
}

///////////////////////////////////////////

pointer_t input_pointer(int32_t index, input_source_ filter) {
	int32_t curr = 0;
	for (int32_t i = 0; i < local.pointers.count; i++) {
		if (local.pointers[i].source & filter) {
			if (curr == index)
				return local.pointers[i];
			curr += 1;
		}
	}
	return {};
}

///////////////////////////////////////////

void input_subscribe(input_source_ source, button_state_ input_event, void (*input_event_callback)(input_source_ source, button_state_ input_event, const pointer_t &in_pointer)) {
	local.listeners.add({ source, input_event, input_event_callback });
}

///////////////////////////////////////////

void input_unsubscribe(input_source_ source, button_state_ input_event, void (*input_event_callback)(input_source_ source, button_state_ input_event, const pointer_t &in_pointer)) {
	for (int32_t i = local.listeners.count-1; i >= 0; i--) {
		if (local.listeners[i].source         == source      &&
			local.listeners[i].event          == input_event &&
			local.listeners[i].event_callback == input_event_callback) {
			local.listeners.remove(i);
		}
	}
}

///////////////////////////////////////////

void input_fire_event(input_source_ source, button_state_ input_event, const pointer_t &pointer) {
	for (int32_t i = 0; i < local.listeners.count; i++) {
		if (local.listeners[i].source & source && local.listeners[i].event & input_event) {
			local.listeners[i].event_callback(source, input_event, pointer);
		}
	}
}

///////////////////////////////////////////

void input_update_poses() {
#if defined(SK_XR_OPENXR)
	if (backend_xr_get_type() == backend_xr_type_openxr)
		oxri_update_poses();
#endif
	input_hand_update_poses();
}

///////////////////////////////////////////

const mouse_t *input_mouse() {
	return &local.mouse_data;
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
	return local.eyes_track_state;
}

///////////////////////////////////////////

void input_eyes_tracked_set(button_state_ state) {
	local.eyes_track_state = state;
}

///////////////////////////////////////////

const controller_t* input_controller(handed_ hand) {
	return &local.controllers[hand];
}

///////////////////////////////////////////

controller_t* input_controller_ref(handed_ handed) {
	return &local.controllers[handed];
}

///////////////////////////////////////////

button_state_ input_controller_menu() {
	return local.controller_menubtn;
}

///////////////////////////////////////////

void input_controller_menu_set(button_state_ state) {
	local.controller_menubtn = state;
}

///////////////////////////////////////////

bool input_controller_key(handed_ hand, controller_key_ key, float *out_amount) {
	*out_amount = 0;
	switch (key) {
	case controller_key_trigger: if (local.controllers[hand].trigger > 0.1f) { *out_amount = local.controllers[hand].trigger; return true; } else { return false; }
	case controller_key_grip:    if (local.controllers[hand].grip    > 0.1f) { *out_amount = local.controllers[hand].grip;    return true; } else { return false; }
	case controller_key_menu:  return (local.controller_menubtn & button_state_active) > 0;
	case controller_key_stick: return (local.controllers[hand].stick_click & button_state_active) > 0;
	case controller_key_x1:    return (local.controllers[hand].x1 & button_state_active) > 0;
	case controller_key_x2:    return (local.controllers[hand].x2 & button_state_active) > 0;
	default: return false;
	}
}

///////////////////////////////////////////

bool input_controller_is_hand(handed_ hand) {
	return local.controller_hand[hand];
}

///////////////////////////////////////////

void input_controller_set_hand(handed_ hand, bool is_hand) {
	local.controller_hand[hand] = is_hand;
}

///////////////////////////////////////////

void input_mouse_update() {
	vec2  mouse_pos    = {};
	float mouse_scroll = platform_get_scroll();
	local.mouse_data.available = platform_get_cursor(&mouse_pos) && sk_app_focus() == app_focus_active;

	// Mouse scroll
	if (sk_app_focus() == app_focus_active) {
		local.mouse_data.scroll_change = mouse_scroll - local.mouse_data.scroll;
		local.mouse_data.scroll        = mouse_scroll;
	}

	// Mouse position and on-screen
	if (local.mouse_data.available) {
		local.mouse_data.pos_change = mouse_pos - local.mouse_data.pos;
		local.mouse_data.pos        = mouse_pos;
	}
}

///////////////////////////////////////////

void input_mouse_override_pos(vec2 override_pos) {
	local.mouse_data.pos = { override_pos.x, override_pos.y };
}

///////////////////////////////////////////

void body_make_shoulders(vec3* out_left, vec3* out_right) {
	// Average shoulder width for women:37cm, men:41cm, center of shoulder
	// joint is around 4cm inwards
	const float avg_shoulder_width = ((39.0f/2.0f)-4.0f)*cm2m;
	const float head_length        = 10*cm2m;
	const float neck_length        = 7*cm2m;

	// Chest center is down to the base of the head, and then down the neck.
	const pose_t *head = input_head();
	vec3 chest_center = head->position + head->orientation * vec3{0,-head_length,0};
	chest_center.y   -= neck_length;

	// Shoulder forward facing direction is head direction weighted equally 
	// with the direction of both hands.
	vec3 face_fwd = input_head()->orientation * vec3_forward;
	face_fwd.y = 0;
	face_fwd   = vec3_normalize(face_fwd) * 2;
	face_fwd  += vec3_normalize(input_hand(handed_left )->wrist.position - chest_center);
	face_fwd  += vec3_normalize(input_hand(handed_right)->wrist.position - chest_center);
	face_fwd  *= 0.25f;
	vec3 face_right = vec3_normalize(vec3_cross(face_fwd, vec3_up)) * avg_shoulder_width;

	if (out_left)  *out_left  = chest_center - face_right;
	if (out_right) *out_right = chest_center + face_right;
}

} // namespace sk