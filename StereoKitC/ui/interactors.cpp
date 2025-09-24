/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2025 Nick Klingensmith
 * Copyright (c) 2024-2025 Qualcomm Technologies, Inc.
 */

#include "interactors.h"

#include "../_stereokit.h"
#include "../sk_math.h"
#include "../libraries/array.h"
#include "../libraries/gen_ids.h"
#include "../systems/input.h"

#include <float.h>
#include <limits.h>

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

struct interactor_state_t {
	array_t<_interactor_t> interactors;
	int32_t                interactors_live;
	id_hash_t              last_element;
	bool32_t               show_volumes;
	material_t             show_volume_mat;
	mesh_t                 show_volume_mesh;

	array_t<bool32_t>      enabled_stack;
	array_t<id_hash_t>     id_stack;

	array_t<bool>          preserve_keyboard_stack;
	id_hash_t              keyboard_focus_lost;
	id_hash_t              keyboard_focus_lost_write;
};
static interactor_state_t local = {};

///////////////////////////////////////////

void interaction_init() {
	local = {};
	local.id_stack.add({ default_hash_root });

	local.show_volume_mesh = mesh_find       (default_id_mesh_cube);
	local.show_volume_mat  = material_copy_id(default_id_material_unlit);
	material_set_transparency(local.show_volume_mat, transparency_add);
	material_set_color       (local.show_volume_mat, "color", color_hsv(0.4f, 0.6f, 0.2f, 1));
	material_set_depth_write (local.show_volume_mat, false);
}

///////////////////////////////////////////

void interaction_shutdown() {
	mesh_release    (local.show_volume_mesh);
	material_release(local.show_volume_mat);
	local.interactors            .free();
	local.enabled_stack          .free();
	local.id_stack               .free();
	local.preserve_keyboard_stack.free();
	local = {};
}

///////////////////////////////////////////
#include <stdio.h>
void interaction_update() {
	const matrix *to_local = hierarchy_to_local();
	pose_t        head     = input_head();

	for (int32_t i = 0; i < local.interactors.count; i++) {
		_interactor_t* actor = &local.interactors[i];

		// Visualize the interactors.
		/*if (actor->tracked & button_state_active) {
			color32 color_start = {255,255,255,255};
			color32 color_end   = {255,255,255,0};
			if (actor->focused_prev != 0) {
				color_start = {255,255,0,255};
				color_end   = {255,255,0,0};
			}
			if (actor->active_prev != 0) {
				color_start = {0,255,0,255};
				color_end   = {0,255,0,0};
			}
			line_add(actor->capsule_start_world, actor->capsule_end_world, color_start, color_end, 0.003f);

			if (actor->focused != 0) {
				vec3 pt = matrix_transform_pt(pose_matrix(actor->focus_pose_world), actor->focus_bounds_local.center);
				mesh_draw(local.show_volume_mesh, local.show_volume_mat, matrix_trs(pt, actor->focus_pose_world.orientation, actor->focus_bounds_local.dimensions));
			}
			if (actor->focus_priority < 1000) {
				char txt[32];
				snprintf(txt, 32, "%.2f", actor->focus_priority);
				text_add_at(txt, matrix_trs(actor->capsule_start_world, quat_lookat(actor->capsule_start_world, head.position)), 1);
			}
		}*/

		actor->focused_prev_prev   = actor->focused_prev;
		actor->focused_prev        = actor->focused;
		actor->active_prev_prev    = actor->active_prev;
		actor->active_prev         = actor->active;
		actor->motion_prev         = actor->motion;

		actor->secondary_motion    = vec3_zero;
		actor->focus_priority      = SHRT_MIN;
		actor->focus_distance      = FLT_MAX;
		actor->focused             = 0;
		actor->active              = 0;

		actor->tracked             = button_state_inactive;
		actor->pinch_state         = button_state_inactive;
	}

	// Clear current keyboard ignore elements
	local.keyboard_focus_lost       = local.keyboard_focus_lost_write;
	local.keyboard_focus_lost_write = 0;
}

