/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

#include "ui_core.h"
#include "ui_layout.h"
#include "ui_theming.h"
#include "interactors.h"

#include "../_stereokit.h"
#include "../libraries/array.h"
#include "../systems/input.h"
#include "../hands/input_hand.h"
#include "../sk_math.h"

#include <float.h>

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

enum interact_mode_ {
	interact_mode_none,
	interact_mode_hands,
	interact_mode_controllers,
	interact_mode_mouse,
	interact_mode_eyes,
};

struct interact_mode_hands_t {
	interactor_id poke [2];
	interactor_id pinch[2];
	interactor_id far  [2];
	float         ray_active [2];
	float         ray_visible[2];
	bool          prev_active[2];
};

struct interact_mode_controllers_t {
	interactor_id far[2];
	float         ray_active  [2];
	float         ray_visible [2];
	float         trigger_last[2];
};

struct interact_mode_mouse_t {
	interactor_id interactor;
};

struct interact_mode_eyes_t {
	interactor_id interactor;
	vec3          hand_motion_prev[2];
	bool          active_prev;
};

///////////////////////////////////////////

interact_mode_ skui_input_mode;
interact_mode_ skui_input_mode_curr;

interact_mode_hands_t       skui_hands;
interact_mode_controllers_t skui_controllers;
interact_mode_mouse_t       skui_mouse;
interact_mode_eyes_t        skui_eyes;

///////////////////////////////////////////

void interact_mode_hands_start      (interact_mode_hands_t*       ref_hands);
void interact_mode_hands_stop       (interact_mode_hands_t*       ref_hands);
void interact_mode_hands_step       (interact_mode_hands_t*       ref_hands);
void interact_mode_controllers_start(interact_mode_controllers_t* ref_controllers);
void interact_mode_controllers_stop (interact_mode_controllers_t* ref_controllers);
void interact_mode_controllers_step (interact_mode_controllers_t* ref_controllers);
void interact_mode_mouse_start      (interact_mode_mouse_t*       ref_mouse);
void interact_mode_mouse_stop       (interact_mode_mouse_t*       ref_mouse);
void interact_mode_mouse_step       (interact_mode_mouse_t*       ref_mouse);
void interact_mode_eyes_start       (interact_mode_eyes_t*        ref_eyes);
void interact_mode_eyes_stop        (interact_mode_eyes_t*        ref_eyes);
void interact_mode_eyes_step        (interact_mode_eyes_t*        ref_eyes);

///////////////////////////////////////////

void ui_core_init() {
	interaction_init();

	skui_hands       = {};
	skui_controllers = {};
	skui_mouse       = {};

	skui_input_mode_curr = interact_mode_none;
	skui_input_mode = device_display_get_type() == display_type_flatscreen
		? interact_mode_mouse
		: interact_mode_hands;
}

///////////////////////////////////////////

void ui_core_shutdown() {
	if      (skui_input_mode_curr == interact_mode_controllers) interact_mode_controllers_start(&skui_controllers);
	else if (skui_input_mode_curr == interact_mode_hands)       interact_mode_hands_start      (&skui_hands);
	else if (skui_input_mode_curr == interact_mode_mouse)       interact_mode_mouse_start      (&skui_mouse);
	else if (skui_input_mode_curr == interact_mode_eyes)        interact_mode_eyes_start       (&skui_eyes);

	interaction_shutdown();
}

///////////////////////////////////////////

