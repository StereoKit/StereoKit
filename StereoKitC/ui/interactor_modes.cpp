/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#include "interactors.h"
#include "interactor_modes.h"

#include "../systems/input.h"
#include "../sk_math.h"

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
	interactor_t poke [2];
	interactor_t pinch[2];
	interactor_t far  [2];
	float        ray_active [2];
	float        ray_visible[2];
	bool         prev_active[2];
};

struct interact_mode_controllers_t {
	interactor_t far[2];
	float        ray_active  [2];
	float        ray_visible [2];
	float        trigger_last[2];
};

struct interact_mode_mouse_t {
	interactor_t interactor;
};

struct interact_mode_eyes_t {
	interactor_t interactor;
	vec3         hand_motion_prev[2];
	bool         active_prev;
};

struct interactor_modes_state_t {
	default_interactors_        default_interactors;
	bool32_t                    draw_interactors;
	interact_mode_              input_mode[handed_max];
	interact_mode_              input_permitted;
	interact_mode_hands_t       hands;
	interact_mode_controllers_t controllers;
	interact_mode_mouse_t       mouse;
	interact_mode_eyes_t        eyes;
};
static interactor_modes_state_t local = {};

///////////////////////////////////////////

void interact_mode_switch           (interact_mode_ mode, handed_ hand);

void interact_mode_hands_start      (interact_mode_hands_t*       ref_hands, handed_ hand);
void interact_mode_hands_stop       (interact_mode_hands_t*       ref_hands, handed_ hand);
void interact_mode_hands_step       (interact_mode_hands_t*       ref_hands, handed_ hand);
void interact_mode_controllers_start(interact_mode_controllers_t* ref_controllers, handed_ hand);
void interact_mode_controllers_stop (interact_mode_controllers_t* ref_controllers, handed_ hand);
void interact_mode_controllers_step (interact_mode_controllers_t* ref_controllers, handed_ hand);
void interact_mode_mouse_start      (interact_mode_mouse_t*       ref_mouse);
void interact_mode_mouse_stop       (interact_mode_mouse_t*       ref_mouse);
void interact_mode_mouse_step       (interact_mode_mouse_t*       ref_mouse);
void interact_mode_eyes_start       (interact_mode_eyes_t*        ref_eyes);
void interact_mode_eyes_stop        (interact_mode_eyes_t*        ref_eyes);
void interact_mode_eyes_step        (interact_mode_eyes_t*        ref_eyes);

///////////////////////////////////////////

void interactor_modes_init() {
	local = {};
	local.draw_interactors = true;
}

///////////////////////////////////////////

void interactor_modes_shutdown() {
	for (int32_t h = 0; h < handed_max; h++) {
		interact_mode_switch(interact_mode_none, (handed_)h);
	}
	local = {};
}

///////////////////////////////////////////

void interactor_modes_update() {
	// auto-switch between hands and controllers
	if (local.default_interactors == default_interactors_default) {
		for (int32_t h = 0; h < handed_max; h++) {
			hand_source_ source = input_hand_source((handed_)h);
			if (source == hand_source_articulated || source == hand_source_overridden) {
				if (input_controller_is_hand((handed_)h)) {
				    interact_mode_switch(interact_mode_hands, (handed_)h);
			    } else {
				    interact_mode_switch(interact_mode_controllers, (handed_)h);
			    }
			}
			else if (source == hand_source_simulated && device_display_get_type() == display_type_flatscreen) interact_mode_switch(interact_mode_mouse, handed_right);
			else {
				for (int32_t h = 0; h < handed_max; h++) {
					if (input_controller_is_hand((handed_)h)) {
				    	interact_mode_switch(interact_mode_hands, (handed_)h);
			    	} else {
				    	interact_mode_switch(interact_mode_controllers, (handed_)h);
			    	}
				}
			}
		}
	}

	for (int32_t h = 0; h < handed_max; h++) {
		// Use input_controller_is_hand to determine if we have a hand or controller
		if (input_controller_is_hand((handed_)h)) {
			if (local.input_mode[h] == interact_mode_hands) {
				interact_mode_hands_step(&local.hands, (handed_)h);
			}
		} else {
			if (local.input_mode[h] == interact_mode_controllers) {
				interact_mode_controllers_step(&local.controllers, (handed_)h);
			}
		}
	}
	
	// Handle global modes that don't depend on individual hands
	if (local.input_mode[handed_right] == interact_mode_mouse) {
		interact_mode_mouse_step(&local.mouse);
	} else if (local.input_mode[handed_right] == interact_mode_eyes) {
		interact_mode_eyes_step(&local.eyes);
	}
}