///////////////////////////////////////////

inline bounds_t size_box(vec3 top_left, vec3 dimensions) {
	return { top_left - dimensions / 2, dimensions };
}

///////////////////////////////////////////

void interaction_1h_plate(id_hash_t id, interactor_event_ event_mask, int32_t priority, vec3 plate_start, vec3 plate_size, button_state_ *out_focus_candidacy, interactor_t* out_interactor, vec3 *out_interaction_at_local) {
	*out_interactor           = -1;
	*out_focus_candidacy      = button_state_inactive;
	*out_interaction_at_local = vec3_zero;

	local.last_element = id;
	if (!ui_is_enabled()) return;

	for (int32_t i = 0; i < local.interactors.count; i++) {
		_interactor_t *actor = &local.interactors[i];
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

		float    surface_offset = actor->activation_type == interactor_activation_position ? actor->capsule_radius*2 : 0;

		bool     was_focused = actor->focused_prev == id;
		bool     was_active  = actor->active_prev  == id;
		float    depth  = fmaxf(0.0001f, 8 * plate_size.z);
		bounds_t bounds = was_focused && actor->activation_type == interactor_activation_position
			? size_box({ plate_start.x, plate_start.y, (plate_start.z + depth) - surface_offset }, { plate_size.x, plate_size.y, depth })
			: size_box({ plate_start.x, plate_start.y, plate_start.z           - surface_offset }, { plate_size.x, plate_size.y, 0.0001f });

		float         distance = 0;
		vec3          interact_at = vec3_zero;
		bool          facing   = actor->shape_type == interactor_type_line ? vec3_dot(hierarchy_to_local_direction(actor->capsule_end_world - actor->capsule_start_world), vec3_forward) < 0 : true;
		bool          in_box   = facing && interactor_check_box(actor, bounds, &interact_at, &distance);
		button_state_ focus    = interactor_set_focus(actor, id, in_box, (actor->activation_type == interactor_activation_state && was_active), priority, distance, pose_identity, size_box({ plate_start.x, plate_start.y, plate_start.z - surface_offset }, { plate_size.x, plate_size.y, 0.0001f }), interact_at);
		if (focus != button_state_inactive) {
			*out_interactor           = gen_id_make(i, actor->generation);
			*out_focus_candidacy      = focus;
			*out_interaction_at_local = interact_at;
		}
	}
}

///////////////////////////////////////////

void interaction_1h_box(id_hash_t id, interactor_event_ event_mask, int32_t priority, vec3 box_unfocused_start, vec3 box_unfocused_size, vec3 box_focused_start, vec3 box_focused_size, button_state_ *out_focus_candidacy, interactor_t* out_interactor) {
	*out_interactor      = -1;
	*out_focus_candidacy = button_state_inactive;
	
	local.last_element = id;
	if (!ui_is_enabled()) return;

	for (int32_t i = 0; i < local.interactors.count; i++) {
		_interactor_t* actor = &local.interactors[i];
		if (interactor_is_preoccupied(actor, id, event_mask, false))
			continue;

		bool     was_focused = actor->focused_prev == id;
		bool     was_active  = actor->active_prev == id;
		bounds_t bounds      = was_focused
			? size_box(box_focused_start,   box_focused_size)
			: size_box(box_unfocused_start, box_unfocused_size);

		vec3          at;
		float         distance;
		bool          in_box = interactor_check_box(actor, bounds, &at, &distance);
		button_state_ focus  = interactor_set_focus(actor, id, in_box, (actor->activation_type == interactor_activation_state && was_active), priority, distance, pose_identity, size_box(box_unfocused_start, box_unfocused_size), at);
		if (focus != button_state_inactive) {
			*out_interactor      = gen_id_make(i, actor->generation);
			*out_focus_candidacy = focus;
		}
	}
}

///////////////////////////////////////////

