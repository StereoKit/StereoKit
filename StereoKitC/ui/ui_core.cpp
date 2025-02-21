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

struct layer_t {
	vec3         finger_pos   [handed_max];
	vec3         finger_prev  [handed_max];
	vec3         thumb_pos    [handed_max];
	vec3         pinch_pt_pos [handed_max];
	vec3         pinch_pt_prev[handed_max];
};

struct ui_id_t {
	uint64_t id;
};

///////////////////////////////////////////

ui_hand_t       skui_hand[2];
float           skui_finger_radius;
uint64_t        skui_last_element = 0xFFFFFFFFFFFFFFFF;
bool32_t        skui_show_volumes;

array_t<bool32_t>    skui_enabled_stack;
array_t<ui_id_t>     skui_id_stack;
array_t<layer_t>     skui_layers;
array_t<bool>        skui_preserve_keyboard_stack;
array_t<uint64_t>    skui_preserve_keyboard_ids[2];
array_t<uint64_t>   *skui_preserve_keyboard_ids_read;
array_t<uint64_t>   *skui_preserve_keyboard_ids_write;

///////////////////////////////////////////

void ui_core_init() {
	skui_finger_radius            = 0;
	skui_last_element             = 0xFFFFFFFFFFFFFFFF;
	skui_show_volumes             = false;
	skui_enabled_stack            = {};
	skui_id_stack                 = {};
	skui_layers                   = {};
	skui_preserve_keyboard_stack  = {};
	skui_preserve_keyboard_ids[0] = {};
	skui_preserve_keyboard_ids[1] = {};
	skui_hand[0]                  = {};
	skui_hand[1]                  = {};

	skui_preserve_keyboard_ids_read  = &skui_preserve_keyboard_ids[0];
	skui_preserve_keyboard_ids_write = &skui_preserve_keyboard_ids[1];

	skui_id_stack.add({ HASH_FNV64_START });
}

///////////////////////////////////////////

void ui_core_shutdown() {
	skui_enabled_stack           .free();
	skui_id_stack                .free();
	skui_layers                  .free();
	skui_preserve_keyboard_stack .free();
	skui_preserve_keyboard_ids[0].free();
	skui_preserve_keyboard_ids[1].free();
	skui_preserve_keyboard_ids_read  = nullptr;
	skui_preserve_keyboard_ids_write = nullptr;
	skui_last_element = 0xFFFFFFFFFFFFFFFF;
}

///////////////////////////////////////////

