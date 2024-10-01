/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

#include "ui_core.h"
#include "ui_layout.h"
#include "ui_theming.h"

#include "../libraries/array.h"
#include "../libraries/ferr_hash.h"
#include "../systems/input.h"
#include "../hands/input_hand.h"
#include "../sk_math.h"
#include "../xr_backends/openxr_extensions.h"

#include <float.h>

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

array_t<interactor_t> skui_interactors;
float                 skui_finger_radius;
id_hash_t             skui_last_element;
bool32_t              skui_show_volumes;

array_t<bool32_t>    skui_enabled_stack;
array_t<id_hash_t>   skui_id_stack;
array_t<bool>        skui_preserve_keyboard_stack;
array_t<id_hash_t>   skui_preserve_keyboard_ids[2];
array_t<id_hash_t>*  skui_preserve_keyboard_ids_read;
array_t<id_hash_t>*  skui_preserve_keyboard_ids_write;

int32_t skui_input_mode                 = 2;
int32_t skui_hand_interactors[6]        = { -1, -1, -1, -1 };
int32_t skui_mouse_interactor           = -1;
float   skui_controller_trigger_last[2] = { 0, 0 };
float   skui_ray_active[2]              = { 0, 0 };
float   skui_ray_visible[2]             = { 0, 0 };

///////////////////////////////////////////

void ui_core_init() {
	skui_finger_radius            = 0;
	skui_last_element             = 0xFFFFFFFFFFFFFFFF;
	skui_show_volumes             = false;
	skui_enabled_stack            = {};
	skui_id_stack                 = {};
	skui_preserve_keyboard_stack  = {};
	skui_preserve_keyboard_ids[0] = {};
	skui_preserve_keyboard_ids[1] = {};
	skui_interactors              = {};

	skui_preserve_keyboard_ids_read  = &skui_preserve_keyboard_ids[0];
	skui_preserve_keyboard_ids_write = &skui_preserve_keyboard_ids[1];

	skui_id_stack.add({ HASH_FNV64_START });

	skui_hand_interactors[0] = interactor_create(interactor_type_point, interactor_event_poke,  interactor_activate_position);
	skui_hand_interactors[1] = interactor_create(interactor_type_point, interactor_event_pinch, interactor_activate_state);
	skui_hand_interactors[2] = interactor_create(interactor_type_line,  (interactor_event_)(interactor_event_poke | interactor_event_pinch), interactor_activate_state);
	skui_hand_interactors[3] = interactor_create(interactor_type_point, interactor_event_poke,  interactor_activate_position);
	skui_hand_interactors[4] = interactor_create(interactor_type_point, interactor_event_pinch, interactor_activate_state);
	skui_hand_interactors[5] = interactor_create(interactor_type_line,  (interactor_event_)(interactor_event_poke | interactor_event_pinch), interactor_activate_state);

	skui_mouse_interactor    = interactor_create(interactor_type_line,  (interactor_event_)(interactor_event_poke | interactor_event_pinch), interactor_activate_state);

	skui_input_mode = device_display_get_type() == display_type_flatscreen
		? 2  // Mouse
		: 0; // Hands
}

///////////////////////////////////////////

void ui_core_shutdown() {
	skui_interactors             .free();
	skui_enabled_stack           .free();
	skui_id_stack                .free();
	skui_preserve_keyboard_stack .free();
	skui_preserve_keyboard_ids[0].free();
	skui_preserve_keyboard_ids[1].free();
	skui_preserve_keyboard_ids_read  = nullptr;
	skui_preserve_keyboard_ids_write = nullptr;
}

///////////////////////////////////////////

void ui_show_ray(int32_t interactor, float skip, bool hide_inactive, float *ref_visible_amt, float *ref_active_amt) {
	interactor_t* actor = &skui_interactors[interactor];
	if ((actor->tracked & button_state_active) == 0) return;

	bool  actor_visible = hide_inactive == false || actor->focused_prev != 0;
	float visibility    = 1;
	if (ref_visible_amt != nullptr) {
		*ref_visible_amt = math_lerp(*ref_visible_amt, actor_visible ? 1.f : 0.f, 16.0f * time_stepf_unscaled());
		visibility = *ref_visible_amt;
	}
	if (visibility < 0.001f) return;

	float active = 0;
	if (ref_active_amt != nullptr) {
		*ref_active_amt = math_lerp(*ref_active_amt, actor->active_prev != 0 ? 1.f : 0.f, 16.0f * time_stepf_unscaled());
		active = *ref_active_amt;
	}

	float length         = 0.35f;
	vec3  uncentered_dir = vec3_normalize(actor->capsule_end_world  - actor->position);
	vec3  centered_dir   = uncentered_dir;
	if (actor->focused_prev != 0) {
		vec3 pt = actor->focus_center_world;
		length       = vec3_distance (pt,  actor->position);
		centered_dir = vec3_normalize(pt - actor->position);
	}
	length = math_lerp(0.35f, length, visibility);
	length = fmaxf(0, length - skip);

	float alpha = 0.35f + active * 0.65f;
	if (hide_inactive) alpha *= visibility;

	const int32_t ct = 20;
	line_point_t pts[ct];
	for (int32_t i = 0; i < ct; i += 1) {
		float pct   = (float)i / (float)(ct - 1);
		float blend = pct * pct * pct * 0.2f;
		float d     = skip + pct * length;
		
		float pct_i = 1 - pct;
		float curve = math_lerp(
			sinf(pct_i * pct_i * 3.14159f),
			fminf(1,sinf(pct * pct * 3.14159f)*1.5f), active);
		float width = (0.002f + curve * 0.003f) * visibility;
		pts[i] = line_point_t{ actor->position + vec3_lerp(uncentered_dir*d, centered_dir*d, blend), width, color32{ 255,255,255,(uint8_t)(curve*alpha*255) } };
	}
	line_add_listv(pts, ct);
}