bool32_t interaction_handle(id_hash_t id, int32_t priority, pose_t* ref_handle_pose, bounds_t handle_bounds, ui_move_ move_type, ui_gesture_ allowed_gestures) {
	bool result = false;

	local.last_element = id;
	if (!ui_is_enabled() || move_type == ui_move_none) return false;

	matrix to_handle_parent_local = *hierarchy_to_local();
	matrix handle_parent_to_world = *hierarchy_to_world();
	hierarchy_push_pose(*ref_handle_pose);

	interactor_event_ event_mask = (interactor_event_)0;
	if (allowed_gestures & ui_gesture_pinch) event_mask = (interactor_event_)(event_mask | interactor_event_pinch);
	if (allowed_gestures & ui_gesture_grip ) event_mask = (interactor_event_)(event_mask | interactor_event_grip );

	for (int32_t i = 0; i < local.interactors.count; i++) {
		_interactor_t* actor = &local.interactors[i];
		// Skip this if something else has some focus!
		if (interactor_is_preoccupied(actor, id, event_mask, false))
			continue;

		// Check to see if the handle has focus
		float hand_attention_dist = 0;
		vec3  at;
		bool  intersects = interactor_check_box(actor, handle_bounds, &at, &hand_attention_dist);
		button_state_ focused = interactor_set_focus (actor, id, intersects, actor->active_prev == id, priority, hand_attention_dist, pose_identity, handle_bounds, at);
		button_state_ active  = interactor_set_active(actor, id,
			(actor->pinch_state & button_state_just_active && actor->focused_prev == id) ||
			(actor->pinch_state & button_state_active      && actor->active_prev  == id));

		if (active & button_state_just_active) {
			actor->interaction_start_motion             = actor->motion;
			actor->interaction_start_motion_anchor      = actor->motion_anchor;
			actor->interaction_start_el                 = *ref_handle_pose;
			actor->interaction_start_el_pivot           = at;
			actor->interaction_secondary_motion_total   = vec3_zero;
			actor->interaction_intersection_local       = matrix_transform_pt(pose_matrix_inv(actor->motion), hierarchy_to_world_point(at));
		}

		if (active & button_state_active) {
			result = true;

			pose_t head     = input_head();
			quat   dest_rot = quat_identity;
			switch (move_type) {
			case ui_move_exact: {
				dest_rot = matrix_transform_quat(handle_parent_to_world, actor->interaction_start_el.orientation) * quat_difference(actor->interaction_start_motion.orientation, actor->motion.orientation);
			} break;
			case ui_move_face_user: {
				if (device_display_get_type() == display_type_flatscreen) {
					// If we're on a flat screen, facing the window is
					// a better experience than facing the user.
					dest_rot = quat_from_angles(0, 180, 0) * head.orientation;
				} else {
					// We can't use the head position directly, it's
					// more of a device position that matches the
					// center of the eyes, and not the center of the
					// head.
					const float head_center_dist = 5    * cm2m; // Quarter head length (20cm front to back)
					const float head_height      = 7.5f * cm2m; // Almost quarter head height (25cm top to bottom)
					vec3 eye_center  = head.position;
					vec3 head_center = eye_center  + head.orientation * vec3{0, 0, head_center_dist};
					vec3 face_point  = head_center + vec3{0, -head_height, 0};

					// Previously, facing happened from a point
					// influenced by the hand-grip position:
					// vec3 world_handle_center = { handle_pose.position.x, local_pt[i].y, handle_pose.position.z };
					vec3  world_handle_center = hierarchy_to_world_point(handle_bounds.center);
					vec3  world_pt            = hierarchy_to_world_point(actor->interaction_start_el_pivot);

					float head_xz_lerp = fminf(1, vec2_distance_sq({ face_point.x, face_point.z }, { world_pt.x, world_pt.z }) / 0.1f);
					vec3  look_from    = vec3_lerp(world_pt, world_handle_center, head_xz_lerp);

					dest_rot = quat_lookat_up(look_from, face_point, vec3_up);
				}
			} break;
			case ui_move_pos_only: { dest_rot = matrix_transform_quat(handle_parent_to_world, actor->interaction_start_el.orientation); } break;
			default:               { dest_rot = matrix_transform_quat(handle_parent_to_world, actor->interaction_start_el.orientation); log_err("Unimplemented move type!"); } break;
			}

			// Amplify the movement in and out, so that objects at a
			// distance can be manipulated easier.
			const float amplify_push = 1.5f;
			const float amplify_pull = 2;
			float       amplify_factor =
				fmaxf(0.01f, vec3_distance(actor->motion.position,                   actor->motion_anchor)) /
				fmaxf(0.01f, vec3_distance(actor->interaction_start_motion.position, actor->interaction_start_motion_anchor));
			amplify_factor = powf(amplify_factor, amplify_factor > 1 ? amplify_push : amplify_pull);

			// Find secondary motion from scroll wheels/analog sticks, etc.
			vec3 secondary_motion = vec3_zero;
			if      (actor->secondary_motion_dimensions == 1) { secondary_motion = actor->motion.orientation * vec3{ 0,0, actor->interaction_secondary_motion_total.x }; }
			else if (actor->secondary_motion_dimensions == 2) { secondary_motion = actor->motion.orientation * vec3{ 0,0,-actor->interaction_secondary_motion_total.y }; }
			else if (actor->secondary_motion_dimensions == 3) { secondary_motion = actor->motion.orientation * actor->interaction_secondary_motion_total; }

			vec3 pivot_new_position  = matrix_transform_pt(matrix_trs(actor->motion.position + secondary_motion, actor->motion.orientation, vec3_one * amplify_factor), actor->interaction_intersection_local);
			vec3 handle_world_offset = dest_rot * (-actor->interaction_start_el_pivot);
			vec3 dest_pos            = pivot_new_position + handle_world_offset;

			// Transform from world space, to the space the handle is in
			dest_pos = matrix_transform_pt  (to_handle_parent_local, dest_pos);
			dest_rot = matrix_transform_quat(to_handle_parent_local, dest_rot);

			ref_handle_pose->position    = vec3_lerp (ref_handle_pose->position,    dest_pos, 0.6f);
			ref_handle_pose->orientation = quat_slerp(ref_handle_pose->orientation, dest_rot, 0.4f);

			if (actor->pinch_state & button_state_just_inactive) {
				actor->active = 0;
			}

			// Update the focus pose with the updated position
			actor->focus_pose_world = matrix_transform_pose(handle_parent_to_world, *ref_handle_pose);
			matrix to_local_update = pose_matrix_inv(actor->focus_pose_world);
			vec3 update_at;
			if (bounds_capsule_intersect(handle_bounds, matrix_transform_pt(to_local_update, actor->capsule_start_world), matrix_transform_pt(to_local_update, actor->capsule_end_world), actor->capsule_radius, &update_at)) {
				actor->focus_intersection_local = update_at - actor->focus_bounds_local.center;
			}
		}
	}
	hierarchy_pop();
	return result;
}