void ui_core_update() {
	skui_finger_radius = 0;
	const matrix *to_local = hierarchy_to_local();
	for (int32_t i = 0; i < handed_max; i++) {
		const hand_t    *hand    = input_hand((handed_)i);
		const pointer_t *pointer = input_get_pointer(input_hand_pointer_id[i]);

		bool was_ray_enabled = skui_hand[i].ray_enabled && !skui_hand[i].ray_discard;

		skui_finger_radius += hand->fingers[1][4].radius;
		skui_hand[i].finger_world_prev   = skui_hand[i].finger_world;
		skui_hand[i].finger_world        = hand->fingers[1][4].position;
		skui_hand[i].thumb_world         = hand->fingers[0][4].position;
		skui_hand[i].pinch_pt_world_prev = skui_hand[i].pinch_pt;
		skui_hand[i].pinch_pt_world      = hand->pinch_pt;
		skui_hand[i].focused_prev_prev   = skui_hand[i].focused_prev;
		skui_hand[i].focused_prev        = skui_hand[i].focused;
		skui_hand[i].active_prev         = skui_hand[i].active;

		skui_hand[i].focus_priority = FLT_MAX;
		skui_hand[i].focused = 0;
		skui_hand[i].active  = 0;
		skui_hand[i].finger       = matrix_transform_pt(*to_local, skui_hand[i].finger_world);
		skui_hand[i].finger_prev  = matrix_transform_pt(*to_local, skui_hand[i].finger_world_prev);
		skui_hand[i].thumb        = matrix_transform_pt(*to_local, skui_hand[i].thumb_world);
		skui_hand[i].pinch_pt     = matrix_transform_pt(*to_local, skui_hand[i].pinch_pt);
		skui_hand[i].pinch_pt_prev= matrix_transform_pt(*to_local, skui_hand[i].pinch_pt_prev);
		skui_hand[i].tracked      = hand->tracked_state & button_state_active;
		skui_hand[i].ray_enabled  = pointer->tracked > 0 && skui_hand[i].tracked && (vec3_dot(hand->palm.orientation * vec3_forward, input_head()->position - hand->palm.position) < 0);

		// Don't let the hand trigger things while popping in and out of
		// tracking
		if (hand->tracked_state & button_state_just_active) {
			skui_hand[i].finger_prev         = skui_hand[i].finger;
			skui_hand[i].finger_world_prev   = skui_hand[i].finger_world;
			skui_hand[i].pinch_pt_prev       = skui_hand[i].pinch_pt;
			skui_hand[i].pinch_pt_world_prev = skui_hand[i].pinch_pt_world;
		}

		// draw hand rays
		skui_hand[i].ray_visibility = math_lerp(skui_hand[i].ray_visibility,
			was_ray_enabled && ui_far_interact_enabled() && skui_hand[i].ray_enabled && !skui_hand[i].ray_discard ? 1.0f : 0.0f,
			20.0f * time_stepf_unscaled());
		if (skui_hand[i].focused_prev != 0) skui_hand[i].ray_visibility = 0;
		if (skui_hand[i].ray_visibility > 0.004f) {
			ray_t       r     = input_get_pointer(input_hand_pointer_id[i])->ray;
			const float scale = 2;
			line_point_t points[5] = {
				line_point_t{r.pos+r.dir*(0.07f              ), 0.001f,  color32{255,255,255,0}},
				line_point_t{r.pos+r.dir*(0.07f + 0.01f*scale), 0.0015f, color32{255,255,255,(uint8_t)(skui_hand[i].ray_visibility * 60 )}},
				line_point_t{r.pos+r.dir*(0.07f + 0.02f*scale), 0.0020f, color32{255,255,255,(uint8_t)(skui_hand[i].ray_visibility * 80)}},
				line_point_t{r.pos+r.dir*(0.07f + 0.07f*scale), 0.0015f, color32{255,255,255,(uint8_t)(skui_hand[i].ray_visibility * 25 )}},
				line_point_t{r.pos+r.dir*(0.07f + 0.11f*scale), 0.001f,  color32{255,255,255,0}} };
			line_add_listv(points, 5);
		}
		skui_hand[i].ray_discard = false;
	}
	skui_finger_radius /= (float)handed_max;

	// Clear current keyboard ignore elements
	skui_preserve_keyboard_ids_read->clear();
	array_t<uint64_t>* tmp = skui_preserve_keyboard_ids_read;
	skui_preserve_keyboard_ids_read  = skui_preserve_keyboard_ids_write;
	skui_preserve_keyboard_ids_write = tmp;
}

///////////////////////////////////////////