///////////////////////////////////////////

void ui_core_hands_step() {
	static bool prev_active[2] = { false, false };

	for (int32_t i = 0; i < handed_max; i++) {
		const hand_t* hand = input_hand((handed_)i);

		// Poke
		interactor_update(skui_hand_interactors[i*3 + 0],
			(hand->tracked_state & button_state_just_active) ? hand->fingers[1][4].position : skui_interactors[i*3 + 0].capsule_end_world, hand->fingers[1][4].position, hand->fingers[1][4].radius,
			hand->fingers[1][4].position, hand->palm.orientation, hand->fingers[1][4].position,
			button_state_inactive, hand->tracked_state);

		// Pinch
		interactor_update(skui_hand_interactors[i*3 + 1],
			hand->fingers[0][4].position, hand->fingers[1][4].position, hand->fingers[1][4].radius,
			hand->pinch_pt,    hand->palm.orientation, hand->pinch_pt,
			hand->pinch_state, hand->tracked_state);

		// Hand ray
		if (ui_far_interact_enabled()) {
			float hand_dist = vec3_distance(hand->palm.position, input_head()->position + vec3{0,-0.12f,0});

			// Pinches can only start when aim_ready, but should remain true
			// as long as the pinch remains active
			bool is_pinched   = (hand->pinch_state & button_state_active) > 0;
			bool just_pinched = (hand->pinch_state & button_state_just_active) > 0;
			bool aim_ready    = (hand->aim_ready   & button_state_active) > 0;
			bool is_active    = (prev_active[i] && is_pinched) || (aim_ready && just_pinched);
			button_state_ far_pinch_state = button_make_state(prev_active[i], is_active);
			prev_active[i] = is_active;

			interactor_min_distance_set(skui_hand_interactors[i*3 + 2], math_lerp(0.35f, 0.20f, math_saturate((hand_dist - 0.1f) / 0.4f)));
			interactor_update          (skui_hand_interactors[i*3 + 2],
				hand->aim.position, hand->aim.position + hand->aim.orientation * vec3_forward * 100, 0.01f,
				hand->aim.position, hand->aim.orientation, input_head()->position + vec3{0,-0.12f,0},
				far_pinch_state, hand->aim_ready);
			ui_show_ray(skui_hand_interactors[i*3 + 2], 0.07f, true, &skui_ray_visible[i], &skui_ray_active[i]);

		}
	}
}

///////////////////////////////////////////

void ui_core_controllers_step() {
	if (ui_far_interact_enabled() == false) return;

	for (int32_t i = 0; i < handed_max; i++) {
		const controller_t *ctrl = input_controller((handed_)i);

		// controller ray

		interactor_min_distance_set(skui_hand_interactors[i*3 + 2], -100000);
		interactor_update(skui_hand_interactors[i*3 + 2],
			ctrl->aim.position, ctrl->aim.position + ctrl->aim.orientation*vec3_forward * 100, 0.005f,
			ctrl->aim.position, ctrl->aim.orientation, input_head()->position,
			button_make_state(skui_controller_trigger_last[i]>0.5f, ctrl->trigger>0.5f),
			ctrl->tracked);
		skui_controller_trigger_last[i] = ctrl->trigger;
		ui_show_ray(skui_hand_interactors[i*3 + 2], 0, false, &skui_ray_visible[i], &skui_ray_active[i]);
	}
}

///////////////////////////////////////////

void ui_core_mouse_step() {
	if (ui_far_interact_enabled() == false) return;

	const pose_t*  head = input_head();
	const mouse_t* m    = input_mouse();
	ray_t ray;
	bool tracked = ray_from_mouse(m->pos, ray);

	vec3 end = ray.pos + ray.dir * 100;
	interactor_update(skui_mouse_interactor,
		ray.pos, end, 0.005f,
		end, quat_lookat(ray.pos, end), end,
		input_key(key_mouse_left), button_make_state(skui_interactors[skui_mouse_interactor].tracked & button_state_active, tracked));
}

///////////////////////////////////////////