///////////////////////////////////////////

interactor_t interactor_create(interactor_type_ shape_type, interactor_event_ events, interactor_activation_ activation_type, int32_t input_source_id, float capsule_radius, int32_t secondary_motion_dimensions) {
	local.interactors_live += 1;

	_interactor_t result = {};
	result.shape_type      = shape_type;
	result.events          = events;
	result.activation_type = activation_type;
	result.input_source_id = input_source_id;
	result.capsule_radius  = capsule_radius;
	result.secondary_motion_dimensions = secondary_motion_dimensions;
	result.min_distance    = -1000000;

	for (int32_t i = 0; i < local.interactors.count; i++) {
		if (gen_is_dead(local.interactors[i].generation)) {
			result.generation = gen_next_gen(local.interactors[i].generation);
			local.interactors[i] = result;
			return gen_id_make(i, result.generation);
		}
	}
	result.generation = 1;
	int32_t idx = local.interactors.add(result);
	return gen_id_make(idx, result.generation);
}

///////////////////////////////////////////

void interactor_destroy(interactor_t interactor) {
	_interactor_t* actor = _interactor_get(interactor);
	if (actor) {
		actor->generation = -actor->generation;
		local.interactors_live -= 1;
	}
}

///////////////////////////////////////////

int32_t interactor_count() {
	return local.interactors_live;
}

