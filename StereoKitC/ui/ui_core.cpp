#include "ui_core.h"
#include "ui_layout.h"
#include "ui_theming.h"

#include "../libraries/array.h"
#include "../libraries/ferr_hash.h"
#include "../systems/input.h"
#include "../hands/input_hand.h"
#include "../sk_math.h"

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

int32_t skui_input_mode = 1;
int32_t skui_hand_interactors[6] = { -1, -1, -1, -1 };
int32_t skui_mouse_interactor    = -1;
int32_t skui_eye_interactors[2]  = { -1, -1 };

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

	skui_hand_interactors[0] = interactor_create(interactor_type_point, interactor_event_poke);
	skui_hand_interactors[1] = interactor_create(interactor_type_line,  interactor_event_pinch);
	skui_hand_interactors[2] = interactor_create(interactor_type_line,  (interactor_event_)(interactor_event_poke | interactor_event_pinch));
	skui_hand_interactors[3] = interactor_create(interactor_type_point, interactor_event_poke);
	skui_hand_interactors[4] = interactor_create(interactor_type_line,  interactor_event_pinch);
	skui_hand_interactors[5] = interactor_create(interactor_type_line,  (interactor_event_)(interactor_event_poke | interactor_event_pinch));

	skui_mouse_interactor    = interactor_create(interactor_type_line,  (interactor_event_)(interactor_event_poke | interactor_event_pinch));

	skui_eye_interactors[0]  = interactor_create(interactor_type_line,  (interactor_event_)(interactor_event_poke | interactor_event_pinch));
	skui_eye_interactors[1]  = interactor_create(interactor_type_line,  (interactor_event_)(interactor_event_poke | interactor_event_pinch));
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