void ui_show_ray(interactor_id interactor, float skip, bool hide_inactive, float *ref_visible_amt, float *ref_active_amt) {
	interactor_t* actor = interactor_get(interactor);
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
	vec3  uncentered_dir = vec3_normalize(actor->capsule_end_world - actor->motion.position);
	vec3  centered_dir   = uncentered_dir;
	if (actor->focused_prev != 0) {
		//vec3 pt = matrix_transform_pt(pose_matrix(actor->focus_pose_world), actor->focus_bounds_local.center);
		vec3 pt = matrix_transform_pt(pose_matrix(actor->focus_pose_world), actor->focus_bounds_local.center + actor->focus_intersection_local);
		//line_add_axis(pose_t{ pt, actor->focus_pose_world.orientation }, 0.04f);
		length       = vec3_distance (pt,  actor->motion.position);
		centered_dir = vec3_normalize(pt - actor->motion.position);
	}
	length = math_lerp(0.35f, length, visibility);
	length = fmaxf(0, length - skip);

	float alpha = 0.35f + active * 0.65f;
	if (hide_inactive) alpha *= visibility;

	const int32_t ct = 20;
	const float   ray_snap = 1.0f; // 0.2f
	line_point_t pts[ct];
	for (int32_t i = 0; i < ct; i += 1) {
		float pct   = (float)i / (float)(ct - 1);
		float blend = pct * pct * pct * ray_snap;
		float d     = skip + pct * length;
		
		//actor->interaction_intersection_local
		float pct_i = 1 - pct;
		float curve = math_lerp(
			sinf(pct_i * pct_i * 3.14159f),
			fminf(1,sinf(pct * pct * 3.14159f)*1.5f), active);
		float width = (0.002f + curve * 0.003f) * visibility;
		pts[i] = line_point_t{ actor->motion.position + vec3_lerp(uncentered_dir*d, centered_dir*d, blend), width, color32{ 255,255,255,(uint8_t)(curve*alpha*255) } };
	}
	line_add_listv(pts, ct);
}

///////////////////////////////////////////
// Hand Interactors
///////////////////////////////////////////

void interact_mode_hands_start(interact_mode_hands_t *ref_hands) {
	for (int32_t h = 0; h < handed_max; h++) {
		ref_hands->poke [h] = interactor_create(interactor_type_point, interactor_event_poke,  interactor_activation_position, 1000+h, 0, 0);
		ref_hands->pinch[h] = interactor_create(interactor_type_point, interactor_event_pinch, interactor_activation_state,    1000+h, 0, 0);
		ref_hands->far  [h] = interactor_create(interactor_type_line, (interactor_event_)(interactor_event_poke | interactor_event_pinch), interactor_activation_state, 1000+h, 0.01f, 2);
		ref_hands->ray_active [h] = 0;
		ref_hands->ray_visible[h] = 0;
	}
}

///////////////////////////////////////////

void interact_mode_hands_stop(interact_mode_hands_t* ref_hands) {
	for (int32_t h = 0; h < handed_max; h++) {
		interactor_destroy(ref_hands->poke [h]);
		interactor_destroy(ref_hands->pinch[h]);
		interactor_destroy(ref_hands->far  [h]);
	}
	*ref_hands = {};
}

///////////////////////////////////////////

void interact_mode_hands_step(interact_mode_hands_t* ref_hands) {
	for (int32_t i = 0; i < handed_max; i++) {
		const hand_t* hand = input_hand((handed_)i);

		// Poke
		interactor_id id         = ref_hands->poke[i];
		interactor_t* interactor = interactor_get(id);
		interactor_radius_set(id, hand->fingers[1][4].radius);
		interactor_update    (id,
			(hand->tracked_state & button_state_just_active) ? hand->fingers[1][4].position : interactor->capsule_end_world, hand->fingers[1][4].position,
			pose_t{ hand->fingers[1][4].position, hand->palm.orientation }, hand->fingers[1][4].position, vec3_zero,
			button_state_inactive, hand->tracked_state);

		// Pinch
		id = ref_hands->pinch[i];
		interactor_radius_set(id, hand->fingers[1][4].radius);
		interactor_update    (id,
			hand->fingers[0][4].position, hand->fingers[1][4].position,
			pose_t{ hand->pinch_pt, hand->palm.orientation }, hand->pinch_pt, vec3_zero,
			hand->pinch_state, hand->tracked_state);

		// Hand ray
		if (ui_far_interact_enabled()) {
			pose_t head      = input_head();
			float  hand_dist = vec3_distance(hand->palm.position, head.position + vec3{0,-0.12f,0});

			// Pinches can only start when aim_ready, but should remain true
			// as long as the pinch remains active
			bool is_pinched   = (hand->pinch_state & button_state_active) > 0;
			bool just_pinched = (hand->pinch_state & button_state_just_active) > 0;
			bool aim_ready    = (hand->aim_ready   & button_state_active) > 0;
			bool is_active    = (ref_hands->prev_active[i] && is_pinched) || (aim_ready && just_pinched);
			button_state_ far_pinch_state = button_make_state(ref_hands->prev_active[i], is_active);
			ref_hands->prev_active[i] = is_active;

			id = ref_hands->far[i];
			interactor_min_distance_set(id, math_lerp(0.35f, 0.20f, math_saturate((hand_dist - 0.1f) / 0.4f)));
			interactor_update          (id,
				hand->aim.position, hand->aim.position + hand->aim.orientation * vec3_forward * 100,
				hand->aim, head.position + vec3{0,-0.12f,0}, vec3_zero,
				far_pinch_state, hand->aim_ready);
			ui_show_ray(id, 0.07f, true, &ref_hands->ray_visible[i], &ref_hands->ray_active[i]);
		}
	}
}