///////////////////////////////////////////

interactor_t interactor_get(int32_t index) {
	int32_t curr = 0;
	for (int32_t i=0; i<local.interactors.count; i+=1) {
		int32_t gen = local.interactors[i].generation;
		if (gen_is_alive(gen)) {
			if (curr == index) return gen_id_make(i, gen);
			curr++;
		}
	}
	return 0;
}

///////////////////////////////////////////

_interactor_t* _interactor_get(interactor_t interactor) {
	int32_t idx = gen_id_index(interactor);
	if (idx >= local.interactors.count) return nullptr;

	_interactor_t* actor = &local.interactors[idx];
	return gen_id_valid_match(interactor, actor->generation)
		? actor
		: nullptr;
}

///////////////////////////////////////////

void interactor_update(interactor_t interactor, vec3 capsule_start, vec3 capsule_end, pose_t motion, vec3 motion_anchor, vec3 secondary_motion, button_state_ active, button_state_ tracked) {
	_interactor_t* actor = _interactor_get(interactor);
	if (actor == nullptr) return;

	actor->tracked     = tracked;
	actor->pinch_state = active;

	if (tracked & button_state_active) {
		actor->capsule_start_world = capsule_start;
		actor->capsule_end_world   = capsule_end;
		actor->motion              = motion;
		actor->motion_anchor       = motion_anchor;
		actor->secondary_motion    = secondary_motion;
		actor->interaction_secondary_motion_total += secondary_motion;
	}

	// Don't let the hand trigger things while popping in and out of
	// tracking
	if (actor->tracked & button_state_just_active) {
		actor->motion_prev = actor->motion;
	}
}

///////////////////////////////////////////

void interactor_set_min_distance(interactor_t interactor, float min_distance) {
	_interactor_t* actor = _interactor_get(interactor);
	if (actor) actor->min_distance = min_distance;
}

///////////////////////////////////////////

float interactor_get_min_distance(interactor_t interactor) {
	_interactor_t* actor = _interactor_get(interactor);
	return actor ? actor->min_distance : 0;
}

///////////////////////////////////////////

vec3 interactor_get_capsule_start(const interactor_t interactor) {
	_interactor_t* actor = _interactor_get(interactor);
	return actor ? actor->capsule_start_world : vec3{};
}

///////////////////////////////////////////

vec3 interactor_get_capsule_end(const interactor_t interactor) {
	_interactor_t* actor = _interactor_get(interactor);
	return actor ? actor->capsule_end_world : vec3{};
}

///////////////////////////////////////////

void interactor_set_radius(interactor_t interactor, float radius) {
	_interactor_t* actor = _interactor_get(interactor);
	if (actor) actor->capsule_radius = radius;
}

///////////////////////////////////////////

float interactor_get_radius(interactor_t interactor) {
	_interactor_t* actor = _interactor_get(interactor);
	return actor ? actor->capsule_radius : 0;
}

///////////////////////////////////////////

button_state_ interactor_get_tracked(interactor_t interactor) {
	_interactor_t* actor = _interactor_get(interactor);
	return actor ? actor->tracked : button_state_inactive;
}

///////////////////////////////////////////

id_hash_t interactor_get_focused(interactor_t interactor) {
	_interactor_t* actor = _interactor_get(interactor);
	return actor ? actor->focused_prev : 0;
}

///////////////////////////////////////////

id_hash_t interactor_get_active(interactor_t interactor) {
	_interactor_t* actor = _interactor_get(interactor);
	return actor ? actor->active_prev : 0;
}

///////////////////////////////////////////