template<typename C>
button_state_ ui_volumei_at_g(const C *id, bounds_t bounds, ui_confirm_ interact_type, handed_ *out_opt_hand, button_state_ *out_opt_focus_state) {
	uint64_t      id_hash = ui_stack_hash(id);
	button_state_ result  = button_state_inactive;
	button_state_ focus   = button_state_inactive;
	int32_t       hand    = -1;

	vec3 start = bounds.center + bounds.dimensions / 2;
	if (interact_type == ui_confirm_push) {
		ui_box_interaction_1h_poke(id_hash,
			start, bounds.dimensions,
			start, bounds.dimensions,
			&focus, &hand);
	} else {
		ui_box_interaction_1h_pinch(id_hash,
			start, bounds.dimensions,
			start, bounds.dimensions,
			&focus, &hand);
	}

	bool active = focus & button_state_active && !(focus & button_state_just_inactive);
	if (interact_type != ui_confirm_push && hand != -1) {
		active = input_hand((handed_)hand)->pinch_state & button_state_active;
		// Focus can get lost if the user is dragging outside the box, so set
		// it to focused if it's still active.
		focus = ui_focus_set(hand, id_hash, active || focus & button_state_active, 0);
	}
	result = ui_active_set(hand, id_hash, active);

	if (out_opt_hand        != nullptr) *out_opt_hand        = (handed_)hand;
	if (out_opt_focus_state != nullptr) *out_opt_focus_state = focus;
	return result;
}
button_state_ ui_volumei_at   (const char     *id, bounds_t bounds, ui_confirm_ interact_type, handed_ *out_opt_hand, button_state_ *out_opt_focus_state) { return ui_volumei_at_g<char    >(id, bounds, interact_type, out_opt_hand, out_opt_focus_state); }
button_state_ ui_volumei_at_16(const char16_t *id, bounds_t bounds, ui_confirm_ interact_type, handed_ *out_opt_hand, button_state_ *out_opt_focus_state) { return ui_volumei_at_g<char16_t>(id, bounds, interact_type, out_opt_hand, out_opt_focus_state); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_volume_at_g(const C *id, bounds_t bounds) {
	uint64_t id_hash = ui_stack_hash(id);
	bool     result  = false;

	skui_last_element = id_hash;

	for (int32_t i = 0; i < handed_max; i++) {
		bool     was_focused = skui_hand[i].focused_prev == id_hash;
		bounds_t size        = bounds;
		if (was_focused) {
			size.dimensions = bounds.dimensions + vec3_one*skui_settings.padding;
		}

		bool          in_box = skui_hand[i].tracked && ui_in_box(skui_hand[i].finger, skui_hand[i].finger_prev, skui_finger_radius, size);
		button_state_ state  = ui_focus_set(i, id_hash, in_box, 0);
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

	for (int32_t i = 0; i < handed_max; i++) {
		bool     was_active  = skui_hand[i].active_prev  != 0;
		bool     was_focused = skui_hand[i].focused_prev != 0 || was_active;
		if (was_active || was_focused)
			continue;

		if (skui_hand[i].tracked && ui_in_box(skui_hand[i].finger, skui_hand[i].finger_prev, skui_finger_radius, bounds)) {
			button_state_ state = input_hand((handed_)i)->pinch_state;
			if (state != button_state_inactive) {
				result = state;
				out_hand = (handed_)i;
			}
		}
	}

	return result;
}

///////////////////////////////////////////

void ui_button_behavior(vec3 window_relative_pos, vec2 size, uint64_t id, float& out_finger_offset, button_state_& out_button_state, button_state_& out_focus_state, int32_t* out_opt_hand) {
	ui_button_behavior_depth(window_relative_pos, size, id, skui_settings.depth, skui_settings.depth / 2, out_finger_offset, out_button_state, out_focus_state, out_opt_hand);
}

///////////////////////////////////////////

void ui_button_behavior_depth(vec3 window_relative_pos, vec2 size, uint64_t id, float button_depth, float button_activation_depth, float &out_finger_offset, button_state_ &out_button_state, button_state_ &out_focus_state, int32_t* out_opt_hand) {
	out_button_state = button_state_inactive;
	out_focus_state  = button_state_inactive;
	int32_t hand     = -1;

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
	float activation_plane = button_depth + skui_finger_radius*2;
	vec3  activation_start = window_relative_pos + vec3{ 0, 0, -activation_plane };
	vec3  activation_size  = vec3{ size.x, size.y, 0.0001f };

	vec3  box_size  = vec3{ size.x + 2*skui_finger_radius, size.y + 2*skui_finger_radius, activation_plane + 6*skui_finger_radius  };
	vec3  box_start = window_relative_pos + vec3{ skui_finger_radius, skui_finger_radius, -activation_plane + box_size.z };
	ui_box_interaction_1h_poke(id,
		activation_start, activation_size,
		box_start,        box_size,
		&out_focus_state, &hand);

	// If a hand is interacting, adjust the button surface accordingly
	out_finger_offset = button_depth;
	if (out_focus_state & button_state_active) {
		out_finger_offset = -(skui_hand[hand].finger.z+skui_finger_radius) - window_relative_pos.z;
		bool pressed  = out_finger_offset < button_activation_depth;
		out_button_state  = ui_active_set(hand, id, pressed);
		out_finger_offset = fminf(fmaxf(2*mm2m, out_finger_offset), button_depth);
	} else if (out_focus_state & button_state_just_inactive) {
		out_button_state = ui_active_set(hand, id, false);
	}
	
	if (out_button_state & button_state_just_active)
		ui_play_sound_on_off(ui_vis_button, id, skui_hand[hand].finger_world);

	if (out_opt_hand)
		*out_opt_hand = hand;
}

///////////////////////////////////////////

bool32_t _ui_handle_begin(uint64_t id, pose_t &handle_pose, bounds_t handle_bounds, bool32_t draw, ui_move_ move_type, ui_gesture_ allowed_gestures) {
	bool  result      = false;
	float color_blend = 0;

	skui_last_element = id;
	if (skui_preserve_keyboard_stack.last()) {
		skui_preserve_keyboard_ids_write->add(id);
	}

	matrix to_handle_parent_local       = *hierarchy_to_local();
	matrix handle_parent_local_to_world = *hierarchy_to_world();
	ui_push_surface(handle_pose);

	// If the handle is scale of zero, we don't actually want to draw or
	// interact with it
	if (vec3_magnitude_sq(handle_bounds.dimensions) == 0)
		return false;

	static vec3 start_2h_pos        = {};
	static quat start_2h_rot        = { quat_identity };
	static vec3 start_2h_handle_pos = {};
	static quat start_2h_handle_rot = { quat_identity };
	static vec3 start_handle_pos[2] = {};
	static quat start_handle_rot[2] = { quat_identity, quat_identity };
	static vec3 start_palm_pos  [2] = {};
	static quat start_palm_rot  [2] = { quat_identity, quat_identity };

	if (!ui_is_enabled() || move_type == ui_move_none) {
		ui_focus_set(-1, id, false, 0);
	} else {
		vec3          local_pt      [2] = {};
		button_state_ interact_state[2] = {};

		for (int32_t i = 0; i < handed_max; i++) {
			// Skip this if something else has some focus!
			if (ui_is_hand_preoccupied((handed_)i, id, false))
				continue;

			const hand_t *hand = input_hand((handed_)i);
			local_pt[i] = matrix_transform_pt(to_handle_parent_local, hand->pinch_pt);
			if (allowed_gestures == ui_gesture_pinch)
				interact_state[i] = hand->pinch_state;
			else if (allowed_gestures == ui_gesture_grip)
				interact_state[i] = hand->grip_state;
			else if (allowed_gestures == ui_gesture_pinch_grip)
				interact_state[i] = button_make_state(skui_hand[i].active_prev == id ||
					(hand->grip_state == button_state_active) || (hand->pinch_state == button_state_active),
					(hand->grip_state &  button_state_active) || (hand->pinch_state &  button_state_active));

			// Check to see if the handle has focus
			bool  has_hand_attention  = skui_hand[i].active_prev == id;
			bool  is_far_interact     = false;
			float hand_attention_dist = 0;
			if (skui_hand[i].tracked && ui_in_box(skui_hand[i].finger, skui_hand[i].thumb, skui_finger_radius, handle_bounds)) {
				has_hand_attention  = true;
				hand_attention_dist = bounds_sdf(handle_bounds, skui_hand[i].pinch_pt) + vec3_distance(skui_hand[i].pinch_pt, skui_hand[i].thumb);
			} else if (skui_hand[i].ray_enabled && ui_far_interact_enabled()) {
				pointer_t *ptr = input_get_pointer(input_hand_pointer_id[i]);
				vec3       at;
				if (ptr->tracked & button_state_active && bounds_ray_intersect(handle_bounds, hierarchy_to_local_ray(ptr->ray), &at)) {
					vec3  head_local = hierarchy_to_local_point(input_head()->position);
					float head_dist  = bounds_sdf(handle_bounds, head_local);
					float hand_dist  = bounds_sdf(handle_bounds, skui_hand[i].pinch_pt);

					if (head_dist < 0.65f || hand_dist < 0.2f) {
						// Reset id to zero if we found a window that's within
						// touching distance
						ui_focus_set(i, 0, true, 10);
						skui_hand[i].ray_discard = true;
					} else {
						has_hand_attention  = true;
						is_far_interact     = true;
						hand_attention_dist = head_dist + 10;
					}
				}
			}
			button_state_ focused = ui_focus_set(i, id, has_hand_attention, hand_attention_dist);

			// If this is the second frame this window has focus for, and it's
			// at a distance, then draw a line to it.
			vec3 from_pt = local_pt[i];
			if (is_far_interact && focused & button_state_active && !(focused & button_state_just_active)) {
				pointer_t *ptr   = input_get_pointer(input_hand_pointer_id[i]);
				vec3       start = hierarchy_to_local_point(ptr->ray.pos);
				line_add(start*0.75f, vec3_zero, { 50,50,50,0 }, { 255,255,255,255 }, 0.002f);
				from_pt = matrix_transform_pt(to_handle_parent_local, hierarchy_to_world_point(vec3_zero));
			}

			// This waits until the window has been focused for a frame,
			// otherwise the handle UI may try and use a frame of focus to move
			// around a bit.
			if (skui_hand[i].focused_prev == id) {
				color_blend = 1;
				if (interact_state[i] & button_state_just_active) {
					ui_play_sound_on(ui_vis_handle, skui_hand[i].finger_world);

					skui_hand[i].active = id;
					start_handle_pos[i] = handle_pose.position;
					start_handle_rot[i] = handle_pose.orientation;
					start_palm_pos  [i] = from_pt;
					start_palm_rot  [i] = matrix_transform_quat(to_handle_parent_local, hand->palm.orientation);
				}
				if (skui_hand[i].active_prev == id || skui_hand[i].active == id) {
					result = true;
					skui_hand[i].active  = id;
					skui_hand[i].focused = id;

					quat dest_rot = quat_identity;
					vec3 dest_pos;

					// If both hands are interacting with this handle, then we
					// do a two handed interaction from the second hand.
					if (skui_hand[0].active_prev == id && skui_hand[1].active_prev == id || (skui_hand[0].active == id && skui_hand[1].active == id)) {
						if (i == 1) {
							dest_rot = quat_lookat(local_pt[0], local_pt[1]);
							dest_pos = local_pt[0]*0.5f + local_pt[1]*0.5f;

							if ((input_hand(handed_left)->pinch_state & button_state_just_active) || (input_hand(handed_right)->pinch_state & button_state_just_active)) {
								start_2h_pos = dest_pos;
								start_2h_rot = dest_rot;
								start_2h_handle_pos = handle_pose.position;
								start_2h_handle_rot = handle_pose.orientation;
							}

							switch (move_type) {
							case ui_move_exact: {
								dest_rot = quat_lookat(local_pt[0], local_pt[1]);
								dest_rot = quat_difference(start_2h_rot, dest_rot);
							} break;
							case ui_move_face_user: {
								dest_rot = quat_lookat(local_pt[0], local_pt[1]);
								dest_rot = quat_difference(start_2h_rot, dest_rot);
							} break;
							case ui_move_pos_only: { dest_rot = quat_identity; } break;
							default:               { dest_rot = quat_identity; log_err("Unimplemented move type!"); } break;
							}

							hierarchy_set_enabled(false);
							line_add(matrix_transform_pt(handle_parent_local_to_world, local_pt[0]), matrix_transform_pt(handle_parent_local_to_world, dest_pos),    { 255,255,255,0   }, {255,255,255,128}, 0.001f);
							line_add(matrix_transform_pt(handle_parent_local_to_world, dest_pos),    matrix_transform_pt(handle_parent_local_to_world, local_pt[1]), { 255,255,255,128 }, {255,255,255,0  }, 0.001f);
							hierarchy_set_enabled(true);

							dest_pos = dest_pos + dest_rot * (start_2h_handle_pos - start_2h_pos);
							dest_rot = start_2h_handle_rot * dest_rot;

							handle_pose.position    = vec3_lerp (handle_pose.position,    dest_pos, 0.6f);
							handle_pose.orientation = quat_slerp(handle_pose.orientation, dest_rot, 0.4f);
						}

						// If one of the hands just let go, reset their
						// starting locations so the handle doesn't 'pop' when
						// switching back to 1-handed interaction.
						if ((input_hand(handed_left)->pinch_state & button_state_just_inactive) || (input_hand(handed_right)->pinch_state & button_state_just_inactive)) {
							start_handle_pos[i] = handle_pose.position;
							start_handle_rot[i] = handle_pose.orientation;
							start_palm_pos  [i] = from_pt;
							start_palm_rot  [i] = matrix_transform_quat( to_handle_parent_local, hand->palm.orientation);
						}
					} else {
						switch (move_type) {
						case ui_move_exact: {
							dest_rot = matrix_transform_quat(to_handle_parent_local, hand->palm.orientation);
							dest_rot = quat_difference(start_palm_rot[i], dest_rot);
						} break;
						case ui_move_face_user: {
							vec3  local_head   = matrix_transform_pt(to_handle_parent_local, input_head()->position - vec3{0,0.12f,0});
							float head_xz_lerp = fminf(1, vec2_distance_sq({ local_head.x, local_head.z }, { local_pt[i].x, local_pt[i].z }) / 0.1f);
							vec3  handle_center= matrix_transform_pt(pose_matrix(handle_pose), handle_bounds.center);
							// Previously, facing happened from a point
							// influenced by the hand-grip position:
							// vec3  handle_center= { handle_pose.position.x, local_pt[i].y, handle_pose.position.z };
							vec3  look_from    = vec3_lerp(local_pt[i], handle_center, head_xz_lerp);
							
							dest_rot = quat_lookat_up(look_from, local_head, matrix_transform_dir(to_handle_parent_local, vec3_up));
							dest_rot = quat_difference(start_handle_rot[i], dest_rot);
						} break;
						case ui_move_pos_only: { dest_rot = quat_identity; } break;
						default:               { dest_rot = quat_identity; log_err("Unimplemented move type!"); } break;
						}

						vec3 curr_pos = local_pt[i];
						dest_pos = curr_pos + dest_rot * (start_handle_pos[i] - start_palm_pos[i]);

						handle_pose.position    = vec3_lerp (handle_pose.position,    dest_pos, 0.6f);
						handle_pose.orientation = quat_slerp(handle_pose.orientation, start_handle_rot[i] * dest_rot, 0.4f); 
					}

					if (interact_state[i] & button_state_just_inactive) {
						skui_hand[i].active = 0;
						ui_play_sound_off(ui_vis_handle, skui_hand[i].finger_world);
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

void ui_box_interaction_1h_pinch(uint64_t id, vec3 box_unfocused_start, vec3 box_unfocused_size, vec3 box_focused_start, vec3 box_focused_size, button_state_ *out_focus_state, int32_t *out_hand) {
	*out_hand        = -1;
	*out_focus_state = button_state_inactive;
	
	skui_last_element = id;

	// If the element is disabled, unfocus it and ditch out
	if (!ui_is_enabled()) {
		*out_focus_state = ui_focus_set(-1, id, false, 0);
		return;
	}

	if (skui_preserve_keyboard_stack.last()) {
		skui_preserve_keyboard_ids_write->add(id);
	}

	for (int32_t i = 0; i < handed_max; i++) {
		if (ui_is_hand_preoccupied((handed_)i, id, false))
			continue;
		bool was_focused = skui_hand[i].focused_prev == id;
		vec3 box_start = box_unfocused_start;
		vec3 box_size  = box_unfocused_size;
		if (was_focused) {
			box_start = box_focused_start;
			box_size  = box_focused_size;
		}

		bounds_t bounds   = ui_size_box(box_start, box_size);
		bool     in_box   = skui_hand[i].tracked && ui_in_box(skui_hand[i].finger, skui_hand[i].thumb, skui_finger_radius, bounds);
		float    priority = in_box
			? bounds_sdf(bounds, skui_hand[i].pinch_pt) + vec3_distance(skui_hand[i].thumb, skui_hand[i].pinch_pt)
			: FLT_MAX;

		button_state_ focus  = ui_focus_set(i, id, in_box, priority);
		if (focus != button_state_inactive) {
			*out_hand        = i;
			*out_focus_state = focus;
		}
	}

	if (*out_hand == -1)
		*out_hand = ui_last_focused_hand(id);
}

///////////////////////////////////////////

void ui_box_interaction_1h_poke(uint64_t id, vec3 box_unfocused_start, vec3 box_unfocused_size, vec3 box_focused_start, vec3 box_focused_size, button_state_ *out_focus_state, int32_t *out_hand) {
	*out_hand        = -1;
	*out_focus_state = button_state_inactive;

	skui_last_element = id;

	// If the element is disabled, unfocus it and ditch out
	if (!ui_is_enabled()) {
		*out_focus_state = ui_focus_set(-1, id, false, 0);
		return;
	}

	if (skui_preserve_keyboard_stack.last()) {
		skui_preserve_keyboard_ids_write->add(id);
	}

	for (int32_t i = 0; i < handed_max; i++) {
		if (ui_is_hand_preoccupied((handed_)i, id, false))
			continue;
		bool was_focused = skui_hand[i].focused_prev == id;
		vec3 box_start = box_unfocused_start;
		vec3 box_size  = box_unfocused_size;
		if (was_focused) {
			box_start = box_focused_start;
			box_size  = box_focused_size;
		}

		bounds_t bounds   = ui_size_box(box_start, box_size);
		bool     in_box   = skui_hand[i].tracked && ui_in_box(skui_hand[i].finger, skui_hand[i].finger_prev, skui_finger_radius, bounds);
		float    priority = in_box
			? bounds_sdf(bounds, skui_hand[i].finger)
			: FLT_MAX;

		button_state_ focus = ui_focus_set(i, id, in_box, priority);
		if (focus != button_state_inactive) {
			*out_hand        = i;
			*out_focus_state = focus;
		}
	}

	if (*out_hand == -1)
		*out_hand = ui_last_focused_hand(id);
}

///////////////////////////////////////////

void ui_push_surface(pose_t surface_pose, vec3 layout_start, vec2 layout_dimensions) {
	vec3   right = surface_pose.orientation * vec3_right;
	matrix trs   = matrix_trs(surface_pose.position + right*layout_start, surface_pose.orientation);
	hierarchy_push(trs);

	skui_layers.add(layer_t{});
	ui_layout_push(layout_start, layout_dimensions, true);

	layer_t*      layer    = &skui_layers.last();
	const matrix *to_local = hierarchy_to_local();
	for (int32_t i = 0; i < handed_max; i++) {
		layer->finger_pos   [i] = skui_hand[i].finger        = matrix_transform_pt(*to_local, skui_hand[i].finger_world);
		layer->finger_prev  [i] = skui_hand[i].finger_prev   = matrix_transform_pt(*to_local, skui_hand[i].finger_world_prev);
		layer->thumb_pos    [i] = skui_hand[i].thumb         = matrix_transform_pt(*to_local, skui_hand[i].thumb_world);
		layer->pinch_pt_pos [i] = skui_hand[i].pinch_pt      = matrix_transform_pt(*to_local, skui_hand[i].pinch_pt_world);
		layer->pinch_pt_prev[i] = skui_hand[i].pinch_pt_prev = matrix_transform_pt(*to_local, skui_hand[i].pinch_pt_world_prev);
	}
}

///////////////////////////////////////////

void ui_pop_surface() {
	skui_layers.pop();
	ui_layout_pop();
	hierarchy_pop();

	if (skui_layers.count <= 0) {
		for (int32_t i = 0; i < handed_max; i++) {
			skui_hand[i].finger        = skui_hand[i].finger_world;
			skui_hand[i].finger_prev   = skui_hand[i].finger_world_prev;
			skui_hand[i].thumb         = skui_hand[i].thumb_world;
			skui_hand[i].pinch_pt      = skui_hand[i].pinch_pt_world;
			skui_hand[i].pinch_pt_prev = skui_hand[i].pinch_pt_world_prev;
		}
	} else {
		layer_t *layer = &skui_layers.last();
		for (int32_t i = 0; i < handed_max; i++) {
			skui_hand[i].finger        = layer->finger_pos[i];
			skui_hand[i].finger_prev   = layer->finger_prev[i];
			skui_hand[i].thumb         = layer->thumb_pos[i];
			skui_hand[i].pinch_pt      = layer->pinch_pt_pos[i];
			skui_hand[i].pinch_pt_prev = layer->pinch_pt_prev[i];
		}
	}
}

///////////////////////////////////////////

button_state_ ui_focus_set(int32_t hand, uint64_t for_el_id, bool32_t focused, float priority) {
	if (hand < 0 || hand > 1) {
		if      (skui_hand[0].active_prev == for_el_id) hand = 0;
		else if (skui_hand[1].active_prev == for_el_id) hand = 1;
		else return button_state_inactive;
	}
	bool was_focused = skui_hand[hand].focused_prev == for_el_id;
	bool is_focused  = false;

	if (focused && hand >= 0 && hand < 2 && priority <= skui_hand[hand].focus_priority) {
		is_focused = focused;
		skui_hand[hand].focused        = for_el_id;
		skui_hand[hand].focus_priority = priority;
	}

	button_state_ result = button_state_inactive;
	if ( is_focused                ) result  = button_state_active;
	if ( is_focused && !was_focused) result |= button_state_just_active;
	if (!is_focused &&  was_focused) result |= button_state_just_inactive;
	return result;
}

///////////////////////////////////////////

button_state_ ui_active_set(int32_t hand, uint64_t for_el_id, bool32_t active) {
	if (hand == -1) return button_state_inactive;

	bool was_active = skui_hand[hand].active_prev == for_el_id;
	bool is_active  = false;

	if (active && (was_active || skui_hand[hand].focused_prev == for_el_id || skui_hand[hand].focused == for_el_id)) {
		is_active = active;
		skui_hand[hand].active = for_el_id;
	}

	button_state_ result = button_state_inactive;
	if ( is_active               ) result  = button_state_active;
	if ( is_active && !was_active) result |= button_state_just_active; 
	if (!is_active &&  was_active) result |= button_state_just_inactive;
	return result;
}

///////////////////////////////////////////

bool32_t ui_id_focused(uint64_t id) {
	return skui_hand[0].focused_prev == id || skui_hand[1].focused_prev == id;
}

///////////////////////////////////////////

bool32_t ui_in_box(vec3 pt, vec3 pt_prev, float radius, bounds_t box) {
	if (skui_show_volumes)
		render_add_mesh(skui_box_dbg, skui_mat_dbg, matrix_trs(box.center, quat_identity, box.dimensions));
	return bounds_capsule_contains(box, pt, pt_prev, radius);
}

///////////////////////////////////////////

void ui_show_volumes(bool32_t show) {
	skui_show_volumes = show;
}

///////////////////////////////////////////

bool32_t ui_is_hand_preoccupied(handed_ hand, uint64_t for_el_id, bool32_t include_focused) {
	const ui_hand_t &h = skui_hand[hand];
	return (include_focused && h.focused_prev != 0 && h.focused_prev != for_el_id)
		|| (h.active_prev != 0 && h.active_prev != for_el_id);
}

///////////////////////////////////////////

int32_t ui_last_active_hand(uint64_t for_el_id) {
	if      (skui_hand[0].active_prev == for_el_id) return 0;
	else if (skui_hand[1].active_prev == for_el_id) return 1;
	return -1;
}

///////////////////////////////////////////

int32_t ui_last_focused_hand(uint64_t for_el_id) {
	if      (skui_hand[0].focused_prev == for_el_id) return 0;
	else if (skui_hand[1].focused_prev == for_el_id) return 1;
	return -1;
}

///////////////////////////////////////////

bool32_t ui_is_interacting(handed_ hand) {
	return skui_hand[hand].active_prev != 0 || skui_hand[hand].focused_prev != 0;
}

///////////////////////////////////////////

button_state_ ui_last_element_hand_used(handed_ hand) {
	return button_make_state(
		skui_hand[hand].active_prev == skui_last_element || skui_hand[hand].focused_prev_prev == skui_last_element,
		skui_hand[hand].active      == skui_last_element || skui_hand[hand].focused_prev      == skui_last_element);
}

///////////////////////////////////////////

button_state_ ui_last_element_hand_active(handed_ hand) {
	return button_make_state(
		skui_hand[hand].active_prev == skui_last_element,
		skui_hand[hand].active      == skui_last_element);
}

///////////////////////////////////////////

button_state_ ui_last_element_hand_focused(handed_ hand) {
	// Because focus can change at any point during the frame, we'll check
	// against the last two frame's focus ids, which are set in stone after the
	// frame ends.
	return button_make_state(
		skui_hand[hand].focused_prev_prev == skui_last_element,
		skui_hand[hand].focused_prev      == skui_last_element);
}

///////////////////////////////////////////

button_state_ ui_last_element_active() {
	return button_make_state(
		skui_hand[handed_left].active_prev == skui_last_element || skui_hand[handed_right].active_prev == skui_last_element,
		skui_hand[handed_left].active      == skui_last_element || skui_hand[handed_right].active      == skui_last_element);
}

///////////////////////////////////////////

button_state_ ui_last_element_focused() {
	// Because focus can change at any point during the frame, we'll check
	// against the last two frame's focus ids, which are set in stone after the
	// frame ends.
	return button_make_state(
		skui_hand[handed_left].focused_prev_prev == skui_last_element || skui_hand[handed_right].focused_prev_prev == skui_last_element,
		skui_hand[handed_left].focused_prev      == skui_last_element || skui_hand[handed_right].focused_prev      == skui_last_element);
}

///////////////////////////////////////////

uint64_t hash_fnv64_string_16(const char16_t* string, uint64_t start_hash = HASH_FNV64_START) {
	uint64_t hash = start_hash;
	while (*string != '\0') {
		hash = (hash ^ ((*string & 0xFF00) >> 2)) * 1099511628211;
		hash = (hash ^ ( *string & 0x00FF      )) * 1099511628211;
		string++;
	}
	return hash;
}

///////////////////////////////////////////

uint64_t ui_stack_hash(const char *string) {
	return skui_id_stack.count > 0 
		? hash_fnv64_string(string, skui_id_stack.last().id) 
		: hash_fnv64_string(string);
}

///////////////////////////////////////////

uint64_t ui_stack_hash_16(const char16_t *string) {
	return skui_id_stack.count > 0 
		? hash_fnv64_string_16(string, skui_id_stack.last().id) 
		: hash_fnv64_string_16(string);
}

///////////////////////////////////////////

uint64_t ui_stack_hashi(int32_t id) {
	return skui_id_stack.count > 0 
		? hash_fnv64_data(&id, sizeof(int32_t), skui_id_stack.last().id) 
		: hash_fnv64_data(&id, sizeof(int32_t));
}

///////////////////////////////////////////

uint64_t ui_push_id(const char *id) {
	uint64_t result = ui_stack_hash(id);
	skui_id_stack.add({ result });
	return result;
}

///////////////////////////////////////////

uint64_t ui_push_id_16(const char16_t *id) {
	uint64_t result = ui_stack_hash(id);
	skui_id_stack.add({ result });
	return result;
}

///////////////////////////////////////////

uint64_t ui_push_idi(int32_t id) {
	uint64_t result = ui_stack_hashi(id);
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

bool32_t ui_keyboard_focus_lost(uint64_t focused_id) {
	for (int32_t i = 0; i < handed_max; i++) {
		const ui_hand_t& h = skui_hand[i];
		if (ui_is_hand_preoccupied((handed_)i, focused_id, false) && h.focused_prev && skui_preserve_keyboard_ids_read->index_of(h.focused_prev) < 0) {
			return true;
		}
	}
	return false;
}

}