///////////////////////////////////////////
// Controller Interactors
///////////////////////////////////////////

void interact_mode_controllers_start(interact_mode_controllers_t* ref_controllers) {
	for (int32_t h = 0; h < handed_max; h++) {
		ref_controllers->far[h] = interactor_create(interactor_type_line, (interactor_event_)(interactor_event_poke | interactor_event_pinch), interactor_activation_state, -1, 0.005f, 2);
		ref_controllers->ray_active [h] = 0;
		ref_controllers->ray_visible[h] = 0;
	}
}

///////////////////////////////////////////

void interact_mode_controllers_stop(interact_mode_controllers_t* ref_controllers) {
	for (int32_t h = 0; h < handed_max; h++) {
		interactor_destroy(ref_controllers->far[h]);
	}
	*ref_controllers = {};
}

///////////////////////////////////////////

void interact_mode_controllers_step(interact_mode_controllers_t* ref_controllers) {
	if (ui_far_interact_enabled() == false) return;

	pose_t head = input_head();
	for (int32_t i = 0; i < handed_max; i++) {
		const controller_t *ctrl = input_controller((handed_)i);

		// controller ray

		interactor_id id = ref_controllers->far[i];
		interactor_update(id,
			ctrl->aim.position, ctrl->aim.position + ctrl->aim.orientation*vec3_forward * 100,
			ctrl->aim, head.position + vec3{ 0,-0.12f,0 }, vec3{ ctrl->stick.x, ctrl->stick.y, 0 }* 0.02f,
			button_make_state(ref_controllers->trigger_last[i]>0.5f, ctrl->trigger>0.5f),
			ctrl->tracked);
		ref_controllers->trigger_last[i] = ctrl->trigger;
		ui_show_ray(id, 0, false, &ref_controllers->ray_visible[i], &ref_controllers->ray_active[i]);
	}
}

///////////////////////////////////////////
// Mouse Interactors
///////////////////////////////////////////

void interact_mode_mouse_start(interact_mode_mouse_t* ref_mouse) {
	ref_mouse->interactor = interactor_create(interactor_type_line, (interactor_event_)(interactor_event_poke | interactor_event_pinch), interactor_activation_state, -1, 0.005f, 1);
}

///////////////////////////////////////////

void interact_mode_mouse_stop(interact_mode_mouse_t* ref_mouse) {
	interactor_destroy(ref_mouse->interactor);
	*ref_mouse = {};
}

///////////////////////////////////////////

void interact_mode_mouse_step(interact_mode_mouse_t *ref_mouse) {
	if (ui_far_interact_enabled() == false) return;

	const mouse_t* m = input_mouse();
	ray_t ray;
	bool  tracked = ray_from_mouse(m->pos, ray);

	vec3 end = ray.pos + ray.dir * 100;
	interactor_update(ref_mouse->interactor,
		ray.pos, end,
		pose_t{ end, quat_lookat(ray.pos, end) }, end, vec3{ m->scroll_change / -6000.0f, 0, 0 },
		input_key(key_mouse_left), button_make_state(interactor_get(ref_mouse->interactor)->tracked & button_state_active, tracked));
}

///////////////////////////////////////////
// Eye Interactors
///////////////////////////////////////////

void interact_mode_eyes_start(interact_mode_eyes_t* ref_eyes) {
	ref_eyes->interactor = interactor_create(interactor_type_line, (interactor_event_)(interactor_event_poke | interactor_event_pinch), interactor_activation_state, -1, 0.005f, 1);
}

