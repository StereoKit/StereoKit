/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2025 Nick Klingensmith
 * Copyright (c) 2024-2025 Qualcomm Technologies, Inc.
 */

#include "ui_core.h"
#include "ui_layout.h"
#include "ui_theming.h"
#include "interactors.h"
#include "interactor_modes.h"

#include "../systems/input.h"

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

void ui_core_init() {
	interaction_init     ();
	interactor_modes_init();
}

///////////////////////////////////////////

void ui_core_shutdown() {
	interactor_modes_shutdown();
	interaction_shutdown     ();
}

///////////////////////////////////////////

void ui_core_update() {
	interaction_update     ();
	interactor_modes_update();
}

///////////////////////////////////////////

inline bounds_t size_box(vec3 top_left, vec3 dimensions) {
	return { top_left - dimensions / 2, dimensions };
}

///////////////////////////////////////////

template<typename C>
button_state_ ui_volume_at_g(const C *id, bounds_t bounds, ui_confirm_ interact_type, handed_ *out_opt_hand, button_state_ *out_opt_focus_state) {
	id_hash_t     id_hash = ui_stack_hash(id);
	button_state_ result  = button_state_inactive;
	button_state_ focus   = button_state_inactive;
	interactor_t interactor = -1;

	vec3 start = bounds.center + bounds.dimensions / 2;
	interaction_1h_box(id_hash, interact_type == ui_confirm_push ? interactor_event_poke : interactor_event_pinch, -2,
		start, bounds.dimensions,
		start, bounds.dimensions,
		&focus, &interactor);

	_interactor_t *actor = _interactor_get(interactor);
	if (actor != nullptr) {
		result = interactor_set_active(actor, id_hash, actor->activation_type == interactor_activation_position
			? (bool32_t)((focus              & button_state_active) != 0)
			: (bool32_t)((actor->pinch_state & button_state_active) != 0));
	}

	if (out_opt_hand        != nullptr) *out_opt_hand        = (handed_)interactor;
	if (out_opt_focus_state != nullptr) *out_opt_focus_state = focus;
	return result;
}
button_state_ ui_volume_at   (const char     *id, bounds_t bounds, ui_confirm_ interact_type, handed_ *out_opt_hand, button_state_ *out_opt_focus_state) { return ui_volume_at_g<char    >(id, bounds, interact_type, out_opt_hand, out_opt_focus_state); }
button_state_ ui_volume_at_16(const char16_t *id, bounds_t bounds, ui_confirm_ interact_type, handed_ *out_opt_hand, button_state_ *out_opt_focus_state) { return ui_volume_at_g<char16_t>(id, bounds, interact_type, out_opt_hand, out_opt_focus_state); }

///////////////////////////////////////////

void ui_button_behavior(vec3 window_relative_pos, vec2 size, id_hash_t id, float& out_finger_offset, button_state_& out_button_state, button_state_& out_focus_state, interactor_t* out_opt_interactor) {
	ui_button_behavior_depth(window_relative_pos, size, id, skui_settings.depth, skui_settings.depth / 2, out_finger_offset, out_button_state, out_focus_state, out_opt_interactor);
}

///////////////////////////////////////////

void ui_button_behavior_depth(vec3 window_relative_pos, vec2 size, id_hash_t id, float button_depth, float button_activation_depth, float &out_finger_offset, button_state_ &out_button_state, button_state_ &out_focus_state, interactor_t* out_opt_interactor) {
	out_button_state  = button_state_inactive;
	out_focus_state   = button_state_inactive;
	out_finger_offset = button_depth;

	int32_t       interactor = -1;
	vec3          interaction_at;
	button_state_ focus_candidacy = button_state_inactive;
	interaction_1h_plate(id, interactor_event_poke, 0,
		{ window_relative_pos.x, window_relative_pos.y, window_relative_pos.z - button_depth }, { size.x, size.y, button_depth },
		&focus_candidacy, &interactor, &interaction_at);

	// If a hand is interacting, adjust the button surface accordingly
	_interactor_t* actor = _interactor_get(interactor);
	if (actor) {
		if (focus_candidacy & button_state_active) {
			bool pressed;
			if (actor->activation_type == interactor_activation_position) {
				out_finger_offset = -(interaction_at.z + actor->capsule_radius) - window_relative_pos.z;
				pressed = out_finger_offset < button_activation_depth;
			} else {
				pressed = (actor->pinch_state & button_state_active) && actor->focused_prev == id;
				if (pressed) out_finger_offset = 0;
			}
			const float min_press_depth = 2 * mm2m;
			out_finger_offset = fminf(fmaxf(min_press_depth, out_finger_offset), button_depth);
			out_button_state  = interactor_set_active(actor, id, pressed);
		} else if (focus_candidacy & button_state_just_inactive) {
			out_button_state = interactor_set_active(actor, id, false);
		}
		out_focus_state = button_make_state(actor->focused_prev_prev == id, actor->focused_prev == id);
	}

	if (out_opt_interactor)
		*out_opt_interactor = interactor;
}