bool32_t interactor_get_focus_bounds(interactor_t interactor, pose_t* out_pose_world, bounds_t* out_bounds_local, vec3* out_at_local) {
	_interactor_t* actor = _interactor_get(interactor);
	if (actor == nullptr || actor->focused_prev == 0) {
		if (out_pose_world  ) *out_pose_world   = pose_identity;
		if (out_bounds_local) *out_bounds_local = {};
		if (out_bounds_local) *out_at_local     = {};
		return false;
	}
	if (out_pose_world  ) *out_pose_world   = actor->focus_pose_world;
	if (out_bounds_local) *out_bounds_local = actor->focus_bounds_local;
	if (out_at_local    ) *out_at_local     = actor->focus_intersection_local;
	return true;
}

///////////////////////////////////////////

pose_t interactor_get_motion(interactor_t interactor) {
	_interactor_t* actor = _interactor_get(interactor);
	return actor ? actor->motion : pose_identity;
}

///////////////////////////////////////////

bool32_t interactor_check_box(const _interactor_t* actor, bounds_t box, vec3* out_at, float* out_distance) {
	*out_distance = FLT_MAX;
	*out_at       = vec3_zero;

	if (actor == nullptr || !(actor->tracked & button_state_active))
		return false;

	if (local.show_volumes)
		render_add_mesh(local.show_volume_mesh, local.show_volume_mat, matrix_trs(box.center, quat_identity, box.dimensions));

	vec3     start_local = hierarchy_to_local_point(actor->capsule_start_world);
	vec3     end_local   = hierarchy_to_local_point(actor->capsule_end_world  );
	bool32_t result      = bounds_capsule_intersect(box, start_local, end_local, actor->capsule_radius, out_at);
	if (result) {
		*out_distance = bounds_sdf(box, *out_at);

		// Line interactors need to choose the closest element they point at,
		// since they're long lines that can intersect with a lot of different
		// elements.
		if (actor->shape_type == interactor_type_line)
			*out_distance += vec3_distance(*out_at, start_local);
	}
	// Point interactors should always be intersecting from their position,
	// even if they're just passing through. Some elements rely on this
	// position for animation and activation.
	if (actor->shape_type == interactor_type_point) {
		*out_at = hierarchy_to_local_point(actor->motion.position);
	}
	return result;
}

///////////////////////////////////////////

button_state_ interactor_set_focus(_interactor_t* interactor, id_hash_t for_el_id, bool32_t physical_focused, bool32_t soft_focused, int32_t priority, float distance, pose_t element_pose_local, bounds_t element_bounds_local, vec3 element_intersection_local) {
	if (interactor == nullptr) return button_state_inactive;
	bool was_focused = interactor->focused_prev == for_el_id;
	bool is_focused  = false;

	bool focused = physical_focused || soft_focused;
	if (focused && (distance <= interactor->focus_distance || (distance <= interactor->capsule_radius && priority >= interactor->focus_priority))) {
		if (distance >= interactor->min_distance || interactor->active_prev == for_el_id) {
			is_focused = focused;
			interactor->focused = for_el_id;
		} else {
			is_focused = false;
			interactor->focused = 0;
		}
		interactor->focus_priority     = priority;
		interactor->focus_distance     = distance;
		interactor->focus_bounds_local = element_bounds_local;
		interactor->focus_pose_world   = hierarchy_to_world_pose( element_pose_local );
		if (physical_focused)
			interactor->focus_intersection_local = element_intersection_local - element_bounds_local.center;
	}

	button_state_ result = button_state_inactive;
	if ( is_focused                ) result  = button_state_active;
	if ( is_focused && !was_focused) result |= button_state_just_active;
	if (!is_focused &&  was_focused) result |= button_state_just_inactive;
	return result;
}

///////////////////////////////////////////

button_state_ interactor_set_active(_interactor_t* interactor, id_hash_t for_el_id, bool32_t active) {
	if (interactor == nullptr) return button_state_inactive;

	bool was_active = interactor->active_prev == for_el_id;
	bool is_active  = false;

	if (active && (was_active || interactor->focused_prev == for_el_id || interactor->focused == for_el_id)) {
		is_active = active;
		interactor->active = for_el_id;

		// Mark an interaction for the keyboard focus system.
		if (!local.preserve_keyboard_stack.last()) {
			local.keyboard_focus_lost_write = for_el_id;
		}
	}

	button_state_ result = button_state_inactive;
	if ( is_active               ) result  = button_state_active;
	if ( is_active && !was_active) result |= button_state_just_active;
	if (!is_active &&  was_active) result |= button_state_just_inactive;
	return result;
}