///////////////////////////////////////////

void interact_mode_eyes_stop(interact_mode_eyes_t* ref_eyes) {
	interactor_destroy(ref_eyes->interactor);
	*ref_eyes = {};
}

///////////////////////////////////////////

void interact_mode_eyes_step(interact_mode_eyes_t* ref_eyes) {
	if (ui_far_interact_enabled() == false) return;

	pose_t        eyes  = input_eyes        ();
	button_state_ state = input_eyes_tracked();
	ray_t         ray   = { eyes.position, eyes.orientation * vec3_forward };

	vec3 secondary   = vec3_zero; // Accumulate movement from all active hands
	bool active_curr = false;     // action is active if either hand is active
	for (int32_t h = 0; h < handed_max; h++) {
		const hand_t *hand  = input_hand((handed_)h);

		if (hand->pinch_state & button_state_active) {
			vec3 delta = hand->pinch_pt - ref_eyes->hand_motion_prev[h];
			ref_eyes->hand_motion_prev[h] = hand->pinch_pt;
			secondary  += delta;
			active_curr = true;
		}
	}

	vec3 end = ray.pos + ray.dir * 100;
	interactor_update(ref_eyes->interactor,
		ray.pos, end,
		pose_t{ end, quat_lookat(ray.pos, end) }, end, secondary,
		button_make_state(ref_eyes->active_prev, active_curr), state);

	ref_eyes->active_prev = active_curr;
}

///////////////////////////////////////////

