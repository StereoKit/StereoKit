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

int32_t skui_input_mode                 = 2;
int32_t skui_hand_interactors[6]        = { -1, -1, -1, -1 };
int32_t skui_mouse_interactor           = -1;
float   skui_controller_trigger_last[2] = { 0, 0 };
float   skui_ray_length[2]              = { 0, 0 };

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
	skui_hand_interactors[1] = interactor_create(interactor_type_point, interactor_event_pinch);
	skui_hand_interactors[2] = interactor_create(interactor_type_line,  (interactor_event_)(interactor_event_poke | interactor_event_pinch));
	skui_hand_interactors[3] = interactor_create(interactor_type_point, interactor_event_poke);
	skui_hand_interactors[4] = interactor_create(interactor_type_point,  interactor_event_pinch);
	skui_hand_interactors[5] = interactor_create(interactor_type_line,  (interactor_event_)(interactor_event_poke | interactor_event_pinch));

	skui_mouse_interactor    = interactor_create(interactor_type_line,  (interactor_event_)(interactor_event_poke | interactor_event_pinch));

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

void ui_show_ray(int32_t interactor, float *ref_length) {
	interactor_t* actor = &skui_interactors[interactor];
	if ((actor->tracked & button_state_active) == 0) return;

	float length         = 0.5f;
	vec3  uncentered_dir = vec3_normalize(actor->capsule_end_world  - actor->position);
	vec3  centered_dir   = uncentered_dir;
	if (actor->focused_prev != 0) {
		length       = vec3_distance (actor->focus_center_world,  actor->position);
		centered_dir = vec3_normalize(actor->focus_center_world - actor->position);
	}
	if (ref_length != nullptr) {
		*ref_length = math_lerp(*ref_length, actor->focused_prev == 0 ? 0: length, 16.0f * time_stepf_unscaled());
		length = *ref_length;
	}
	if (length < 0.001f) return;

	float width_mod = fminf(1,length/0.06f);

	const int32_t ct = 20;
	line_point_t pts[ct];
	for (int32_t i = 0; i < ct; i += 1) {
		float pct   = (float)i / (float)(ct - 1);
		float blend = pct * pct;
		float d     = pct * length;

		float pct_i = 1 - pct;
		float curve = sin(pct_i * pct_i * 3.14159f);
		float width = (0.002f + curve * 0.0015f) * width_mod;
		pts[i] = line_point_t{ actor->position + vec3_lerp(uncentered_dir*d, centered_dir*d, blend), width, color32{ 255,255,255,(uint8_t)(curve*width_mod*255) } };
	}
	line_add_listv(pts, ct);
}

///////////////////////////////////////////

void ui_core_hands_step() {
	for (int32_t i = 0; i < handed_max; i++) {
		const hand_t*    hand    = input_hand       ((handed_)i);
		const pointer_t* pointer = input_get_pointer(input_hand_pointer_id[i]);

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
			float hand_dist = vec3_distance(hand->palm.position, input_head()->position + vec3{0,-0.12,0});
			float ray_dist  = math_lerp(0.35f, 0.15f, math_saturate((hand_dist-0.1f) / 0.4f));
			interactor_update(skui_hand_interactors[i*3 + 2],
				pointer->ray.pos + pointer->ray.dir * ray_dist, pointer->ray.pos + pointer->ray.dir * 100, 0.01f,
				pointer->ray.pos,  pointer->orientation, input_head()->position + vec3{0,-0.12,0},
				hand->pinch_state, pointer->tracked);
			ui_show_ray(skui_hand_interactors[i*3 + 2], &skui_ray_length[i]);
		}
	}
}

///////////////////////////////////////////