void ui_core_update() {
	const matrix *to_local = hierarchy_to_local();

	for (int32_t i = 0; i < skui_interactors.count; i++) {
		skui_interactors[i].focused_prev_prev   = skui_interactors[i].focused_prev;
		skui_interactors[i].focused_prev        = skui_interactors[i].focused;
		skui_interactors[i].active_prev_prev    = skui_interactors[i].active_prev;
		skui_interactors[i].active_prev         = skui_interactors[i].active;
		skui_interactors[i].orientation_prev    = skui_interactors[i].orientation;
		skui_interactors[i].position_prev       = skui_interactors[i].position;

		skui_interactors[i].focus_priority = FLT_MAX;
		skui_interactors[i].focused        = 0;
		skui_interactors[i].active         = 0;
		skui_interactors[i].capsule_end    = matrix_transform_pt(*to_local, skui_interactors[i].capsule_end_world);
		skui_interactors[i].capsule_start  = matrix_transform_pt(*to_local, skui_interactors[i].capsule_start_world);
		skui_interactors[i].ray_enabled    = skui_interactors[i].tracked > 0 && skui_interactors[i].tracked && (vec3_dot(skui_interactors[i].capsule_end_world - skui_interactors[i].capsule_start_world, input_head()->position - skui_interactors[i].capsule_start_world) < 0);

		skui_interactors[i].tracked     = button_state_inactive;
		skui_interactors[i].pinch_state = button_state_inactive;
		// draw hand rays
		//skui_interactor[i].ray_visibility = math_lerp(skui_interactor[i].ray_visibility,
		//	was_ray_enabled && ui_far_interact_enabled() && skui_interactor[i].ray_enabled && !skui_interactor[i].ray_discard ? 1.0f : 0.0f,
		//	20.0f * time_stepf_unscaled());

		/*if (skui_interactors[i].focused_prev != 0) skui_interactors[i].ray_visibility = 0;
		if (skui_interactors[i].ray_visibility > 0.004f) {
			ray_t       r     = input_get_pointer(input_hand_pointer_id[i])->ray;
			const float scale = 2;
			line_point_t points[5] = {
				line_point_t{r.pos+r.dir*(0.07f              ), 0.001f,  color32{255,255,255,0}},
				line_point_t{r.pos+r.dir*(0.07f + 0.01f*scale), 0.0015f, color32{255,255,255,(uint8_t)(skui_interactors[i].ray_visibility * 60 )}},
				line_point_t{r.pos+r.dir*(0.07f + 0.02f*scale), 0.0020f, color32{255,255,255,(uint8_t)(skui_interactors[i].ray_visibility * 80)}},
				line_point_t{r.pos+r.dir*(0.07f + 0.07f*scale), 0.0015f, color32{255,255,255,(uint8_t)(skui_interactors[i].ray_visibility * 25 )}},
				line_point_t{r.pos+r.dir*(0.07f + 0.11f*scale), 0.001f,  color32{255,255,255,0}} };
			line_add_listv(points, 5);
		}
		skui_interactors[i].ray_discard = false;*/

		if ((skui_interactors[i].tracked & button_state_active) > 0 && skui_interactors[i].focused_prev != 0) {
			line_add(skui_interactors[i].position, skui_interactors[i].focus_center_world, color32{255,255,255,255}, color32{ 255,255,255,255 }, 0.001f);
		}
	}

	if (skui_input_mode == 0) {
		input_hand_visible(handed_max, true);
		for (int32_t i = 0; i < handed_max; i++) {
			const hand_t*    hand    = input_hand       ((handed_)i);
			const pointer_t* pointer = input_get_pointer(input_hand_pointer_id[i]);

			// Poke
			interactor_update(skui_hand_interactors[i*3 + 0],
				(hand->tracked_state & button_state_just_active) ? hand->fingers[1][4].position : skui_interactors[i].capsule_end_world, hand->fingers[1][4].position, hand->fingers[1][4].radius,
				hand->fingers[1][4].position, hand->palm.orientation, hand->fingers[1][4].position,
				button_state_inactive, hand->tracked_state);

			// Pinch
			interactor_update(skui_hand_interactors[i*3 + 1],
				hand->fingers[0][4].position, hand->fingers[1][4].position, hand->fingers[1][4].radius,
				hand->pinch_pt, hand->palm.orientation, hand->pinch_pt,
				hand->pinch_state, hand->tracked_state);

			// Hand ray
			interactor_update(skui_hand_interactors[i*3 + 2],
				pointer->ray.pos, pointer->ray.pos + pointer->ray.dir * 100, 0,
				pointer->ray.pos, pointer->orientation, input_head()->position,
				pointer->state, pointer->tracked);
		}
	} else if (skui_input_mode == 1) {
		input_hand_visible(handed_max, false);
		const pose_t*  head = input_head();
		const mouse_t* m    = input_mouse();
		ray_t ray;
		bool tracked = ray_from_mouse(m->pos, ray);

		vec3 end = ray.pos + ray.dir * 100;
		interactor_update(skui_mouse_interactor,
			ray.pos, end, 0,
			end, quat_lookat(ray.pos, end), end,
			input_key(key_mouse_left), button_make_state(skui_interactors[skui_mouse_interactor].tracked & button_state_active, tracked));
	}
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
button_state_ ui_volumei_at_g(const C *id, bounds_t bounds, ui_confirm_ interact_type, handed_ *out_opt_hand, button_state_ *out_opt_focus_state) {
	id_hash_t     id_hash = ui_stack_hash(id);
	button_state_ result  = button_state_inactive;
	button_state_ focus   = button_state_inactive;
	int32_t       interactor = -1;

	vec3 start = bounds.center + bounds.dimensions / 2;
	ui_box_interaction_1h(id_hash, interact_type == ui_confirm_push ? interactor_event_poke : interactor_event_pinch,
		start, bounds.dimensions,
		start, bounds.dimensions,
		&focus, &interactor);

	bool active = focus & button_state_active && !(focus & button_state_just_inactive);
	if (interact_type != ui_confirm_push && interactor != -1) {
		active = skui_interactors[interactor].pinch_state & button_state_active;
		// Focus can get lost if the user is dragging outside the box, so set
		// it to focused if it's still active.
		focus = interactor_set_focus(interactor, id_hash, active || focus & button_state_active, 0, bounds.center);
	}
	result = interactor_set_active(interactor, id_hash, active);

	if (out_opt_hand        != nullptr) *out_opt_hand        = (handed_)interactor;
	if (out_opt_focus_state != nullptr) *out_opt_focus_state = focus;
	return result;
}
button_state_ ui_volumei_at   (const char     *id, bounds_t bounds, ui_confirm_ interact_type, handed_ *out_opt_hand, button_state_ *out_opt_focus_state) { return ui_volumei_at_g<char    >(id, bounds, interact_type, out_opt_hand, out_opt_focus_state); }
button_state_ ui_volumei_at_16(const char16_t *id, bounds_t bounds, ui_confirm_ interact_type, handed_ *out_opt_hand, button_state_ *out_opt_focus_state) { return ui_volumei_at_g<char16_t>(id, bounds, interact_type, out_opt_hand, out_opt_focus_state); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_volume_at_g(const C *id, bounds_t bounds) {
	id_hash_t id_hash = ui_stack_hash(id);
	bool      result  = false;

	skui_last_element = id_hash;

	for (int32_t i = 0; i < skui_interactors.count; i++) {
		interactor_t* actor  = &skui_interactors[i];
		bool     was_focused = actor->focused_prev == id_hash;
		bounds_t size        = bounds;
		if (was_focused) {
			size.dimensions = bounds.dimensions + vec3_one*skui_settings.padding;
		}

		vec3  at;
		float priority;
		bool          in_box = interactor_check_box(actor, size, &at, &priority);
		button_state_ state  = interactor_set_focus(i, id_hash, in_box, priority, bounds.center);
		if (state & button_state_just_active)
			result = true;
	}

	return result;
}
bool32_t ui_volume_at   (const char     *id, bounds_t bounds) { return ui_volume_at_g<char    >(id, bounds); }
bool32_t ui_volume_at_16(const char16_t *id, bounds_t bounds) { return ui_volume_at_g<char16_t>(id, bounds); }

///////////////////////////////////////////

// TODO: Has no Id, might be good to move this over to ui_focus/active_set around v0.4
button_state_ ui_interact_volume_at(bounds_t bounds, handed_ &out_hand) {
	button_state_ result  = button_state_inactive;

	for (int32_t i = 0; i < skui_interactors.count; i++) {
		interactor_t* actor = &skui_interactors[i];

		bool was_active  = actor->active_prev  != 0;
		bool was_focused = actor->focused_prev != 0;
		if (was_active || was_focused)
			continue;

		vec3  at;
		float priority;
		if (interactor_check_box(actor, bounds, &at, &priority)) {
			button_state_ state = actor->pinch_state;
			if (state != button_state_inactive) {
				result = state;
				out_hand = (handed_)i;
			}
		}
	}

	return result;
}

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

	interactor_plate_1h(id, interactor_event_poke,
		{ window_relative_pos.x, window_relative_pos.y, window_relative_pos.z - button_depth }, size,
		&out_focus_state, &interactor, &interaction_at);

	// If a hand is interacting, adjust the button surface accordingly
	out_finger_offset = button_depth;
	if (out_focus_state & button_state_active) {
		interactor_t* actor = &skui_interactors[interactor];

		out_finger_offset = -(interaction_at.z+actor->capsule_radius) - window_relative_pos.z;
		bool pressed = out_finger_offset < skui_settings.depth / 2;
		if ((actor->active_prev == id && (actor->pinch_state & button_state_active)) || (actor->pinch_state & button_state_just_active)) {
			pressed = true;
			out_finger_offset = 0;
		}
		out_button_state = interactor_set_active(interactor, id, pressed);
		out_finger_offset = fminf(fmaxf(2*mm2m, out_finger_offset), skui_settings.depth);
	} else if (out_focus_state & button_state_just_inactive) {
		out_button_state = interactor_set_active(interactor, id, false);
	}
	
	if (out_button_state & button_state_just_active)
		ui_play_sound_on_off(ui_vis_button, id, skui_interactors[interactor].capsule_end_world);

	if (out_opt_hand)
		*out_opt_hand = interactor;
}

///////////////////////////////////////////

void ui_slider_behavior(id_hash_t id, vec2* value, vec2 min, vec2 max, vec2 step, vec3 window_relative_pos, vec2 size, vec2 button_size, ui_confirm_ confirm_method, vec2 *out_button_center, float *out_finger_offset, button_state_ *out_focus_state, button_state_ *out_active_state, int32_t *out_interactor) {
	const float snap_scale = 1;
	const float snap_dist  = 7*cm2m;

	// Find sizes of slider elements
	vec2  range        = max - min;
	vec2  percent      = { range.x == 0 ? 0.5f : (value->x - min.x)/range.x,  range.y == 0 ? 0.5f : (value->y - min.y)/range.y };
	float button_depth = confirm_method == ui_confirm_push ? skui_settings.depth : skui_settings.depth * 1.5f;

	// Activation bounds sizing
	float activation_plane = button_depth + skui_finger_radius;

	// Set up for getting the state of the sliders.
	*out_focus_state   = button_state_inactive;
	*out_active_state  = button_state_inactive;
	*out_interactor    = -1;
	*out_finger_offset = button_depth;
	*out_button_center = {
		window_relative_pos.x - (percent.x * (size.x - button_size.x) + button_size.x/2.0f),
		window_relative_pos.y - (percent.y * (size.y - button_size.y) + button_size.y/2.0f) };

	vec2 finger_at = {};
	if (confirm_method == ui_confirm_push) {
		vec3  activation_size  = vec3{ button_size.x, button_size.y, 0.0001f };
		vec3  activation_start = { out_button_center->x+activation_size.x/2.0f, out_button_center->y+activation_size.y/2.0f, -activation_plane };
		vec3  sustain_size     = vec3{ size.x, size.y, activation_plane + 6*skui_finger_radius  };
		vec3  sustain_start    = window_relative_pos + vec3{ skui_finger_radius, skui_finger_radius, -activation_plane + sustain_size.z };

		//ui_button_behavior(activation_start, { activation_size.x, activation_size.y }, id,
		//	*out_finger_offset, *out_active_state, *out_focus_state, out_interactor);
		//
		//if (*out_active_state & button_state_active) {
		//	//finger_at = { finger_pt.x, finger_pt.y };
		//}

		*out_interactor = ui_id_active_interactor(id);
		if (*out_interactor != -1 && skui_interactors[*out_interactor].pinch_state & button_state_active) {
			*out_focus_state  = interactor_set_focus (*out_interactor, id, true, 0, vec3_zero);
			*out_active_state = interactor_set_active(*out_interactor, id, true);
		} else {
			vec3 finger_pt;
			interactor_plate_1h(id, (interactor_event_)(interactor_event_poke | interactor_event_pinch),
				activation_start, { activation_size.x, activation_size.y },
				out_focus_state, out_interactor, &finger_pt);

			if (*out_focus_state & button_state_active) {
				interactor_t* actor = &skui_interactors[*out_interactor];

				*out_finger_offset = -(finger_pt.z + actor->capsule_radius) - window_relative_pos.z;
				bool pressed = *out_finger_offset < skui_settings.depth / 2;
				if ((actor->active_prev == id && (actor->pinch_state & button_state_active)) || (actor->pinch_state & button_state_just_active)) {
					pressed = true;
					*out_finger_offset = 0;
				}
				*out_active_state  = interactor_set_active(*out_interactor, id, pressed);
				*out_finger_offset = fminf(fmaxf(2 * mm2m, *out_finger_offset), skui_settings.depth);

				finger_at = { finger_pt.x, finger_pt.y };
			}
			else if (*out_focus_state & button_state_just_inactive) {
				*out_active_state = interactor_set_active(*out_interactor, id, false);
			}
		}

		//ui_box_interaction_1h_poke(id,
		//	activation_start, activation_size,
		//	sustain_start,    sustain_size,
		//	out_focus_state, out_interactor);

		// Here, we allow for pressing or pinching of the button to activate
		// the slider!
		/*if (*out_interactor != -1) {
			button_state_ pinch = skui_interactors[*out_interactor].pinch_state;

			*out_finger_offset = -skui_interactors[*out_interactor].capsule_end.z - window_relative_pos.z;
			bool pressed  = *out_finger_offset < button_depth / 2;
			*out_finger_offset = fminf(fmaxf(2 * mm2m, *out_finger_offset), button_depth);

			*out_active_state = interactor_set_active(*out_interactor, id, pinch & button_state_active || pressed);
			// Focus can get lost if the user is dragging outside the box, so set
			// it to focused if it's still active.
			*out_focus_state = interactor_set_focus(*out_interactor, id, pinch & button_state_active || (*out_focus_state) & button_state_active, 0, activation_start - activation_size/2);

			finger_at = { finger_pt.x, finger_pt.y };
		}*/
	} else if (confirm_method == ui_confirm_pinch || confirm_method == ui_confirm_variable_pinch) {
		vec3 activation_start = window_relative_pos + vec3{ percent.x * -(size.x - button_size.x) + button_size.x / 2.0f, percent.y * -(size.y - button_size.y) + button_size.y / 2.0f, button_depth };
		vec3 activation_size  = vec3{ button_size.x, button_size.y, button_depth * 2 };

		ui_box_interaction_1h(id, interactor_event_pinch,
			activation_start, activation_size,
			activation_start, activation_size,
			out_focus_state, out_interactor);

		// Pinch confirm uses a handle that the user must pinch, in order to
		// drag it around the slider.
		if (*out_interactor != -1) {
			button_state_ pinch = skui_interactors[*out_interactor].pinch_state;
			*out_active_state = interactor_set_active(*out_interactor, id, pinch & button_state_active);
			// Focus can get lost if the user is dragging outside the box, so set
			// it to focused if it's still active.
			*out_focus_state = interactor_set_focus(*out_interactor, id, (*out_active_state) & button_state_active || (*out_focus_state) & button_state_active, 0, activation_start - activation_size/2);
			vec3    pinch_local = hierarchy_to_local_point(skui_interactors[*out_interactor].position);
			int32_t scale_step  = (int32_t)((-pinch_local.z-activation_plane) / snap_dist);
			finger_at = { pinch_local.x, pinch_local.y };
		}
	}

	vec2 new_percent = percent;
	if ((*out_active_state) & button_state_active) {
		vec2 pos_in_slider = {
			(float)fmin(1, fmax(0, ((window_relative_pos.x-button_size.x/2)-finger_at.x) / (size.x-button_size.x))),
			(float)fmin(1, fmax(0, ((window_relative_pos.y-button_size.y/2)-finger_at.y) / (size.y-button_size.y)))};
		vec2 new_val = min + pos_in_slider*range;
		if (step.x != 0) new_val.x = min.x + ((int)(((new_val.x - min.x) / step.x) + 0.5f)) * step.x;
		if (step.y != 0) new_val.y = min.y + ((int)(((new_val.y - min.x) / step.y) + 0.5f)) * step.y;

		new_percent = {
			range.x == 0 ? 0.5f : (new_val.x - min.x) / range.x,
			range.y == 0 ? 0.5f : (new_val.y - min.y) / range.y };
		*out_button_center = {
			window_relative_pos.x - (new_percent.x * (size.x - button_size.x) + button_size.x/2.0f),
			window_relative_pos.y - (new_percent.y * (size.y - button_size.y) + button_size.y/2.0f) };

		// Play tick sound as the value updates
		if (value->x != new_val.x || value->y != new_val.y) {
			
			if (step.x != 0 || step.y != 0) {
				// Play on every change if there's a user specified step value
				ui_play_sound_on(ui_vis_slider_line, skui_interactors[*out_interactor].capsule_end_world);
			} else {
				// If no user specified step, then we'll do a set number of
				// clicks across the whole bar.
				const int32_t click_steps = 10;

				int32_t old_quantize = (int32_t)(percent    .x * click_steps + 0.5f) + (int32_t)(percent    .y * click_steps + 0.5f) * 7000;
				int32_t new_quantize = (int32_t)(new_percent.x * click_steps + 0.5f) + (int32_t)(new_percent.y * click_steps + 0.5f) * 7000;

				if (old_quantize != new_quantize) {
					ui_play_sound_on(ui_vis_slider_line, skui_interactors[*out_interactor].capsule_end_world);
				}
			}
		}

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
		interactor_set_focus(-1, id, false, 0, vec3_zero);
	} else {
		for (int32_t i = 0; i < skui_interactors.count; i++) {
			interactor_t* actor = &skui_interactors[i];
			// Skip this if something else has some focus!
			if (((actor->events & event_mask) == 0) || interactor_is_preoccupied(i, id, false))
				continue;

			// Check to see if the handle has focus
			bool  has_hand_attention  = actor->active_prev == id;
			bool  is_far_interact     = false;
			float hand_attention_dist = 0;
			vec3  at;
			if (interactor_check_box(actor, handle_bounds, &at, &hand_attention_dist)) {
				has_hand_attention = true;

				if (actor->pinch_state & button_state_just_active) {
					ui_play_sound_on(ui_vis_handle, actor->capsule_end_world);

					actor->active = id;
					actor->interaction_start_position      = actor->position;
					actor->interaction_start_orientation   = actor->orientation;
					actor->interaction_start_motion_anchor = actor->motion_anchor;

					actor->interaction_pt_position         = handle_pose.position;
					actor->interaction_pt_orientation      = handle_pose.orientation;
					actor->interaction_pt_pivot            = at;

					actor->interaction_intersection_local  = matrix_transform_pt(matrix_invert(matrix_trs(actor->position, actor->orientation)), hierarchy_to_world_point(at));
				}
			}
			button_state_ focused = interactor_set_focus(i, id, has_hand_attention, hand_attention_dist, vec3_zero);


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
							vec3  local_pt     = matrix_transform_pt(to_handle_parent_local, hierarchy_to_world_point(actor->interaction_pt_pivot));
							vec3  local_head   = matrix_transform_pt(to_handle_parent_local, input_head()->position);
							float head_xz_lerp = fminf(1, vec2_distance_sq({ local_head.x, local_head.z }, { local_pt.x, local_pt.z }) / 0.1f);
							vec3  handle_center= matrix_transform_pt(pose_matrix(handle_pose), handle_bounds.center);
							// Previously, facing happened from a point
							// influenced by the hand-grip position:
							// vec3  handle_center= { handle_pose.position.x, local_pt[i].y, handle_pose.position.z };
							vec3  look_from    = vec3_lerp(local_pt, handle_center, head_xz_lerp);

							dest_rot = quat_lookat_up(look_from, local_head, matrix_transform_dir(to_handle_parent_local, vec3_up));
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
						ui_play_sound_off(ui_vis_handle, actor->capsule_end_world);
					}
					ui_pop_surface();
					ui_push_surface(handle_pose);
				}
			}
		}
	}

	if (draw) {
		ui_draw_el(ui_vis_handle,
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

int32_t interactor_create(interactor_type_ type, interactor_event_ events) {
	interactor_t result = {};
	result.type   = type;
	result.events = events;
	return skui_interactors.add(result);
}

///////////////////////////////////////////

void interactor_update(int32_t interactor, vec3 capsule_start, vec3 capsule_end, float capsule_radius, vec3 motion_pos, quat motion_orientation, vec3 motion_anchor, button_state_ active, button_state_ tracked) {
	if (interactor < 0 || interactor >= skui_interactors.count) return;
	interactor_t *actor = &skui_interactors[interactor];
	actor->capsule_start_world = capsule_start;
	actor->capsule_end_world   = capsule_end;
	actor->capsule_radius      = capsule_radius;
	actor->position            = motion_pos;
	actor->orientation         = motion_orientation;
	actor->motion_anchor       = motion_anchor;
	actor->tracked             = tracked;
	actor->pinch_state         = active;

	// Don't let the hand trigger things while popping in and out of
	// tracking
	if (actor->tracked & button_state_just_active) {
		actor->orientation_prev = actor->orientation;
		actor->position_prev    = actor->position;
	}
}

///////////////////////////////////////////

bool32_t interactor_check_box(const interactor_t* actor, bounds_t box, vec3* out_at, float* out_priority) {
	*out_priority = FLT_MAX;
	*out_at       = vec3_zero;

	if (!(actor->tracked & button_state_active))
		return false;

	if (skui_show_volumes)
		render_add_mesh(skui_box_dbg, skui_mat_dbg, matrix_trs(box.center, quat_identity, box.dimensions));

	switch (actor->type) {
	case interactor_type_point: {
		bool32_t result = bounds_capsule_contains(box, actor->capsule_start, actor->capsule_end, actor->capsule_radius);
		if (result) {
			*out_at       = actor->capsule_end;
			*out_priority = bounds_sdf_manhattan(box, *out_at);
		}
		return result;
	} break;
	case interactor_type_line: {
		ray_t    ray    = { actor->capsule_start, actor->capsule_end - actor->capsule_start };
		float    dist   = 0;
		bool32_t result = bounds_ray_intersect_dist(box, ray, &dist);
		if (result && dist <= 1) {
			*out_at       = ray.pos + dist * ray.dir;
			*out_priority = bounds_sdf_manhattan(box, *out_at) + vec3_distance_sq(ray.pos, *out_at);
		}
		return result;
	} break;
	default: return false;
	}
}

///////////////////////////////////////////

void interactor_plate_1h(id_hash_t id, interactor_event_ event_mask, vec3 plate_start, vec2 plate_size, button_state_ *out_focus_state, int32_t *out_interactor, vec3 *out_interaction_at_local) {
	*out_interactor           = -1;
	*out_focus_state          = button_state_inactive;
	*out_interaction_at_local = vec3_zero;

	if (!ui_is_enabled()) { return; }

	//if (skui_preserve_keyboard_stack.last()) {
	///	skui_preserve_keyboard_ids_write->add(id);
	//}

	for (int32_t i = 0; i < skui_interactors.count; i++) {
		const interactor_t *actor = &skui_interactors[i];
		if (((actor->events & event_mask) == 0) ||
			interactor_is_preoccupied(i, id, false))
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
		bounds_t bounds = {};
		if (actor->focused_prev != id) {
			bounds = size_box({ plate_start.x, plate_start.y, plate_start.z - actor->capsule_radius*2 }, { plate_size.x, plate_size.y, 0.0001f });
		} else {
			float depth = fmaxf(0.0001f, 8 * actor->capsule_radius);
			bounds = size_box({ plate_start.x, plate_start.y, (plate_start.z + depth) - actor->capsule_radius*2 }, { plate_size.x, plate_size.y, depth });
		}

		float         priority = 0;
		vec3          interact_at;
		bool          in_box   = interactor_check_box(actor, bounds, &interact_at, &priority);
		button_state_ focus    = interactor_set_focus(i, id, in_box, priority, plate_start-vec3{plate_size.x/2, plate_size.y/2, 0});
		if (focus != button_state_inactive) {
			*out_interactor           = i;
			*out_focus_state          = focus;
			*out_interaction_at_local = interact_at;
		}
	}

	if (*out_interactor == -1)
		*out_interactor = interactor_last_focused(id);
}

///////////////////////////////////////////

void ui_box_interaction_1h(id_hash_t id, interactor_event_ event_mask, vec3 box_unfocused_start, vec3 box_unfocused_size, vec3 box_focused_start, vec3 box_focused_size, button_state_ *out_focus_state, int32_t *out_interactor) {
	*out_interactor  = -1;
	*out_focus_state = button_state_inactive;
	
	skui_last_element = id;

	// If the element is disabled, unfocus it and ditch out
	if (!ui_is_enabled()) {
		*out_focus_state = interactor_set_focus(-1, id, false, 0, vec3_zero);
		return;
	}

	if (skui_preserve_keyboard_stack.last()) {
		skui_preserve_keyboard_ids_write->add(id);
	}

	for (int32_t i = 0; i < skui_interactors.count; i++) {
		const interactor_t* actor = &skui_interactors[i];
		if (((actor->events & event_mask) == 0) || 
			interactor_is_preoccupied(i, id, false))
			continue;

		bool     was_focused = skui_interactors[i].focused_prev == id;
		bounds_t bounds      = was_focused
			? ui_size_box(box_focused_start,   box_focused_size)
			: ui_size_box(box_unfocused_start, box_unfocused_size);

		vec3  at;
		float priority;
		bool  in_box = interactor_check_box(actor, bounds, &at, &priority);

		button_state_ focus = interactor_set_focus(i, id, in_box, priority, bounds.center);
		if (focus != button_state_inactive) {
			*out_interactor        = i;
			*out_focus_state = focus;
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

button_state_ interactor_set_focus(int32_t interactor, id_hash_t for_el_id, bool32_t focused, float priority, vec3 element_center_local) {
	if (interactor < 0 || interactor >= skui_interactors.count) {
		for (int32_t i = 0; i < skui_interactors.count; i++)
			if (skui_interactors[i].active_prev == for_el_id) { interactor = i; break; }
		if (interactor < 0 || interactor >= skui_interactors.count) return button_state_inactive;
	}
	bool was_focused = skui_interactors[interactor].focused_prev == for_el_id;
	bool is_focused  = false;

	if (focused && priority <= skui_interactors[interactor].focus_priority) {
		is_focused = focused;
		skui_interactors[interactor].focused            = for_el_id;
		skui_interactors[interactor].focus_priority     = priority;
		skui_interactors[interactor].focus_center_world = hierarchy_to_world_point( element_center_local );
	}

	button_state_ result = button_state_inactive;
	if ( is_focused                ) result  = button_state_active;
	if ( is_focused && !was_focused) result |= button_state_just_active;
	if (!is_focused &&  was_focused) result |= button_state_just_inactive;
	return result;
}

///////////////////////////////////////////

button_state_ interactor_set_active(int32_t interactor, id_hash_t for_el_id, bool32_t active) {
	if (interactor == -1) return button_state_inactive;

	bool was_active = skui_interactors[interactor].active_prev == for_el_id;
	bool is_active  = false;

	if (active && (was_active || skui_interactors[interactor].focused_prev == for_el_id || skui_interactors[interactor].focused == for_el_id)) {
		is_active = active;
		skui_interactors[interactor].active = for_el_id;
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

bool32_t interactor_is_preoccupied(int32_t interactor, id_hash_t for_el_id, bool32_t include_focused) {
	const interactor_t &h = skui_interactors[interactor];
	return (include_focused && h.focused_prev != 0 && h.focused_prev != for_el_id)
		|| (h.active_prev != 0 && h.active_prev != for_el_id);
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

button_state_ ui_last_element_hand_used(handed_ hand) {
	return button_make_state(
		skui_interactors[hand].active_prev == skui_last_element || skui_interactors[hand].focused_prev_prev == skui_last_element,
		skui_interactors[hand].active      == skui_last_element || skui_interactors[hand].focused_prev      == skui_last_element);
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
	// TODO: needs interactor work
	return button_make_state(
		skui_interactors[handed_left].active_prev == skui_last_element || skui_interactors[handed_right].active_prev == skui_last_element,
		skui_interactors[handed_left].active      == skui_last_element || skui_interactors[handed_right].active      == skui_last_element);
}

///////////////////////////////////////////

button_state_ ui_last_element_focused() {
	// TODO: needs interactor work

	// Because focus can change at any point during the frame, we'll check
	// against the last two frame's focus ids, which are set in stone after the
	// frame ends.
	return button_make_state(
		skui_interactors[handed_left].focused_prev_prev == skui_last_element || skui_interactors[handed_right].focused_prev_prev == skui_last_element,
		skui_interactors[handed_left].focused_prev      == skui_last_element || skui_interactors[handed_right].focused_prev      == skui_last_element);
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

void ui_push_enabled(bool32_t enabled) {
	skui_enabled_stack.add(enabled);
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
	return skui_enabled_stack.last();
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
		const interactor_t& h = skui_interactors[i];
		if (interactor_is_preoccupied(i, focused_id, false) && h.focused_prev && skui_preserve_keyboard_ids_read->index_of(h.focused_prev) < 0) {
			return true;
		}
	}
	return false;
}

}