void ui_core_update() {
	interaction_update();

	// auto-switch between hands and controllers
	hand_source_ source = input_hand_source(handed_right);
	if      (source == hand_source_articulated || source == hand_source_overridden)                   skui_input_mode = interact_mode_hands;
	else if (source == hand_source_simulated && device_display_get_type() == display_type_flatscreen) skui_input_mode = interact_mode_mouse;
	else                                                                                              skui_input_mode = interact_mode_controllers;

	if (skui_input_mode_curr != skui_input_mode) {
		if      (skui_input_mode_curr == interact_mode_controllers) interact_mode_controllers_stop(&skui_controllers);
		else if (skui_input_mode_curr == interact_mode_hands      ) interact_mode_hands_stop      (&skui_hands);
		else if (skui_input_mode_curr == interact_mode_mouse      ) interact_mode_mouse_stop      (&skui_mouse);
		else if (skui_input_mode_curr == interact_mode_eyes       ) interact_mode_eyes_stop       (&skui_eyes);

		skui_input_mode_curr  = skui_input_mode;

		if      (skui_input_mode_curr == interact_mode_controllers) interact_mode_controllers_start(&skui_controllers);
		else if (skui_input_mode_curr == interact_mode_hands      ) interact_mode_hands_start      (&skui_hands);
		else if (skui_input_mode_curr == interact_mode_mouse      ) interact_mode_mouse_start      (&skui_mouse);
	}

	if      (skui_input_mode_curr == interact_mode_hands)       { interact_mode_hands_step      (&skui_hands); }
	if      (skui_input_mode_curr == interact_mode_controllers) { interact_mode_controllers_step(&skui_controllers); }
	else if (skui_input_mode_curr == interact_mode_mouse)       { interact_mode_mouse_step      (&skui_mouse); }
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
	interaction_1h_box(id_hash, interact_type == ui_confirm_push ? interactor_event_poke : interactor_event_pinch,
		start, bounds.dimensions,
		start, bounds.dimensions,
		&focus, &interactor);

	interactor_t *actor = interactor_get(interactor);
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

void ui_button_behavior(vec3 window_relative_pos, vec2 size, id_hash_t id, float& out_finger_offset, button_state_& out_button_state, button_state_& out_focus_state, int32_t* out_opt_hand) {
	ui_button_behavior_depth(window_relative_pos, size, id, skui_settings.depth, skui_settings.depth / 2, out_finger_offset, out_button_state, out_focus_state, out_opt_hand);
}

///////////////////////////////////////////

void ui_button_behavior_depth(vec3 window_relative_pos, vec2 size, id_hash_t id, float button_depth, float button_activation_depth, float &out_finger_offset, button_state_ &out_button_state, button_state_ &out_focus_state, int32_t* out_opt_hand) {
	out_button_state  = button_state_inactive;
	out_focus_state   = button_state_inactive;
	out_finger_offset = button_depth;

	int32_t       interactor = -1;
	vec3          interaction_at;
	button_state_ focus_candidacy = button_state_inactive;
	interaction_1h_plate(id, interactor_event_poke,
		{ window_relative_pos.x, window_relative_pos.y, window_relative_pos.z - button_depth }, { size.x, size.y, button_depth },
		&focus_candidacy, &interactor, &interaction_at);

	// If a hand is interacting, adjust the button surface accordingly
	interactor_t* actor = interactor_get(interactor);
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
	
	if (out_button_state & button_state_just_active)
		ui_play_sound_on_off(ui_vis_button, id, hierarchy_to_world_point(ui_layout_last().center));

	if (out_opt_hand)
		*out_opt_hand = interactor;
}

///////////////////////////////////////////

void ui_slider_behavior(vec3 window_relative_pos, vec2 size, id_hash_t id, vec2* value, vec2 min, vec2 max, vec2 button_size_visual, vec2 button_size_interact, ui_confirm_ confirm_method, ui_slider_data_t* out) {
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

	// Secondary motion check
	ui_push_idi(id);
	button_state_ secondary_focus;
	int32_t       secondary_interactor;
	interaction_1h_box(ui_stack_hash("secondary"), (interactor_event_)(interactor_event_grip | interactor_event_pinch | interactor_event_poke),
		window_relative_pos, {size.x, size.y, button_depth*0.5f },
		window_relative_pos, {size.x, size.y, button_depth*0.5f },
		& secondary_focus, &secondary_interactor);
	ui_pop_id();
	interactor_t* secondary_actor = interactor_get(secondary_interactor);
	if (secondary_focus & button_state_active) {
		vec2 secondary_motion = vec2_zero;
		if (secondary_actor->secondary_motion_dimensions == 1) secondary_motion = vec2{ secondary_actor->secondary_motion.x,  secondary_actor->secondary_motion.x };
		if (secondary_actor->secondary_motion_dimensions >= 2) secondary_motion = vec2{ secondary_actor->secondary_motion.x, -secondary_actor->secondary_motion.y };
		vec2 new_percent = {
			range.x == 0 ? 0.5f : fminf(1, fmaxf(0, percent.x + secondary_motion.x)),
			range.y == 0 ? 0.5f : fminf(1, fmaxf(0, percent.y + secondary_motion.y)) };
		vec2 new_val = min + new_percent * range;

		out->button_center = {
			window_relative_pos.x - (new_percent.x * (size.x - button_size_visual.x) + button_size_visual.x / 2.0f),
			window_relative_pos.y - (new_percent.y * (size.y - button_size_visual.y) + button_size_visual.y / 2.0f) };
		*value = new_val;
	}

	if (confirm_method == ui_confirm_push) {
		ui_button_behavior_depth(activation_start, { activation_size.x, activation_size.y }, id, button_depth, button_depth / 2, out->finger_offset, out->active_state, out->focus_state, &out->interactor);

		actor = interactor_get(out->interactor);
		
	} else if (confirm_method == ui_confirm_pinch || confirm_method == ui_confirm_variable_pinch) {
		activation_start.z += skui_settings.depth;
		activation_size.z  += skui_settings.depth;
		interaction_1h_box(id, interactor_event_pinch,
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
	if (out->active_state & button_state_active) {
		vec2 pos_in_slider = {
			fminf(1, fmaxf(0, ((window_relative_pos.x-button_size_visual.x/2)-finger_at.x) / (size.x-button_size_visual.x))),
			fminf(1, fmaxf(0, ((window_relative_pos.y-button_size_visual.y/2)-finger_at.y) / (size.y-button_size_visual.y)))};
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
	bool result = interaction_handle(id, &handle_pose, handle_bounds, move_type, allowed_gestures);
	ui_push_surface(handle_pose);

	float color_blend = 0;
	if (ui_id_focused(id)) color_blend = 1;
	if (ui_id_active_state(id) & button_state_just_active)
		ui_play_sound_on_off(ui_vis_handle, id, hierarchy_to_world_point(handle_pose.position));

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