void ui_core_controllers_step() {
	if (ui_far_interact_enabled() == false) return;

	for (int32_t i = 0; i < handed_max; i++) {
		const controller_t *ctrl = input_controller((handed_)i);

		// controller ray
		interactor_update(skui_hand_interactors[i*3 + 2],
			ctrl->aim.position, ctrl->aim.position + ctrl->aim.orientation*vec3_forward * 100, 0.01f,
			ctrl->aim.position, ctrl->aim.orientation, input_head()->position,
			button_make_state(skui_controller_trigger_last[i]>0.5f, ctrl->trigger>0.5f),
			ctrl->tracked);
		skui_controller_trigger_last[i] = ctrl->trigger;
		ui_show_ray(skui_hand_interactors[i*3 + 2], nullptr);
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
	}

	// auto-switch between hands and controllers
	if (skui_input_mode != 2) {
		skui_input_mode = input_hand_source(handed_right) == hand_source_articulated
			? 0
			: 1;
	}

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

	bool active = focus & button_state_active && !(focus & button_state_just_inactive);
	if (interact_type != ui_confirm_push && interactor != -1) {
		active = actor->pinch_state & button_state_active;
		// Focus can get lost if the user is dragging outside the box, so set
		// it to focused if it's still active.
		focus = interactor_set_focus(actor, id_hash, active || focus & button_state_active, 0, bounds.center);
	}
	result = interactor_set_active(actor, id_hash, active);

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

	interactor_plate_1h(id, interactor_event_poke,
		{ window_relative_pos.x, window_relative_pos.y, window_relative_pos.z - button_depth }, { size.x, size.y, button_depth },
		&out_focus_state, &interactor, &interaction_at);
	interactor_t* actor = interactor_get(interactor);

	// If a hand is interacting, adjust the button surface accordingly
	out_finger_offset = button_depth;
	if (out_focus_state & button_state_active) {
		bool pressed;
		if (actor->type == interactor_type_point) {
			out_finger_offset = -(interaction_at.z + actor->capsule_radius) - window_relative_pos.z;
			pressed = out_finger_offset < button_activation_depth;
		} else {
			pressed = actor->pinch_state & button_state_active;
			if (pressed) out_finger_offset = 0;
		}
		out_finger_offset = fminf(fmaxf(2 * mm2m, out_finger_offset), button_depth);
		out_button_state  = interactor_set_active(actor, id, pressed);
	} else if (out_focus_state & button_state_just_inactive) {
		out_button_state = interactor_set_active(actor, id, false);
	}
	
	if (out_button_state & button_state_just_active)
		ui_play_sound_on_off(ui_vis_button, id, hierarchy_to_world_point(ui_layout_last().center));

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
	float button_depth = skui_settings.depth;

	// Set up for getting the state of the sliders.
	*out_focus_state   = button_state_inactive;
	*out_active_state  = button_state_inactive;
	*out_interactor    = -1;
	*out_finger_offset = button_depth;
	*out_button_center = {
		window_relative_pos.x - (percent.x * (size.x - button_size.x) + button_size.x/2.0f),
		window_relative_pos.y - (percent.y * (size.y - button_size.y) + button_size.y/2.0f) };

	vec2 finger_at = {};
	interactor_t* actor = nullptr;
	vec3 activation_size  = vec3{ button_size.x, button_size.y, button_depth };
	vec3 activation_start = { out_button_center->x+activation_size.x/2.0f, out_button_center->y+activation_size.y/2.0f, window_relative_pos.z };
	if (confirm_method == ui_confirm_push) {
		ui_button_behavior_depth(activation_start, { activation_size.x, activation_size.y }, id, button_depth, button_depth / 2, *out_finger_offset, *out_active_state, *out_focus_state, out_interactor);

		actor = interactor_get(*out_interactor);
		
	} else if (confirm_method == ui_confirm_pinch || confirm_method == ui_confirm_variable_pinch) {
		ui_box_interaction_1h(id, interactor_event_pinch,
			activation_start, activation_size,
			activation_start, activation_size,
			out_focus_state, out_interactor);

		// Pinch confirm uses a handle that the user must pinch, in order to
		// drag it around the slider.
		actor = interactor_get(*out_interactor);
		if (actor != nullptr) {
			*out_active_state = interactor_set_active(actor, id, actor->pinch_state & button_state_active);
			// Focus can get lost if the user is dragging outside the box, so set
			// it to focused if it's still active.
			*out_focus_state = interactor_set_focus(actor, id, (*out_active_state) & button_state_active || (*out_focus_state) & button_state_active, 0, activation_start - activation_size/2);
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
				ui_play_sound_on(ui_vis_slider_line, hierarchy_to_world_point({ out_button_center->x, out_button_center->y, window_relative_pos.z }));
			} else {
				// If no user specified step, then we'll do a set number of
				// clicks across the whole bar.
				const int32_t click_steps = 10;

				int32_t old_quantize = (int32_t)(percent    .x * click_steps + 0.5f) + (int32_t)(percent    .y * click_steps + 0.5f) * 7000;
				int32_t new_quantize = (int32_t)(new_percent.x * click_steps + 0.5f) + (int32_t)(new_percent.y * click_steps + 0.5f) * 7000;

				if (old_quantize != new_quantize) {
					ui_play_sound_on(ui_vis_slider_line, hierarchy_to_world_point({ out_button_center->x, out_button_center->y, window_relative_pos.z }));
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
		interactor_set_focus(nullptr, id, false, 0, vec3_zero);
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
				hand_attention_dist += 0.1f; // penalty to prefer non-handle elements

				if (actor->pinch_state & button_state_just_active && actor->focused_prev == id) {
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
			} else { at = actor->interaction_pt_pivot; }
			button_state_ focused = interactor_set_focus(actor, id, has_hand_attention, hand_attention_dist, at);


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
	actor->capsule_start_world = actor->capsule_start = capsule_start;
	actor->capsule_end_world   = actor->capsule_end   = capsule_end;
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

void interactor_plate_1h(id_hash_t id, interactor_event_ event_mask, vec3 plate_start, vec3 plate_size, button_state_ *out_focus_state, int32_t *out_interactor, vec3 *out_interaction_at_local) {
	*out_interactor           = -1;
	*out_focus_state          = button_state_inactive;
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

		float    surface_offset = actor->type == interactor_type_point ? actor->capsule_radius*2 : 0;

		bool     was_focused = actor->focused_prev == id;
		bool     was_active  = actor->active_prev  == id;
		float    depth  = fmaxf(0.0001f, 8 * plate_size.z);
		bounds_t bounds = was_focused && actor->type == interactor_type_point
			? size_box({ plate_start.x, plate_start.y, (plate_start.z + depth) - surface_offset }, { plate_size.x, plate_size.y, depth })
			: size_box({ plate_start.x, plate_start.y, plate_start.z           - surface_offset }, { plate_size.x, plate_size.y, 0.0001f });

		float         priority = 0;
		vec3          interact_at;
		bool          in_box   = interactor_check_box(actor, bounds, &interact_at, &priority);
		button_state_ focus    = interactor_set_focus(actor, id, in_box || (actor->type != interactor_type_point && was_active), priority, plate_start-vec3{plate_size.x/2, plate_size.y/2, 0});
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

	if (!ui_is_enabled()) return;
	if (skui_preserve_keyboard_stack.last()) {
		skui_preserve_keyboard_ids_write->add(id);
	}

	for (int32_t i = 0; i < skui_interactors.count; i++) {
		interactor_t* actor = &skui_interactors[i];
		if (interactor_is_preoccupied(actor, id, event_mask, false))
			continue;

		bool     was_focused = actor->focused_prev == id;
		bounds_t bounds      = was_focused
			? ui_size_box(box_focused_start,   box_focused_size)
			: ui_size_box(box_unfocused_start, box_unfocused_size);

		vec3  at;
		float priority;
		bool  in_box = interactor_check_box(actor, bounds, &at, &priority);

		button_state_ focus = interactor_set_focus(actor, id, in_box, priority, bounds.center);
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

button_state_ interactor_set_focus(interactor_t* interactor, id_hash_t for_el_id, bool32_t focused, float priority, vec3 element_center_local) {
	if (interactor == nullptr) return button_state_inactive;
	bool was_focused = interactor->focused_prev == for_el_id;
	bool is_focused  = false;

	if (focused && priority <= interactor->focus_priority) {
		is_focused = focused;
		interactor->focused            = for_el_id;
		interactor->focus_priority     = priority;
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

void ui_push_enabled(bool32_t enabled, bool32_t ignore_parent) {
	skui_enabled_stack.add(ignore_parent == true
		? enabled
		: (enabled == true && ui_is_enabled() == true));
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