void ui_core_update() {
	const matrix *to_local = hierarchy_to_local();

	for (int32_t i = 0; i < skui_interactors.count; i++) {
		/*if (skui_interactors[i].tracked & button_state_active) {
			color32 color_start = {255,255,255,255};
			color32 color_end   = {255,255,255,0};
			if (skui_interactors[i].focused_prev != 0) {
				color_start = {255,255,0,255};
				color_end   = {255,255,0,0};
			}
			if (skui_interactors[i].active_prev != 0) {
				color_start = {0,255,0,255};
				color_end   = {0,255,0,0};
			}
			line_add(skui_interactors[i].capsule_start_world, skui_interactors[i].capsule_end_world, color_start, color_end, 0.003f);

			//char txt[32];
			//snprintf(txt,32, "%.2f", skui_interactors[i].focus_priority);
			//text_add_at(txt,matrix_trs(skui_interactors[i].capsule_start_world, quat_lookat(skui_interactors[i].capsule_start_world, input_head()->position)), 1);
		}*/

		skui_interactors[i].focused_prev_prev   = skui_interactors[i].focused_prev;
		skui_interactors[i].focused_prev        = skui_interactors[i].focused;
		skui_interactors[i].active_prev_prev    = skui_interactors[i].active_prev;
		skui_interactors[i].active_prev         = skui_interactors[i].active;
		skui_interactors[i].orientation_prev    = skui_interactors[i].orientation;
		skui_interactors[i].position_prev       = skui_interactors[i].position;

		skui_interactors[i].focus_priority = FLT_MAX;
		skui_interactors[i].focus_distance = FLT_MAX;
		skui_interactors[i].focused        = 0;
		skui_interactors[i].active         = 0;
		skui_interactors[i].capsule_end    = matrix_transform_pt(*to_local, skui_interactors[i].capsule_end_world);
		skui_interactors[i].capsule_start  = matrix_transform_pt(*to_local, skui_interactors[i].capsule_start_world);
		skui_interactors[i].ray_enabled    = skui_interactors[i].tracked > 0 && skui_interactors[i].tracked && (vec3_dot(skui_interactors[i].capsule_end_world - skui_interactors[i].capsule_start_world, input_head()->position - skui_interactors[i].capsule_start_world) < 0);

		skui_interactors[i].tracked     = button_state_inactive;
		skui_interactors[i].pinch_state = button_state_inactive;
	}

	// auto-switch between hands and controllers
	hand_source_ source = input_hand_source(handed_right);
	if      (source == hand_source_articulated || source == hand_source_overridden)                   skui_input_mode = 0;
	else if (source == hand_source_simulated && device_display_get_type() == display_type_flatscreen) skui_input_mode = 2;
	else                                                                                              skui_input_mode = 1;

	if      (skui_input_mode == 0) { ui_core_hands_step(); }
	if      (skui_input_mode == 1) { ui_core_controllers_step(); }
	else if (skui_input_mode == 2) { ui_core_mouse_step(); }
	skui_finger_radius = input_hand(handed_right)->fingers[1][4].radius;

	// Clear current keyboard ignore elements
	skui_preserve_keyboard_ids_read->clear();
	array_t<id_hash_t>* tmp = skui_preserve_keyboard_ids_read;
	skui_preserve_keyboard_ids_read  = skui_preserve_keyboard_ids_write;
	skui_preserve_keyboard_ids_write = tmp;
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
	int32_t       interactor = -1;

	vec3 start = bounds.center + bounds.dimensions / 2;
	ui_box_interaction_1h(id_hash, interact_type == ui_confirm_push ? interactor_event_poke : interactor_event_pinch,
		start, bounds.dimensions,
		start, bounds.dimensions,
		&focus, &interactor);

	interactor_t *actor = interactor_get(interactor);
	if (actor != nullptr) {
		result = interactor_set_active(actor, id_hash, actor->activation == interactor_activate_position
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

void ui_button_behavior(vec3 window_relative_pos, vec2 size, id_hash_t id, float& out_finger_offset, button_state_& out_button_state, button_state_& out_focus_state, int32_t* out_opt_hand) {
	ui_button_behavior_depth(window_relative_pos, size, id, skui_settings.depth, skui_settings.depth / 2, out_finger_offset, out_button_state, out_focus_state, out_opt_hand);
}

///////////////////////////////////////////

void ui_button_behavior_depth(vec3 window_relative_pos, vec2 size, id_hash_t id, float button_depth, float button_activation_depth, float &out_finger_offset, button_state_ &out_button_state, button_state_ &out_focus_state, int32_t* out_opt_hand) {
	out_button_state = button_state_inactive;
	out_focus_state  = button_state_inactive;
	int32_t interactor = -1;
	vec3    interaction_at;
	button_state_ focus_candidacy = button_state_inactive;

	interactor_plate_1h(id, interactor_event_poke,
		{ window_relative_pos.x, window_relative_pos.y, window_relative_pos.z - button_depth }, { size.x, size.y, button_depth },
		& focus_candidacy, &interactor, &interaction_at);
	interactor_t* actor = interactor_get(interactor);

	// If a hand is interacting, adjust the button surface accordingly
	out_finger_offset = button_depth;
	if (focus_candidacy & button_state_active) {
		bool pressed;
		if (actor->activation == interactor_activate_position) {
			out_finger_offset = -(interaction_at.z + actor->capsule_radius) - window_relative_pos.z;
			pressed = out_finger_offset < button_activation_depth;
		} else {
			pressed = (actor->pinch_state & button_state_active) && actor->focused_prev == id;
			if (pressed) out_finger_offset = 0;
		}
		out_finger_offset = fminf(fmaxf(2 * mm2m, out_finger_offset), button_depth);
		out_button_state  = interactor_set_active(actor, id, pressed);
	} else if (focus_candidacy & button_state_just_inactive) {
		out_button_state = interactor_set_active(actor, id, false);
	}
	
	if (out_button_state & button_state_just_active)
		ui_play_sound_on_off(ui_vis_button, id, hierarchy_to_world_point(ui_layout_last().center));

	if (actor)
		out_focus_state = button_make_state(actor->focused_prev_prev == id, actor->focused_prev == id);
	if (out_opt_hand)
		*out_opt_hand = interactor;
}

///////////////////////////////////////////

void ui_slider_behavior(vec3 window_relative_pos, vec2 size, id_hash_t id, vec2* value, vec2 min, vec2 max, vec2 button_size_visual, vec2 button_size_interact, ui_confirm_ confirm_method, ui_slider_data_t* out) {
	const float snap_scale = 1;
	const float snap_dist  = 7*cm2m;

	// Input comes from the developer and could be any value, so we want to
	// ensure it's clamped to the correct range.
	if (value->x < min.x) value->x = min.x;
	if (value->y < min.y) value->y = min.y;
	if (value->x > max.x) value->x = max.x;
	if (value->y > max.y) value->y = max.y;

	// Find sizes of slider elements
	vec2  range        = max - min;
	vec2  percent      = { range.x == 0 ? 0.5f : (value->x - min.x)/range.x,  range.y == 0 ? 0.5f : (value->y - min.y)/range.y };
	float button_depth = skui_settings.depth;

	// Set up for getting the state of the sliders.
	out->focus_state   = button_state_inactive;
	out->active_state  = button_state_inactive;
	out->interactor    = -1;
	out->finger_offset = button_depth;
	out->button_center = {
		window_relative_pos.x - (percent.x * (size.x - button_size_visual.x) + button_size_visual.x/2.0f),
		window_relative_pos.y - (percent.y * (size.y - button_size_visual.y) + button_size_visual.y/2.0f) };

	vec2 finger_at = {};
	interactor_t* actor = nullptr;
	vec3 activation_size  = vec3{ button_size_interact.x, button_size_interact.y, button_depth };
	vec3 activation_start = { out->button_center.x+activation_size.x/2.0f, out->button_center.y+activation_size.y/2.0f, window_relative_pos.z };
	if (button_size_interact.x == 0 && button_size_interact.y == 0) {
		activation_size  = {size.x, size.y, button_depth};
		activation_start = window_relative_pos;
	}

	if (confirm_method == ui_confirm_push) {
		ui_button_behavior_depth(activation_start, { activation_size.x, activation_size.y }, id, button_depth, button_depth / 2, out->finger_offset, out->active_state, out->focus_state, &out->interactor);

		actor = interactor_get(out->interactor);
		
	} else if (confirm_method == ui_confirm_pinch || confirm_method == ui_confirm_variable_pinch) {
		activation_start.z += skui_settings.depth;
		activation_size.z  += skui_settings.depth;
		ui_box_interaction_1h(id, interactor_event_pinch,
			activation_start, activation_size,
			activation_start, activation_size,
			&out->focus_state, &out->interactor);

		// Pinch confirm uses a handle that the user must pinch, in order to
		// drag it around the slider.
		actor = interactor_get(out->interactor);
		if (actor != nullptr) {
			out->active_state = interactor_set_active(actor, id, actor->pinch_state & button_state_active);
		}
	}

	if (actor != nullptr) {
		if (actor->type == interactor_type_point) {
			vec3 local_pos = hierarchy_to_local_point(actor->position);
			finger_at = { local_pos.x, local_pos.y };
		} else {
			ray_t ray = { actor->capsule_start, actor->capsule_end - actor->capsule_start };
			float t;
			if (ray_intersect_plane(ray, {0,0,window_relative_pos.z}, { 0,0,1 }, t)) {
				vec3 pt = ray.pos + ray.dir * t;
				finger_at = { pt.x, pt.y };
			}
		}
	}

	vec2 new_percent = percent;
	if (out->active_state & button_state_active) {
		vec2 pos_in_slider = {
			(float)fmin(1, fmax(0, ((window_relative_pos.x-button_size_visual.x/2)-finger_at.x) / (size.x-button_size_visual.x))),
			(float)fmin(1, fmax(0, ((window_relative_pos.y-button_size_visual.y/2)-finger_at.y) / (size.y-button_size_visual.y)))};
		vec2 new_val = min + pos_in_slider*range;

		new_percent = {
			range.x == 0 ? 0.5f : (new_val.x - min.x) / range.x,
			range.y == 0 ? 0.5f : (new_val.y - min.y) / range.y };
		out->button_center = {
			window_relative_pos.x - (new_percent.x * (size.x - button_size_visual.x) + button_size_visual.x/2.0f),
			window_relative_pos.y - (new_percent.y * (size.y - button_size_visual.y) + button_size_visual.y/2.0f) };

		// Do this down here so we can calculate old_percent above
		*value = new_val;
	}
}

///////////////////////////////////////////

bool32_t _ui_handle_begin(id_hash_t id, pose_t &handle_pose, bounds_t handle_bounds, bool32_t draw, ui_move_ move_type, ui_gesture_ allowed_gestures) {
	bool  result      = false;
	float color_blend = 0;

	skui_last_element = id;
	if (skui_preserve_keyboard_stack.last()) {
		skui_preserve_keyboard_ids_write->add(id);
	}

	matrix to_handle_parent_local       = *hierarchy_to_local();
	matrix handle_parent_local_to_world = *hierarchy_to_world();
	ui_push_surface(handle_pose);

	interactor_event_ event_mask = (interactor_event_)0;
	if (allowed_gestures & ui_gesture_pinch) event_mask = (interactor_event_)(event_mask | interactor_event_pinch);
	if (allowed_gestures & ui_gesture_grip ) event_mask = (interactor_event_)(event_mask | interactor_event_grip );

	if (!ui_is_enabled() || move_type == ui_move_none) {
		interactor_set_focus(nullptr, id, false, 0, 0, vec3_zero);
	} else {
		for (int32_t i = 0; i < skui_interactors.count; i++) {
			interactor_t* actor = &skui_interactors[i];
			// Skip this if something else has some focus!
			if (interactor_is_preoccupied(actor, id, event_mask, false))
				continue;

			// Check to see if the handle has focus
			bool  has_hand_attention  = actor->active_prev == id;
			bool  is_far_interact     = false;
			float hand_attention_dist = 0;
			vec3  at;
			if (interactor_check_box(actor, handle_bounds, &at, &hand_attention_dist)) {
				has_hand_attention = true;

				if (actor->pinch_state & button_state_just_active && actor->focused_prev == id) {
					ui_play_sound_on(ui_vis_handle, hierarchy_to_world_point(at));

					actor->active = id;
					actor->interaction_start_position      = actor->position;
					actor->interaction_start_orientation   = actor->orientation;
					actor->interaction_start_motion_anchor = actor->motion_anchor;

					actor->interaction_pt_position         = handle_pose.position;
					actor->interaction_pt_orientation      = handle_pose.orientation;
					actor->interaction_pt_pivot            = at;

					actor->interaction_intersection_local  = matrix_transform_pt(matrix_invert(matrix_trs(actor->position, actor->orientation)), hierarchy_to_world_point(at));
				}
			} else { at = actor->interaction_pt_pivot; }
			button_state_ focused = interactor_set_focus(actor, id, has_hand_attention, hand_attention_dist + 0.1f, hand_attention_dist, at);


			// This waits until the window has been focused for a frame,
			// otherwise the handle UI may try and use a frame of focus to move
			// around a bit.
			if (actor->focused_prev == id) {
				color_blend = 1;
				
				if (actor->active_prev == id || actor->active == id) {
					result = true;
					actor->active  = id;
					actor->focused = id;

					quat dest_rot = quat_identity;
					switch (move_type) {
					case ui_move_exact: {
						dest_rot = actor->interaction_pt_orientation * quat_difference(actor->interaction_start_orientation, actor->orientation);
					} break;
					case ui_move_face_user: {
						if (device_display_get_type() == display_type_flatscreen) {
							// If we're on a flat screen, facing the window is
							// a better experience than facing the user.
							dest_rot = quat_from_angles(0, 180, 0) * input_head()->orientation;
						} else {
							// We can't use the head position directly, it's
							// more of a device position that matches the
							// center of the eyes, and not the center of the
							// head.
							const float head_center_dist = 5    * cm2m; // Quarter head length (20cm front to back)
							const float head_height      = 7.5f * cm2m; // Almost quarter head height (25cm top to bottom)
							vec3 eye_center  = input_head()->position;
							vec3 head_center = eye_center  + input_head()->orientation * vec3{0, 0, head_center_dist};
							vec3 face_point  = head_center + vec3{0, -head_height, 0};

							// Previously, facing happened from a point
							// influenced by the hand-grip position:
							// vec3 world_handle_center = { handle_pose.position.x, local_pt[i].y, handle_pose.position.z };
							vec3 world_handle_center = hierarchy_to_world_point(handle_bounds.center);
							vec3 world_pt            = hierarchy_to_world_point(actor->interaction_pt_pivot);

							float head_xz_lerp = fminf(1, vec2_distance_sq({ face_point.x, face_point.z }, { world_pt.x, world_pt.z }) / 0.1f);
							vec3  look_from    = vec3_lerp(world_pt, world_handle_center, head_xz_lerp);

							dest_rot = quat_lookat_up(look_from, face_point, vec3_up);
						}
					} break;
					case ui_move_pos_only: { dest_rot = actor->interaction_pt_orientation; } break;
					default:               { dest_rot = actor->interaction_pt_orientation; log_err("Unimplemented move type!"); } break;
					}

					// Amplify the movement in and out, so that objects at a
					// distance can be manipulated easier.
					const float amp = 3;
					float amplify_factor = 
						vec3_distance(actor->position,                   actor->motion_anchor) /
						vec3_distance(actor->interaction_start_position, actor->interaction_start_motion_anchor);
					amplify_factor = fmaxf(0, (amplify_factor - 1) * amp + 1);
					// Disable amplification for now
					amplify_factor = 1;

					vec3 pivot_new_position  = matrix_transform_pt(matrix_trs(actor->position, actor->orientation, vec3_one * amplify_factor), actor->interaction_intersection_local);
					vec3 handle_world_offset = dest_rot * (-actor->interaction_pt_pivot);
					vec3 dest_pos            = pivot_new_position + handle_world_offset;

					// Transform from world space, to the space the handle is in
					dest_pos = matrix_transform_pt  (to_handle_parent_local, dest_pos);
					dest_rot = matrix_transform_quat(to_handle_parent_local, dest_rot);

					handle_pose.position    = vec3_lerp (handle_pose.position,    dest_pos, 0.6f);
					handle_pose.orientation = quat_slerp(handle_pose.orientation, dest_rot, 0.4f);

					if (actor->pinch_state & button_state_just_inactive) {
						actor->active = 0;
						ui_play_sound_off(ui_vis_handle, hierarchy_to_world_point(actor->interaction_pt_pivot));
					}
					ui_pop_surface();
					ui_push_surface(handle_pose);
				}
			}
		}
	}

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

bool32_t ui_handle_begin(const char *text, pose_t &movement, bounds_t handle, bool32_t draw, ui_move_ move_type, ui_gesture_ allowed_gestures) {
	return _ui_handle_begin(ui_stack_hash(text), movement, handle, draw, move_type, allowed_gestures);
}
bool32_t ui_handle_begin_16(const char16_t *text, pose_t &movement, bounds_t handle, bool32_t draw, ui_move_ move_type, ui_gesture_ allowed_gestures) {
	return _ui_handle_begin(ui_stack_hash_16(text), movement, handle, draw, move_type, allowed_gestures);
}

///////////////////////////////////////////

void ui_handle_end() {
	ui_pop_surface();
}

///////////////////////////////////////////

int32_t interactor_create(interactor_type_ type, interactor_event_ events, interactor_activate_ activation) {
	interactor_t result = {};
	result.type   = type;
	result.events = events;
	result.activation = activation;
	result.min_distance = -1000000;
	return skui_interactors.add(result);
}

///////////////////////////////////////////

void interactor_update(int32_t interactor, vec3 capsule_start, vec3 capsule_end, float capsule_radius, vec3 motion_pos, quat motion_orientation, vec3 motion_anchor, button_state_ active, button_state_ tracked) {
	if (interactor < 0 || interactor >= skui_interactors.count) return;
	interactor_t *actor = &skui_interactors[interactor];
	actor->tracked             = tracked;
	actor->pinch_state         = active;

	if (tracked & button_state_active) {
		actor->capsule_start_world = actor->capsule_start = capsule_start;
		actor->capsule_end_world   = actor->capsule_end   = capsule_end;
		actor->capsule_radius      = capsule_radius;
		actor->position            = motion_pos;
		actor->orientation         = motion_orientation;
		actor->motion_anchor       = motion_anchor;
	}

	// Don't let the hand trigger things while popping in and out of
	// tracking
	if (actor->tracked & button_state_just_active) {
		actor->orientation_prev = actor->orientation;
		actor->position_prev    = actor->position;
	}
}

///////////////////////////////////////////

void interactor_min_distance_set(int32_t interactor, float min_distance) {
	if (interactor < 0 || interactor >= skui_interactors.count) return;
	skui_interactors[interactor].min_distance = min_distance;
}

///////////////////////////////////////////

interactor_t* interactor_get(int32_t interactor) {
	return interactor >= 0 
		? &skui_interactors[interactor]
		: nullptr;
}

///////////////////////////////////////////

bool32_t interactor_check_box(const interactor_t* actor, bounds_t box, vec3* out_at, float* out_priority) {
	*out_priority = FLT_MAX;
	*out_at       = vec3_zero;

	if (!(actor->tracked & button_state_active))
		return false;

	if (skui_show_volumes)
		render_add_mesh(skui_box_dbg, skui_mat_dbg, matrix_trs(box.center, quat_identity, box.dimensions));

	bool32_t result = bounds_capsule_intersect(box, actor->capsule_start, actor->capsule_end, actor->capsule_radius, out_at);
	if (actor->type == interactor_type_point) *out_at = hierarchy_to_local_point(actor->position);
	if (result) {
		*out_priority = bounds_sdf(box, *out_at) + vec3_distance(*out_at, actor->capsule_start);
	}
	return result;
}

///////////////////////////////////////////

void interactor_plate_1h(id_hash_t id, interactor_event_ event_mask, vec3 plate_start, vec3 plate_size, button_state_ *out_focus_candidacy, int32_t *out_interactor, vec3 *out_interaction_at_local) {
	*out_interactor           = -1;
	*out_focus_candidacy      = button_state_inactive;
	*out_interaction_at_local = vec3_zero;

	skui_last_element = id;

	if (!ui_is_enabled()) return;
	if (skui_preserve_keyboard_stack.last()) {
		skui_preserve_keyboard_ids_write->add(id);
	}

	for (int32_t i = 0; i < skui_interactors.count; i++) {
		interactor_t *actor = &skui_interactors[i];
		if (interactor_is_preoccupied(actor, id, event_mask, false))
			continue;

		// Button interaction focus is detected out in front of the button to 
		// prevent 'reverse' or 'side' presses where the finger comes from the
		// back or side.
		//
		// Once focused is gained, interaction is tracked within a volume that
		// extends from the detection plane, to a good distance through the
		// button's back. This is to help when the user's finger inevitably goes
		// completely through the button. Width and height is added to this 
		// volume to account for vertical or horizontal movement during a press,
		// such as the downward motion often accompanying a 'poke' motion.

		float    surface_offset = actor->activation == interactor_activate_position ? actor->capsule_radius*2 : 0;

		bool     was_focused = actor->focused_prev == id;
		bool     was_active  = actor->active_prev  == id;
		float    depth  = fmaxf(0.0001f, 8 * plate_size.z);
		bounds_t bounds = was_focused && actor->activation == interactor_activate_position
			? size_box({ plate_start.x, plate_start.y, (plate_start.z + depth) - surface_offset }, { plate_size.x, plate_size.y, depth })
			: size_box({ plate_start.x, plate_start.y, plate_start.z           - surface_offset }, { plate_size.x, plate_size.y, 0.0001f });

		float         priority = 0;
		vec3          interact_at = vec3_zero;
		bool          facing   = actor->type == interactor_type_line ? vec3_dot(actor->capsule_end - actor->capsule_start, vec3_forward) < 0 : true;
		bool          in_box   = facing && interactor_check_box(actor, bounds, &interact_at, &priority);
		button_state_ focus    = interactor_set_focus(actor, id, in_box || (actor->activation == interactor_activate_state && was_active), priority, priority, plate_start-vec3{plate_size.x/2, plate_size.y/2, 0});
		if (focus != button_state_inactive) {
			*out_interactor           = i;
			*out_focus_candidacy      = focus;
			*out_interaction_at_local = interact_at;
		}
	}

	if (*out_interactor == -1)
		*out_interactor = interactor_last_focused(id);
}

///////////////////////////////////////////

void ui_box_interaction_1h(id_hash_t id, interactor_event_ event_mask, vec3 box_unfocused_start, vec3 box_unfocused_size, vec3 box_focused_start, vec3 box_focused_size, button_state_ *out_focus_candidacy, int32_t *out_interactor) {
	*out_interactor      = -1;
	*out_focus_candidacy = button_state_inactive;
	
	skui_last_element = id;

	if (!ui_is_enabled()) return;
	if (skui_preserve_keyboard_stack.last()) {
		skui_preserve_keyboard_ids_write->add(id);
	}

	for (int32_t i = 0; i < skui_interactors.count; i++) {
		interactor_t* actor = &skui_interactors[i];
		if (interactor_is_preoccupied(actor, id, event_mask, false))
			continue;

		bool     was_focused = actor->focused_prev == id;
		bool     was_active  = actor->active_prev == id;
		bounds_t bounds      = was_focused
			? ui_size_box(box_focused_start,   box_focused_size)
			: ui_size_box(box_unfocused_start, box_unfocused_size);

		vec3  at;
		float priority;
		bool  in_box = interactor_check_box(actor, bounds, &at, &priority);
		button_state_ focus = interactor_set_focus(actor, id, in_box || (actor->activation == interactor_activate_state && was_active), priority, priority, bounds.center);
		if (focus != button_state_inactive) {
			*out_interactor      = i;
			*out_focus_candidacy = focus;
		}
	}

	if (*out_interactor == -1)
		*out_interactor = interactor_last_focused(id);
}

///////////////////////////////////////////

void ui_push_surface(pose_t surface_pose, vec3 layout_start, vec2 layout_dimensions) {
	vec3   right = surface_pose.orientation * vec3_right;
	matrix trs   = matrix_trs(surface_pose.position + right*layout_start, surface_pose.orientation);
	hierarchy_push(trs);
	ui_layout_push(layout_start, layout_dimensions, true);

	const matrix *to_local = hierarchy_to_local();
	for (int32_t i = 0; i < skui_interactors.count; i++) {
		skui_interactors[i].capsule_end   = matrix_transform_pt(*to_local, skui_interactors[i].capsule_end_world);
		skui_interactors[i].capsule_start = matrix_transform_pt(*to_local, skui_interactors[i].capsule_start_world);
	}
}

///////////////////////////////////////////

void ui_pop_surface() {
	ui_layout_pop();
	hierarchy_pop();

	const matrix *to_local = hierarchy_to_local();
	for (int32_t i = 0; i < skui_interactors.count; i++) {
		skui_interactors[i].capsule_end   = matrix_transform_pt(*to_local, skui_interactors[i].capsule_end_world);
		skui_interactors[i].capsule_start = matrix_transform_pt(*to_local, skui_interactors[i].capsule_start_world);
	}
}

///////////////////////////////////////////

button_state_ interactor_set_focus(interactor_t* interactor, id_hash_t for_el_id, bool32_t focused, float priority, float distance, vec3 element_center_local) {
	if (interactor == nullptr) return button_state_inactive;
	bool was_focused = interactor->focused_prev == for_el_id;
	bool is_focused  = false;

	if (focused && priority <= interactor->focus_priority) {
		if (distance >= interactor->min_distance || interactor->active_prev == for_el_id) {
			is_focused = focused;
			interactor->focused = for_el_id;
		} else {
			is_focused = false;
			interactor->focused = 0;
		}
		interactor->focus_priority     = priority;
		interactor->focus_distance     = distance;
		interactor->focus_center_world = hierarchy_to_world_point( element_center_local );
	}

	button_state_ result = button_state_inactive;
	if ( is_focused                ) result  = button_state_active;
	if ( is_focused && !was_focused) result |= button_state_just_active;
	if (!is_focused &&  was_focused) result |= button_state_just_inactive;
	return result;
}

///////////////////////////////////////////

button_state_ interactor_set_active(interactor_t* interactor, id_hash_t for_el_id, bool32_t active) {
	if (interactor == nullptr) return button_state_inactive;

	bool was_active = interactor->active_prev == for_el_id;
	bool is_active  = false;

	if (active && (was_active || interactor->focused_prev == for_el_id || interactor->focused == for_el_id)) {
		is_active = active;
		interactor->active = for_el_id;
	}

	button_state_ result = button_state_inactive;
	if ( is_active               ) result  = button_state_active;
	if ( is_active && !was_active) result |= button_state_just_active; 
	if (!is_active &&  was_active) result |= button_state_just_inactive;
	return result;
}

///////////////////////////////////////////

bool32_t ui_id_focused(id_hash_t id) {
	for (int32_t i = 0; i < skui_interactors.count; i++)
		if (skui_interactors[i].focused_prev == id) return true;
	return false;
}

///////////////////////////////////////////

button_state_ ui_id_focus_state(id_hash_t id) {
	bool was_focused = false;
	bool is_focused  = false;

	for (int32_t i = 0; i < skui_interactors.count; i++) {
		if (skui_interactors[i].focused_prev      == id) is_focused  = true;
		if (skui_interactors[i].focused_prev_prev == id) was_focused = true;
	}
	return button_make_state(was_focused, is_focused);
}

///////////////////////////////////////////

bool32_t ui_id_active(id_hash_t id) {
	for (int32_t i = 0; i < skui_interactors.count; i++)
		if (skui_interactors[i].active_prev == id) return true;
	return false;
}

///////////////////////////////////////////

button_state_ ui_id_active_state(id_hash_t id) {
	bool was_active = false;
	bool is_active  = false;

	for (int32_t i = 0; i < skui_interactors.count; i++) {
		if (skui_interactors[i].active_prev      == id) is_active  = true;
		if (skui_interactors[i].active_prev_prev == id) was_active = true;
	}
	return button_make_state(was_active, is_active);
}

///////////////////////////////////////////

int32_t ui_id_active_interactor(id_hash_t id) {
	for (int32_t i = 0; i < skui_interactors.count; i++)
		if (skui_interactors[i].active_prev == id) return i;
	return -1;
}

///////////////////////////////////////////

void ui_show_volumes(bool32_t show) {
	skui_show_volumes = show;
}

///////////////////////////////////////////

bool32_t interactor_is_preoccupied(const interactor_t* interactor, id_hash_t for_el_id, interactor_event_ event_mask, bool32_t include_focused) {
	return
		(interactor->events & event_mask) == 0 ||
		(include_focused && interactor->focused_prev != 0 && interactor->focused_prev != for_el_id) ||
		(interactor->active_prev != 0 && interactor->active_prev != for_el_id);
}

///////////////////////////////////////////

int32_t ui_last_active_hand(id_hash_t for_el_id) {
	for (int32_t i = 0; i < skui_interactors.count; i++)
		if (skui_interactors[i].active_prev == for_el_id) return i;
	return -1;
}

///////////////////////////////////////////

int32_t interactor_last_focused(id_hash_t for_el_id) {
	for (int32_t i = 0; i < skui_interactors.count; i++)
		if (skui_interactors[i].focused_prev == for_el_id) return i;
	return -1;
}

///////////////////////////////////////////

bool32_t ui_is_interacting(handed_ hand) {
	return skui_interactors[hand].active_prev != 0 || skui_interactors[hand].focused_prev != 0;
}

///////////////////////////////////////////

button_state_ ui_last_element_hand_active(handed_ hand) {
	return button_make_state(
		skui_interactors[hand].active_prev == skui_last_element,
		skui_interactors[hand].active      == skui_last_element);
}

///////////////////////////////////////////

button_state_ ui_last_element_hand_focused(handed_ hand) {
	// Because focus can change at any point during the frame, we'll check
	// against the last two frame's focus ids, which are set in stone after the
	// frame ends.
	return button_make_state(
		skui_interactors[hand].focused_prev_prev == skui_last_element,
		skui_interactors[hand].focused_prev      == skui_last_element);
}

///////////////////////////////////////////

button_state_ ui_last_element_active() {
	bool was_active = false;
	bool is_active  = false;
	for (int32_t i = 0; i < skui_interactors.count; i++) {
		was_active = was_active || (skui_interactors[i].active_prev == skui_last_element);
		is_active  = is_active  || (skui_interactors[i].active      == skui_last_element);
	}
	return button_make_state(was_active, is_active);
}

///////////////////////////////////////////

button_state_ ui_last_element_focused() {
	bool was_focused = false;
	bool is_focused  = false;
	for (int32_t i = 0; i < skui_interactors.count; i++) {
		// Because focus can change at any point during the frame, we'll check
		// against the last two frame's focus ids, which are set in stone after the
		// frame ends.
		was_focused = was_focused || (skui_interactors[i].focused_prev_prev == skui_last_element);
		is_focused  = is_focused  || (skui_interactors[i].focused_prev      == skui_last_element);
	}
	return button_make_state(was_focused, is_focused);
}

///////////////////////////////////////////

id_hash_t hash_fnv64_string_16(const char16_t* string, id_hash_t start_hash = HASH_FNV64_START) {
	id_hash_t hash = start_hash;
	while (*string != '\0') {
		hash = (hash ^ ((*string & 0xFF00) >> 2)) * 1099511628211;
		hash = (hash ^ ( *string & 0x00FF      )) * 1099511628211;
		string++;
	}
	return hash;
}

///////////////////////////////////////////

id_hash_t ui_stack_hash(const char *string) {
	return skui_id_stack.count > 0 
		? hash_fnv64_string(string, skui_id_stack.last())
		: hash_fnv64_string(string);
}

///////////////////////////////////////////

id_hash_t ui_stack_hash_16(const char16_t *string) {
	return skui_id_stack.count > 0 
		? hash_fnv64_string_16(string, skui_id_stack.last())
		: hash_fnv64_string_16(string);
}

///////////////////////////////////////////

id_hash_t ui_stack_hashi(int32_t id) {
	return skui_id_stack.count > 0 
		? hash_fnv64_data(&id, sizeof(int32_t), skui_id_stack.last())
		: hash_fnv64_data(&id, sizeof(int32_t));
}

///////////////////////////////////////////

id_hash_t ui_push_id(const char *id) {
	id_hash_t result = ui_stack_hash(id);
	skui_id_stack.add({ result });
	return result;
}

///////////////////////////////////////////

id_hash_t ui_push_id_16(const char16_t *id) {
	id_hash_t result = ui_stack_hash(id);
	skui_id_stack.add({ result });
	return result;
}

///////////////////////////////////////////

id_hash_t ui_push_idi(int32_t id) {
	id_hash_t result = ui_stack_hashi(id);
	skui_id_stack.add({ result });
	return result;
}

///////////////////////////////////////////

void ui_pop_id() {
	skui_id_stack.pop();
}

///////////////////////////////////////////

void ui_push_enabled(bool32_t enabled, hierarchy_parent_ parent_behavior) {
	skui_enabled_stack.add(parent_behavior == hierarchy_parent_ignore
		? enabled
		: ((bool)enabled == true) && ((bool)ui_is_enabled() == true));
}

///////////////////////////////////////////

void ui_pop_enabled() {
	if (skui_enabled_stack.count <= 1) {
		log_errf("Tried to pop too many %s! Do you have a push/pop mismatch?", "'enabled'");
		return;
	}
	skui_enabled_stack.pop();
}

///////////////////////////////////////////

bool32_t ui_is_enabled() {
	return skui_enabled_stack.count == 0
		? true
		: skui_enabled_stack.last();
}

///////////////////////////////////////////

void ui_push_preserve_keyboard(bool32_t preserve_keyboard){
	skui_preserve_keyboard_stack.add(preserve_keyboard);
}

///////////////////////////////////////////

void ui_pop_preserve_keyboard(){
	if (skui_preserve_keyboard_stack.count <= 1) {
		log_errf("Tried to pop too many %s! Do you have a push/pop mismatch?", "preserve keyboards");
		return;
	}
	skui_preserve_keyboard_stack.pop();
}

///////////////////////////////////////////

bool32_t ui_keyboard_focus_lost(id_hash_t focused_id) {
	for (int32_t i = 0; i < skui_interactors.count; i++) {
		id_hash_t active_id = skui_interactors[i].active_prev;
		if (active_id != 0 && active_id != focused_id && skui_preserve_keyboard_ids_read->index_of(active_id) < 0)
			return true;
	}
	return false;
}

}