///////////////////////////////////////////

void ui_slider_behavior(vec3 window_relative_pos, vec2 size, id_hash_t id, vec2* value, vec2 min, vec2 max, vec2 button_size_visual, vec2 button_size_interact, ui_confirm_ confirm_method, ui_slider_data_t* out_state) {
	const float snap_scale = 1;
	const float snap_dist  = 7*cm2m;

	// Input comes from the developer and could be any value, so we want to
	// ensure it's clamped to the correct range.
	vec2 smallest = min;
	vec2 largest  = max;
	if (min.x > max.x) { smallest.x = max.x; largest.x = min.x; }
	if (min.y > max.y) { smallest.y = max.y; largest.y = min.y; }
	if (value->x < smallest.x) value->x = smallest.x;
	if (value->y < smallest.y) value->y = smallest.y;
	if (value->x > largest .x) value->x = largest .x;
	if (value->y > largest .y) value->y = largest .y;

	// Find sizes of slider elements
	vec2  range        = max - min;
	vec2  percent      = { range.x == 0 ? 0.5f : (value->x - min.x)/range.x,
	                       range.y == 0 ? 0.5f : (value->y - min.y)/range.y };
	float button_depth = skui_settings.depth;

	// Set up for getting the state of the sliders.
	out_state->focus_state   = button_state_inactive;
	out_state->active_state  = button_state_inactive;
	out_state->interactor    = -1;
	out_state->finger_offset = button_depth;
	out_state->button_center = {
		window_relative_pos.x - (percent.x * (size.x - button_size_visual.x) + button_size_visual.x/2.0f),
		window_relative_pos.y - (percent.y * (size.y - button_size_visual.y) + button_size_visual.y/2.0f) };

	// Secondary motion check
	ui_push_idi((int32_t)id);
	button_state_ secondary_focus;
	int32_t       secondary_interactor;
	id_hash_t     secondary_id = ui_stack_hash("secondary");
	interaction_1h_box(secondary_id, (interactor_event_)(interactor_event_grip | interactor_event_pinch | interactor_event_poke), -1,
		window_relative_pos, {size.x, size.y, button_depth*0.4f },
		window_relative_pos, {size.x, size.y, button_depth*0.4f },
		& secondary_focus, &secondary_interactor);
	ui_pop_id();

	vec2 finger_at = {};
	_interactor_t* actor = nullptr;
	vec3 activation_size  = vec3{ button_size_interact.x, button_size_interact.y, button_depth };
	vec3 activation_start = { out_state->button_center.x+activation_size.x/2.0f, out_state->button_center.y+activation_size.y/2.0f, window_relative_pos.z };
	if (button_size_interact.x == 0 && button_size_interact.y == 0) {
		activation_size  = {size.x, size.y, button_depth};
		activation_start = window_relative_pos;
	}

	if (confirm_method == ui_confirm_push) {
		ui_button_behavior_depth(activation_start, { activation_size.x, activation_size.y }, id, button_depth, button_depth / 2, out_state->finger_offset, out_state->active_state, out_state->focus_state, &out_state->interactor);

		actor = _interactor_get(out_state->interactor);
		
	} else if (confirm_method == ui_confirm_pinch || confirm_method == ui_confirm_variable_pinch) {
		activation_start.z += skui_settings.depth;
		activation_size.z  += skui_settings.depth;
		interaction_1h_box(id, interactor_event_pinch, 0,
			activation_start, activation_size,
			activation_start, activation_size,
			&out_state->focus_state, &out_state->interactor);

		// Pinch confirm uses a handle that the user must pinch, in order to
		// drag it around the slider.
		actor = _interactor_get(out_state->interactor);
		if (actor != nullptr) {
			out_state->active_state = interactor_set_active(actor, id, actor->pinch_state & button_state_active);
		}
	}

	// Combine focus from both elements for using secondary motion
	_interactor_t* secondary_actor = _interactor_get(secondary_interactor);
	if ((secondary_actor && secondary_actor->focused_prev == secondary_id) ||
		(actor           && actor          ->focused_prev == id)) {

		_interactor_t* motion_actor     = actor ? actor : secondary_actor;
		vec2           secondary_motion = vec2_zero;
		if (motion_actor->secondary_motion_dimensions == 1) secondary_motion = vec2{ motion_actor->secondary_motion.x,  motion_actor->secondary_motion.x };
		if (motion_actor->secondary_motion_dimensions >= 2) secondary_motion = vec2{ motion_actor->secondary_motion.x, -motion_actor->secondary_motion.y };

		vec2 new_percent = {
			range.x == 0 ? 0.5f : fminf(1, fmaxf(0, percent.x + secondary_motion.x)),
			range.y == 0 ? 0.5f : fminf(1, fmaxf(0, percent.y + secondary_motion.y)) };
		vec2 new_val = min + new_percent * range;

		out_state->button_center = {
			window_relative_pos.x - (new_percent.x * (size.x - button_size_visual.x) + button_size_visual.x / 2.0f),
			window_relative_pos.y - (new_percent.y * (size.y - button_size_visual.y) + button_size_visual.y / 2.0f) };
		*value = new_val;
	}

	if (actor != nullptr) {
		if (actor->shape_type == interactor_type_point) {
			vec3 local_pos = hierarchy_to_local_point(actor->motion.position);
			finger_at = { local_pos.x, local_pos.y };
		} else {
			vec3 local_start = hierarchy_to_local_point(actor->capsule_start_world);
			vec3 local_end   = hierarchy_to_local_point(actor->capsule_end_world);
			ray_t ray = { local_start, local_end - local_start };
			float t;
			if (ray_intersect_plane(ray, {0,0,window_relative_pos.z}, { 0,0,1 }, t)) {
				vec3 pt = ray.pos + ray.dir * t;
				finger_at = { pt.x, pt.y };
			}
		}
	}

	vec2 new_percent = percent;
	if (out_state->active_state & button_state_active) {
		vec2 pos_in_slider = {
			fminf(1, fmaxf(0, ((window_relative_pos.x-button_size_visual.x/2)-finger_at.x) / (size.x-button_size_visual.x))),
			fminf(1, fmaxf(0, ((window_relative_pos.y-button_size_visual.y/2)-finger_at.y) / (size.y-button_size_visual.y)))};
		vec2 new_val = min + pos_in_slider*range;

		new_percent = {
			range.x == 0 ? 0.5f : (new_val.x - min.x) / range.x,
			range.y == 0 ? 0.5f : (new_val.y - min.y) / range.y };
		out_state->button_center = {
			window_relative_pos.x - (new_percent.x * (size.x - button_size_visual.x) + button_size_visual.x/2.0f),
			window_relative_pos.y - (new_percent.y * (size.y - button_size_visual.y) + button_size_visual.y/2.0f) };

		// Do this down here so we can calculate old_percent above
		*value = new_val;
	}
}