///////////////////////////////////////////

void interaction_set_default_interactors(default_interactors_ default_interactors) {
	if (local.default_interactors == default_interactors) return;
	local.default_interactors = default_interactors;

	if (local.default_interactors == default_interactors_none) {
		for (int32_t h = 0; h < handed_max; h++) {
			interact_mode_switch(interact_mode_none, (handed_)h);
		}
	}
}

///////////////////////////////////////////

default_interactors_ interaction_get_default_interactors() {
	return local.default_interactors;
}

///////////////////////////////////////////

void interaction_set_default_draw(bool32_t draw_interactors) {
	local.draw_interactors = draw_interactors;
}

///////////////////////////////////////////

bool32_t interaction_get_default_draw() {
	return local.draw_interactors;
}

///////////////////////////////////////////

void interact_mode_switch(interact_mode_ mode, handed_ hand) {
	int32_t h = (int32_t)hand;
	if (local.input_mode[h] == mode) return;

	if      (local.input_mode[h] == interact_mode_controllers) interact_mode_controllers_stop(&local.controllers, hand);
	else if (local.input_mode[h] == interact_mode_hands      ) interact_mode_hands_stop      (&local.hands, hand);
	else if (local.input_mode[h] == interact_mode_mouse      ) interact_mode_mouse_stop      (&local.mouse);
	else if (local.input_mode[h] == interact_mode_eyes       ) interact_mode_eyes_stop       (&local.eyes);

	const char* hand_str = hand == handed_left ? "left" : hand == handed_right ? "right" : "unknown";
	const char* mode_str = "unknown";
	switch (mode) {
		case interact_mode_none:        mode_str = "none"; break;
		case interact_mode_hands:       mode_str = "hands"; break;
		case interact_mode_controllers: mode_str = "controllers"; break;
		case interact_mode_mouse:       mode_str = "mouse"; break;
		case interact_mode_eyes:        mode_str = "eyes"; break;
		default: break;
	}
	log_infof("[interactor_modes] Switching %s hand to mode: %s", hand_str, mode_str);

	local.input_mode[h] = mode;

	if      (local.input_mode[h] == interact_mode_controllers) interact_mode_controllers_start(&local.controllers, hand);
	else if (local.input_mode[h] == interact_mode_hands      ) interact_mode_hands_start      (&local.hands, hand);
	else if (local.input_mode[h] == interact_mode_mouse      ) interact_mode_mouse_start      (&local.mouse);
	else if (local.input_mode[h] == interact_mode_eyes       ) interact_mode_eyes_start       (&local.eyes);
}

///////////////////////////////////////////