///////////////////////////////////////////

bool32_t ui_id_focused(id_hash_t id) {
	for (int32_t i = 0; i < local.interactors.count; i++) {
		const _interactor_t* actor = &local.interactors[i];
		if (gen_is_dead(actor->generation)) continue;

		if (actor->focused_prev == id) return true;
	}
	return false;
}

///////////////////////////////////////////

button_state_ ui_id_focus_state(id_hash_t id) {
	bool was_focused = false;
	bool is_focused  = false;

	for (int32_t i = 0; i < local.interactors.count; i++) {
		const _interactor_t* actor = &local.interactors[i];
		if (gen_is_dead(actor->generation)) continue;

		if (actor->focused_prev      == id) is_focused  = true;
		if (actor->focused_prev_prev == id) was_focused = true;
	}
	return button_make_state(was_focused, is_focused);
}

///////////////////////////////////////////

bool32_t ui_id_active(id_hash_t id) {
	for (int32_t i = 0; i < local.interactors.count; i++) {
		const _interactor_t* actor = &local.interactors[i];
		if (gen_is_alive(actor->generation) && actor->active_prev == id)
			return true;
	}
	return false;
}

///////////////////////////////////////////

button_state_ ui_id_active_state(id_hash_t id) {
	bool was_active = false;
	bool is_active  = false;

	for (int32_t i = 0; i < local.interactors.count; i++) {
		const _interactor_t* actor = &local.interactors[i];
		if (gen_is_dead(actor->generation)) continue;

		if (actor->active_prev      == id) is_active  = true;
		if (actor->active_prev_prev == id) was_active = true;
	}
	return button_make_state(was_active, is_active);
}

///////////////////////////////////////////

interactor_t ui_id_active_interactor(id_hash_t id) {
	for (int32_t i = 0; i < local.interactors.count; i++) {
		const _interactor_t* actor = &local.interactors[i];
		if (gen_is_dead(actor->generation)) continue;

		if (actor->active_prev == id) return gen_id_make(i, actor->generation);
	}
	return GEN_EMPTY_ID;
}

///////////////////////////////////////////

id_hash_t ui_id_last_element() {
	return local.last_element;
}

///////////////////////////////////////////

void ui_show_volumes(bool32_t show) {
	local.show_volumes = show;
}

///////////////////////////////////////////

bool32_t interactor_is_preoccupied(const _interactor_t* interactor, id_hash_t for_el_id, interactor_event_ event_mask, bool32_t include_focused) {
	if (interactor == nullptr || gen_is_dead(interactor->generation) || interactor->tracked == button_state_inactive) return true;

	// Check if this actor can interact, or if it's already busy with something
	// else.
	if ((interactor->events & event_mask) == 0 ||
		(include_focused && interactor->focused_prev != 0 && interactor->focused_prev != for_el_id) ||
		(interactor->active_prev != 0 && interactor->active_prev != for_el_id))
		return true;

	// Check if another interactor with the same source is already busy
	if (interactor->input_source_id >= 0) {
		for (int32_t i = 0; i < local.interactors.count; i++) {
			const _interactor_t* curr = &local.interactors[i];

			if ((curr                  != interactor)                  &&
				(gen_is_alive(curr->generation))                       &&
				(curr->input_source_id == interactor->input_source_id) &&
				(curr->active_prev     != 0 || curr->active != 0))
				return true;
		}
	}
	return false;
}

///////////////////////////////////////////