///////////////////////////////////////////

bool32_t _ui_handle_begin(id_hash_t id, pose_t* handle_pose, bounds_t handle_bounds, bool32_t draw, ui_move_ move_type, ui_gesture_ allowed_gestures) {
	bool result = interaction_handle(id, -2, handle_pose, handle_bounds, move_type, allowed_gestures);
	ui_push_surface(*handle_pose);

	float color_blend = 0;
	if (ui_id_focused(id)) color_blend = 1;
	if (ui_id_active_state(id) & button_state_just_active)
		ui_play_sound_on_off(ui_vis_handle, id, handle_bounds.center);

	if (draw) {
		ui_draw_element(ui_vis_handle,
			handle_bounds.center+handle_bounds.dimensions/2,
			handle_bounds.dimensions,
			color_blend);
		ui_nextline();
	}
	return result;
}

///////////////////////////////////////////

bool32_t ui_handle_begin(const char *text, pose_t& movement, bounds_t handle, bool32_t draw, ui_move_ move_type, ui_gesture_ allowed_gestures) {
	return _ui_handle_begin(ui_stack_hash(text), &movement, handle, draw, move_type, allowed_gestures);
}
bool32_t ui_handle_begin_16(const char16_t *text, pose_t& movement, bounds_t handle, bool32_t draw, ui_move_ move_type, ui_gesture_ allowed_gestures) {
	return _ui_handle_begin(ui_stack_hash_16(text), &movement, handle, draw, move_type, allowed_gestures);
}

///////////////////////////////////////////

void ui_handle_end() {
	ui_pop_surface();
}

///////////////////////////////////////////

void ui_push_surface(pose_t surface_pose, vec3 layout_start, vec2 layout_dimensions) {
	vec3   right = surface_pose.orientation * vec3_right;
	matrix trs   = matrix_trs(surface_pose.position + right*layout_start, surface_pose.orientation);
	hierarchy_push(trs);
	ui_layout_push(layout_start, layout_dimensions, true);
}

///////////////////////////////////////////

void ui_pop_surface() {
	ui_layout_pop();
	hierarchy_pop();
}

}