void interactor_show_ray(interactor_t interactor, float skip, bool hide_inactive, float *ref_visible_amt, float *ref_active_amt) {
	_interactor_t* actor = _interactor_get(interactor);
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

void interact_mode_hands_start(interact_mode_hands_t *ref_hands, handed_ hand) {
	int32_t h = (int32_t)hand;
	ref_hands->poke [h] = interactor_create(interactor_type_point, interactor_event_poke,  interactor_activation_position, 1000+h, 0, 0);
	ref_hands->pinch[h] = interactor_create(interactor_type_point, interactor_event_pinch, interactor_activation_state,    1000+h, 0, 0);
	ref_hands->far  [h] = interactor_create(interactor_type_line, (interactor_event_)(interactor_event_poke | interactor_event_pinch), interactor_activation_state, 1000+h, 0.01f, 2);
	ref_hands->ray_active [h] = 0;
	ref_hands->ray_visible[h] = 0;
}

///////////////////////////////////////////

void interact_mode_hands_stop(interact_mode_hands_t* ref_hands, handed_ hand) {
	int32_t h = (int32_t)hand;
	interactor_destroy(ref_hands->poke [h]);
	interactor_destroy(ref_hands->pinch[h]);
	interactor_destroy(ref_hands->far  [h]);
	ref_hands->poke[h] = {};
	ref_hands->pinch[h] = {};
	ref_hands->far[h] = {};
}

///////////////////////////////////////////

void interact_mode_hands_step(interact_mode_hands_t* ref_hands, handed_ hand) {
	int32_t i = (int32_t)hand;
	const hand_t* hand_data = input_hand(hand);

	// Poke
	interactor_t id         = ref_hands->poke[i];
	_interactor_t* interactor = _interactor_get(id);
	interactor_set_radius(id, hand_data->fingers[1][4].radius);
	interactor_update    (id,
		(hand_data->tracked_state & button_state_just_active) ? hand_data->fingers[1][4].position : interactor->capsule_end_world, hand_data->fingers[1][4].position,
		pose_t{ hand_data->fingers[1][4].position, hand_data->palm.orientation }, hand_data->fingers[1][4].position, vec3_zero,
		button_state_inactive, hand_data->tracked_state);

	// Pinch
	id = ref_hands->pinch[i];
	interactor_set_radius(id, hand_data->fingers[1][4].radius);
	interactor_update    (id,
		hand_data->fingers[0][4].position, hand_data->fingers[1][4].position,
		pose_t{ hand_data->pinch_pt, hand_data->palm.orientation }, hand_data->pinch_pt, vec3_zero,
		hand_data->pinch_state, hand_data->tracked_state);

	// Hand ray
	if (ui_far_interact_enabled()) {
		pose_t head      = input_head();
		float  hand_dist = vec3_distance(hand_data->palm.position, head.position + vec3{0,-0.12f,0});

		// Pinches can only start when aim_ready, but should remain true
		// as long as the pinch remains active
		bool is_pinched   = (hand_data->pinch_state & button_state_active) > 0;
		bool just_pinched = (hand_data->pinch_state & button_state_just_active) > 0;
		bool aim_ready    = (hand_data->aim_ready   & button_state_active) > 0;
		bool is_active    = (ref_hands->prev_active[i] && is_pinched) || (aim_ready && just_pinched);
		button_state_ far_pinch_state = button_make_state(ref_hands->prev_active[i], is_active);
		ref_hands->prev_active[i] = is_active;

		id = ref_hands->far[i];
		interactor_set_min_distance(id, math_lerp(0.35f, 0.20f, math_saturate((hand_dist - 0.1f) / 0.4f)));
		interactor_update          (id,
			hand_data->aim.position, hand_data->aim.position + hand_data->aim.orientation * vec3_forward * 100,
			hand_data->aim, head.position + vec3{0,-0.12f,0}, vec3_zero,
			far_pinch_state, hand_data->aim_ready);
		if (local.draw_interactors)
			interactor_show_ray(id, 0.07f, true, &ref_hands->ray_visible[i], &ref_hands->ray_active[i]);
	}
}

///////////////////////////////////////////
// Controller Interactors
///////////////////////////////////////////

void interact_mode_controllers_start(interact_mode_controllers_t* ref_controllers, handed_ hand) {
	int32_t h = (int32_t)hand;
	ref_controllers->far[h] = interactor_create(interactor_type_line, (interactor_event_)(interactor_event_poke | interactor_event_pinch), interactor_activation_state, -1, 0.005f, 2);
	ref_controllers->ray_active [h] = 0;
	ref_controllers->ray_visible[h] = 0;
}

///////////////////////////////////////////

void interact_mode_controllers_stop(interact_mode_controllers_t* ref_controllers, handed_ hand) {
	int32_t h = (int32_t)hand;
	interactor_destroy(ref_controllers->far[h]);
	ref_controllers->far[h] = {};
}

///////////////////////////////////////////

void interact_mode_controllers_step(interact_mode_controllers_t* ref_controllers, handed_ hand) {
	if (ui_far_interact_enabled() == false) return;

	int32_t i = (int32_t)hand;
	pose_t head = input_head();
	const controller_t *ctrl = input_controller(hand);

	// controller ray
	interactor_t id = ref_controllers->far[i];
	interactor_update(id,
		ctrl->aim.position, ctrl->aim.position + ctrl->aim.orientation*vec3_forward * 100,
		ctrl->aim, head.position + vec3{ 0,-0.12f,0 }, vec3{ ctrl->stick.x, ctrl->stick.y, 0 }* 0.02f,
		button_make_state(ref_controllers->trigger_last[i]>0.5f, ctrl->trigger>0.5f),
		ctrl->tracked);
	ref_controllers->trigger_last[i] = ctrl->trigger;
	if (local.draw_interactors)
		interactor_show_ray(id, 0, false, &ref_controllers->ray_visible[i], &ref_controllers->ray_active[i]);
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
		input_key(key_mouse_left), button_make_state(_interactor_get(ref_mouse->interactor)->tracked & button_state_active, tracked));
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

// TODO: v0.4 These functions use hands instead of interactors, they need replaced!
bool32_t ui_is_interacting(handed_ hand) {
	int32_t h = (int32_t)hand;
	if (local.input_mode[h] == interact_mode_controllers) {

		const _interactor_t* actor = _interactor_get(local.controllers.far[hand]);
		return actor->active_prev != 0 || actor->focused_prev != 0;

	} else if (local.input_mode[h] == interact_mode_hands) {

		const _interactor_t* actor = _interactor_get(local.hands.far[hand]);
		if (actor->active_prev != 0 || actor->focused_prev != 0) return true;

		actor = _interactor_get(local.hands.pinch[hand]);
		if (actor->active_prev != 0 || actor->focused_prev != 0) return true;

		actor = _interactor_get(local.hands.poke[hand]);
		if (actor->active_prev != 0 || actor->focused_prev != 0) return true;

	} else if (local.input_mode[h] == interact_mode_mouse) {

		if (hand == handed_left) return false;

		const _interactor_t* actor = _interactor_get(local.mouse.interactor);
		return actor->active_prev != 0 || actor->focused_prev != 0;

	}
	return false;
}

///////////////////////////////////////////

// TODO: v0.4 These functions use hands instead of interactors, they need replaced!
button_state_ ui_last_element_hand_active(handed_ hand) {
	id_hash_t last = ui_id_last_element();
	int32_t h = (int32_t)hand;

	if (local.input_mode[h] == interact_mode_controllers) {

		const _interactor_t* actor = _interactor_get(local.controllers.far[hand]);
		return button_make_state(actor->active_prev == last, actor->active == last);

	} else if (local.input_mode[h] == interact_mode_hands) {

		const _interactor_t* actor = _interactor_get(local.hands.far[hand]);
		if (actor->active_prev != 0 || actor->active != 0)
			return button_make_state( actor->active_prev == last, actor->active == last);

		actor = _interactor_get(local.hands.pinch[hand]);
		if (actor->active_prev != 0 || actor->active != 0)
			return button_make_state(actor->active_prev == last, actor->active == last);

		actor = _interactor_get(local.hands.poke[hand]);
		if (actor->active_prev != 0 || actor->active != 0)
			return button_make_state(actor->active_prev == last, actor->active == last);

	} else if (local.input_mode[h] == interact_mode_mouse) {

		if (hand == handed_left) return button_state_inactive;

		const _interactor_t* actor = _interactor_get(local.mouse.interactor);
		return button_make_state(actor->active_prev == last, actor->active == last);

	}
	return button_state_inactive;
}

///////////////////////////////////////////

// TODO: v0.4 These functions use hands instead of interactors, they need replaced!
button_state_ ui_last_element_hand_focused(handed_ hand) {
	// Because focus can change at any point during the frame, we'll check
	// against the last two frame's focus ids, which are set in stone after the
	// frame ends.

	id_hash_t last = ui_id_last_element();
	int32_t h = (int32_t)hand;
	if (local.input_mode[h] == interact_mode_controllers) {

		const _interactor_t* actor = _interactor_get(local.controllers.far[hand]);
		return button_make_state(actor->focused_prev_prev == last, actor->focused_prev == last);

	} else if (local.input_mode[h] == interact_mode_hands) {

		const _interactor_t* actor = _interactor_get(local.hands.far[hand]);
		if (actor->focused_prev_prev != 0 || actor->focused_prev != 0)
			return button_make_state( actor->focused_prev_prev == last, actor->focused_prev == last);

		actor = _interactor_get(local.hands.pinch[hand]);
		if (actor->focused_prev_prev != 0 || actor->focused_prev != 0)
			return button_make_state(actor->focused_prev_prev == last, actor->focused_prev == last);

		actor = _interactor_get(local.hands.poke[hand]);
		if (actor->focused_prev_prev != 0 || actor->focused_prev != 0)
			return button_make_state(actor->focused_prev_prev == last, actor->focused_prev == last);

	} else if (local.input_mode[h] == interact_mode_mouse) {

		if (hand == handed_left) return button_state_inactive;

		const _interactor_t* actor = _interactor_get(local.mouse.interactor);
		return button_make_state(actor->focused_prev_prev == last, actor->focused_prev == last);

	}
	return button_state_inactive;
}

///////////////////////////////////////////

pointer_t input_pointer(int32_t index, input_source_ filter) {
	int32_t start = 0;
	if      (filter & input_source_hand      ) start = 0;
	else if (filter & input_source_hand_left ) start = 0;
	else if (filter & input_source_hand_right) start = 1;
	int32_t idx = index + start;
	
	// Handle controllers mode for the specific hand
	if (idx <= 1 && local.input_mode[idx] == interact_mode_controllers) {
		interactor_t actor_id = local.controllers.far[idx];
		pointer_t result   = {};
		result.orientation = interactor_get_motion (actor_id).orientation;
		result.state       = _interactor_get       (actor_id)->pinch_state;
		result.tracked     = interactor_get_tracked(actor_id);
		result.ray         = { _interactor_get(actor_id)->capsule_start_world, result.orientation * vec3_forward };
		result.source      = idx == 0 ? input_source_hand_left : input_source_hand_right;
		return result;
	}
	
	// Handle hands mode for the specific hand
	if (idx <= 1 && local.input_mode[idx] == interact_mode_hands) {
		interactor_t actor_id = local.hands.far[idx];
		pointer_t result   = {};
		result.orientation = interactor_get_motion (actor_id).orientation;
		result.state       = _interactor_get        (actor_id)->pinch_state;
		result.tracked     = interactor_get_tracked(actor_id);
		result.ray         = { _interactor_get(actor_id)->capsule_start_world, result.orientation * vec3_forward };
		result.source      = idx == 0 ? input_source_hand_left : input_source_hand_right;
		return result;
	}
	
	// Handle mouse mode (only for right hand)
	if (index == 0 && local.input_mode[handed_right] == interact_mode_mouse) {
		interactor_t actor_id = local.mouse.interactor;
		pointer_t result   = {};
		result.orientation = interactor_get_motion (actor_id).orientation;
		result.state       = _interactor_get        (actor_id)->pinch_state;
		result.tracked     = interactor_get_tracked(actor_id);
		result.ray         = { _interactor_get(actor_id)->capsule_start_world, result.orientation * vec3_forward };
		result.source      = input_source_hand_right;
		return result;
	}
	
	return {};
}


///////////////////////////////////////////

int32_t input_pointer_count(input_source_ filter) {
	int32_t count = 0;
	
	// Count active pointers based on per-hand mode
	for (int32_t h = 0; h < handed_max; h++) {
		if (local.input_mode[h] == interact_mode_controllers || local.input_mode[h] == interact_mode_hands) {
			count++;
		}
	}
	
	// Add mouse pointer if active (only for right hand)
	if (local.input_mode[handed_right] == interact_mode_mouse) {
		count = 1; // Mouse replaces hand modes, so only count 1
	}
	
	return count;
}

}