button_state_ ui_last_element_active() {
	bool was_active = false;
	bool is_active  = false;
	for (int32_t i = 0; i < local.interactors.count; i++) {
		const _interactor_t* actor = &local.interactors[i];
		if (gen_is_dead(actor->generation)) continue;

		was_active = was_active || (actor->active_prev == local.last_element && local.last_element != 0);
		is_active  = is_active  || (actor->active      == local.last_element && local.last_element != 0);
	}
	return button_make_state(was_active, is_active);
}

///////////////////////////////////////////

button_state_ ui_last_element_focused() {
	bool was_focused = false;
	bool is_focused  = false;
	for (int32_t i = 0; i < local.interactors.count; i++) {
		const _interactor_t* actor = &local.interactors[i];
		if (gen_is_dead(actor->generation)) continue;

		// Because focus can change at any point during the frame, we'll check
		// against the last two frame's focus ids, which are set in stone after the
		// frame ends.
		was_focused = was_focused || (actor->focused_prev_prev == local.last_element && local.last_element != 0);
		is_focused  = is_focused  || (actor->focused_prev      == local.last_element && local.last_element != 0);
	}
	return button_make_state(was_focused, is_focused);
}

///////////////////////////////////////////

id_hash_t hash_fnv64_string_16(const char16_t* string, id_hash_t start_hash = default_hash_root) {
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
	return local.id_stack.count > 0
		? hash_string_with(string, local.id_stack.last())
		: hash_string     (string);
}

///////////////////////////////////////////

id_hash_t ui_stack_hash_16(const char16_t *string) {
	return local.id_stack.count > 0
		? hash_fnv64_string_16(string, local.id_stack.last())
		: hash_fnv64_string_16(string);
}

///////////////////////////////////////////

id_hash_t ui_stack_hashi(int32_t id) {
	return local.id_stack.count > 0
		? hash_int_with(id, local.id_stack.last())
		: hash_int     (id);
}

///////////////////////////////////////////

id_hash_t ui_push_id(const char *id) {
	id_hash_t result = ui_stack_hash(id);
	local.id_stack.add({ result });
	return result;
}

///////////////////////////////////////////

id_hash_t ui_push_id_16(const char16_t *id) {
	id_hash_t result = ui_stack_hash_16(id);
	local.id_stack.add({ result });
	return result;
}

///////////////////////////////////////////

id_hash_t ui_push_idi(int32_t id) {
	id_hash_t result = ui_stack_hashi(id);
	local.id_stack.add({ result });
	return result;
}

///////////////////////////////////////////

void ui_pop_id() {
	if (local.id_stack.count <= 1) {
		log_errf("Tried to pop too many %s! Do you have a push/pop mismatch?", "ui ids");
		return;
	}
	local.id_stack.pop();
}

///////////////////////////////////////////

void ui_push_enabled(bool32_t enabled, hierarchy_parent_ parent_behavior) {
	local.enabled_stack.add(parent_behavior == hierarchy_parent_ignore
		? enabled
		: ((bool)enabled == true) && ((bool)ui_is_enabled() == true));
}

///////////////////////////////////////////

void ui_pop_enabled() {
	if (local.enabled_stack.count <= 1) {
		log_errf("Tried to pop too many %s! Do you have a push/pop mismatch?", "'enabled'");
		return;
	}
	local.enabled_stack.pop();
}

///////////////////////////////////////////

bool32_t ui_is_enabled() {
	return local.enabled_stack.count == 0
		? true
		: local.enabled_stack.last();
}

///////////////////////////////////////////

void ui_push_preserve_keyboard(bool32_t preserve_keyboard){
	local.preserve_keyboard_stack.add(preserve_keyboard);
}

///////////////////////////////////////////

void ui_pop_preserve_keyboard(){
	if (local.preserve_keyboard_stack.count <= 1) {
		log_errf("Tried to pop too many %s! Do you have a push/pop mismatch?", "preserve keyboards");
		return;
	}
	local.preserve_keyboard_stack.pop();
}

///////////////////////////////////////////

bool32_t ui_keyboard_focus_lost(id_hash_t focused_id) {
	return
		local.keyboard_focus_lost != 0 &&
		local.keyboard_focus_lost != focused_id;
}

}