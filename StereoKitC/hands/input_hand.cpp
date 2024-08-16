/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
 */

#include "../stereokit.h"
#include "../sk_math.h"
#include "../sk_memory.h"
#include "../systems/input.h"
#include "../libraries/array.h"
#include "input_hand.h"
#include "hand_poses.h"

#include "hand_mouse.h"
#include "hand_override.h"
#include "hand_oxr_controller.h"
#include "hand_oxr_articulated.h"

#include "../platforms/platform.h"

#include <math.h>
#include <string.h>

namespace sk {

///////////////////////////////////////////

#define SK_FINGERS 5
#define SK_FINGERJOINTS 5
#define SK_SQRT2 1.41421356237f

struct hand_state_t {
	hand_t      info;
	pose_t      pose_blend[5][5];
	pose_t      pose_prev[5][5];
	pose_t      aim;
	mesh_t      active_mesh;
	vec3        pinch_pt_relative;
	bool        visible;
	bool        enabled;
	hand_sim_id_t pose_prev_id;
	float         pose_prev_time;
};

typedef struct hand_sim_t {
	hand_sim_id_t   id;
	pose_t          hand_joints[25];
	controller_key_ button1;
	controller_key_ button2;
	key_            hotkey1;
	key_            hotkey2;
} hand_sim_t;

typedef struct hand_system_t {
	hand_system_ system;
	hand_source_ source;
	float pinch_blend;
	bool (*available)();
	void (*init)();
	void (*shutdown)();
	void (*update_inactive)();
	void (*update_frame)();
	void (*update_poses)();
} hand_system_t;

hand_system_t hand_sources[] = { // In order of priority
	{ hand_system_override,
		hand_source_overridden,
		0.2f,
		hand_override_available,
		hand_override_init,
		hand_override_shutdown,
		nullptr,
		hand_override_update_frame,
		nullptr },
#if defined(SK_XR_OPENXR)
	{ hand_system_oxr_articulated,
		hand_source_articulated,
		0.3f,
		hand_oxra_available,
		hand_oxra_init,
		hand_oxra_shutdown,
		hand_oxra_update_inactive,
		hand_oxra_update_frame,
		hand_oxra_update_poses },
	{ hand_system_oxr_controllers,
		hand_source_simulated,
		0.6f,
		hand_oxrc_available,
		hand_oxrc_init,
		hand_oxrc_shutdown,
		nullptr,
		hand_oxrc_update_frame,
		hand_oxrc_update_poses },
#endif
	{ hand_system_mouse,
		hand_source_simulated,
		1,
		hand_mouse_available,
		hand_mouse_init,
		hand_mouse_shutdown,
		nullptr,
		hand_mouse_update_frame,
		hand_mouse_update_poses },
	{ hand_system_none,
		hand_source_none,
		1,
		[]() {return true;},
		[]() {},
		[]() {},
		nullptr,
		[]() {},
		[]() {} },
};
int32_t             hand_system = -1;
hand_state_t        hand_state[2] = {};
float               hand_size_update = 0;
int32_t             input_hand_pointer_id[handed_max] = {-1, -1};
array_t<hand_sim_t> hand_sim_poses   = {};
hand_sim_id_t       hand_sim_next_id = 1;

void input_gen_fallback_mesh(const hand_joint_t fingers[][5], mesh_t mesh, vert_t** ref_verts, vind_t** ref_inds);

///////////////////////////////////////////

const hand_t *input_hand(handed_ hand) {
	return &hand_state[hand].info;
}

///////////////////////////////////////////

hand_t* input_hand_ref(handed_ hand) {
	return &hand_state[hand].info;
}

///////////////////////////////////////////

hand_source_ input_hand_source(handed_) {
	return hand_sources[hand_system].source;
}

///////////////////////////////////////////

hand_system_ input_hand_get_system() {
	return hand_sources[hand_system].system;
}

///////////////////////////////////////////

void input_hand_refresh_system() {
	int available_source = _countof(hand_sources) - 1;
	for (int32_t i = 0; i < _countof(hand_sources); i++) {
		if (hand_sources[i].available()) {
			available_source = i;
			break;
		}
	}
	if (available_source != hand_system) {
		hand_system = available_source;

		// Force the hand size to recalculate next update
		hand_size_update = 0;
	}
}

///////////////////////////////////////////

void input_hand_init() {
	input_hand_pointer_id[handed_left ] = input_add_pointer(input_source_hand | input_source_hand_left  | input_source_can_press);
	input_hand_pointer_id[handed_right] = input_add_pointer(input_source_hand | input_source_hand_right | input_source_can_press);

	float blend = 1;
	for (int32_t i = 0; i < _countof(hand_sources); i++) {
		if (hand_sources[i].system == hand_system_oxr_controllers) {
			blend = hand_sources[i].pinch_blend;
			break;
		}
	}
	vec3 from_pt = vec3_lerp(input_pose_neutral[4].position, input_pose_neutral[5+4].position, blend);
	vec3 grab_pt = vec3_lerp(input_pose_pinch  [4].position, input_pose_pinch  [5+4].position, blend);
	vec3 offset  = from_pt - grab_pt;
	for (int32_t i = 0; i < 25; i++) {
		input_pose_pinch[i].position += offset;
	}

	input_hand_sim_pose_add(input_pose_neutral, controller_key_none);
	input_hand_sim_pose_add(input_pose_pinch,   controller_key_trigger, controller_key_none, key_mouse_left);
	input_hand_sim_pose_add(input_pose_point,   controller_key_grip,    controller_key_none, key_mouse_right);
	input_hand_sim_pose_add(input_pose_fist,    controller_key_trigger, controller_key_grip, key_mouse_left, key_mouse_right);

	// Initialize the hands!
	for (int32_t i = 0; i < handed_max; i++) {
		hand_state[i].visible      = true;
		hand_state[i].pose_prev_id = -1;
		memcpy(hand_state[i].pose_prev,  input_pose_neutral, sizeof(pose_t) * SK_FINGERS * SK_FINGERJOINTS);
		memcpy(hand_state[i].pose_blend, input_pose_neutral, sizeof(pose_t) * SK_FINGERS * SK_FINGERJOINTS);
		hand_state[i].info.palm.orientation = quat_identity;
		hand_state[i].info.handedness       = (handed_)i;

		// Set up initial default hand pose, so we don't get any accidental pinch/grips on start
		hand_t &hand = hand_state[i].info;
		for (int32_t f = 0; f < 5; f++) {
		for (int32_t j = 0; j < 5; j++) {
			vec3 pos = input_pose_neutral[f*5+j].position;
			quat rot = input_pose_neutral[f*5+j].orientation;
			if (i == handed_right) {
				// mirror along x axis, our pose data is for left hand
				pos.x = -pos.x;
				rot.y = -rot.y;
				rot.z = -rot.z;
			}
			hand.fingers[f][j].position    = pos;
			hand.fingers[f][j].orientation = rot;
			hand.fingers[f][j].radius      = hand_joint_size[f*5+j];
		} }
	}

	for (int32_t i = 0; i < _countof(hand_sources); i++) {
		hand_sources[i].init();
	}

	input_hand_refresh_system();
}

///////////////////////////////////////////

void input_hand_shutdown() {
	for (int32_t i = 0; i < _countof(hand_sources); i++) {
		hand_sources[i].shutdown();
	}
	hand_sim_poses.free();
}

///////////////////////////////////////////

void input_hand_update() {
	hand_sources[hand_system].update_frame();

	// Update the hand size every second
	hand_size_update -= time_stepf_unscaled();
	if (hand_size_update <= 0) {
		hand_size_update = 1;

		for (int32_t h = 0; h < handed_max; h++) {
			if (!(hand_state[h].info.tracked_state & button_state_active))
				continue;

			hand_state[h].info.size = 0;
			for (int32_t j = 0; j < hand_joint_tip-1; j++) {
				hand_state[h].info.size += vec3_magnitude(
					hand_state[h].info.fingers[hand_finger_middle][j  ].position -
					hand_state[h].info.fingers[hand_finger_middle][j+1].position);
			}
			hand_state[h].info.size += hand_state[h].info.fingers[hand_finger_middle][hand_joint_root].radius;
			hand_state[h].info.size += hand_state[h].info.fingers[hand_finger_middle][hand_joint_tip].radius;
		}
	}

	// Update hand states
	for (int32_t i = 0; i < handed_max; i++) {
		input_hand_state_update((handed_)i);
	}

	for (int32_t i = 0; i < _countof(hand_sources); i++) {
		if (hand_system != i && hand_sources[i].update_inactive != nullptr)
			hand_sources[i].update_inactive();
	}
}

///////////////////////////////////////////

void input_hand_update_poses() {
	if (hand_system >= 0 && hand_sources[hand_system].update_poses)
		hand_sources[hand_system].update_poses();
}

///////////////////////////////////////////

void input_hand_sim_trigger(button_state_ prev_state, hand_joint_t a, hand_joint_t b, float activation_dist, float deactivation_dist, float max_dist, button_state_ *out_state, float *out_activation) {
	button_state_ result        = button_state_inactive;
	bool          was_triggered = prev_state & button_state_active;
	float         threshold     = was_triggered ? deactivation_dist : activation_dist;

	float offset = a.radius + b.radius;
	float dist   = vec3_distance(a.position, b.position) - offset;

	float activation   = fminf(1, fmaxf(0, 1 - ((dist - threshold) / (max_dist - threshold))));
	bool  is_triggered = dist <= threshold;

	if (was_triggered != is_triggered) result |= is_triggered ? button_state_just_active : button_state_just_inactive;
	if (is_triggered) result |= button_state_active;

	*out_state      = result;
	*out_activation = activation;
}

///////////////////////////////////////////

void input_hand_state_update(handed_ handedness) {
	hand_t &hand = hand_state[handedness].info;

	hand.pinch_pt = vec3_lerp(
		hand.fingers[0][4].position,
		hand.fingers[1][4].position,
		hand_sources[hand_system].pinch_blend);

	if (hand_sources[hand_system].system == hand_system_oxr_articulated) {
		// Preserve the pinch point relative to the root of the index finger
		// while the pinch is active. This helps prevent traveling of the pinch
		// point during release on real articulated hands.
		if ((hand.pinch_state & button_state_just_active) > 0) {
			matrix to_relative = matrix_invert(matrix_trs(hand.fingers[1][0].position, hand.fingers[1][0].orientation, vec3_one));
			hand_state[handedness].pinch_pt_relative = matrix_transform_pt(to_relative, hand.pinch_pt);
		} else if ((hand.pinch_state & button_state_active) > 0 || (hand.pinch_state & button_state_just_inactive) > 0) {
			matrix from_relative = matrix_trs(hand.fingers[1][0].position, hand.fingers[1][0].orientation, vec3_one);
			hand.pinch_pt = matrix_transform_pt(from_relative, hand_state[handedness].pinch_pt_relative);
		}
	}

	pointer_t* pointer = input_get_pointer(input_hand_pointer_id[handedness]);
	pointer->state = button_make_state(
		(pointer->state & button_state_active) != 0,
		((pointer->tracked & button_state_active) != 0) && ((hand.pinch_state & button_state_active) != 0));
}

///////////////////////////////////////////

bool input_hand_get_visible(handed_ hand) {
	return hand_state[hand].visible;
}

///////////////////////////////////////////

hand_joint_t *input_hand_get_pose_buffer(handed_ hand) {
	return &hand_state[hand].info.fingers[0][0];
}

///////////////////////////////////////////

void input_hand_sim_poses(handed_ handedness, bool mouse_adjustments, vec3 hand_pos, quat orientation) {
	hand_t &hand = hand_state[handedness].info;

	// only sim it if it's tracked
	if (!(hand.tracked_state & button_state_active))
		return;

	// Calculate the orientation of the palm joint, we need this right away
	// because our simulated hand poses are relative to this joint. OpenXR's
	// facing direction for the palm joint seems... somewhat less practical, so
	// here we are alsso arranging them so that "forward" faces out from the
	// palm.
	quat palm_rot = quat_from_angles(
		0,
		handedness == handed_right ? 90.f : -90.f,
		handedness == handed_right ? -90.f : 90.f) * orientation;

	// For mice based hands, we change the hand's location to center the
	// pointer finger on the mouse
	vec3 finger_off = mouse_adjustments
		? hand_state[handedness].pose_blend[1][4].position
		: vec3_zero;

	// Update the position and orientation of each joint to be at its proper
	// location in world space
	for (int32_t f = 0; f < 5; f++) {
		const pose_t *finger = &hand_state[handedness].pose_blend[f][0];
	for (int32_t j = 0; j < 5; j++) {
		vec3 pos = finger[j].position - finger_off;
		quat rot = finger[j].orientation;
		if (handedness == handed_left) {
			// mirror along x axis, our pose data is for left hand
			pos.x = -pos.x;
			rot.y = -rot.y;
			rot.z = -rot.z;
		}
		hand.fingers[f][j].position    = palm_rot * pos + hand_pos;
		hand.fingers[f][j].orientation = rot * palm_rot;
		hand.fingers[f][j].radius      = hand_joint_size[f*5+j];
	} }

	// Update some of the higher level hand poses

	// OpenXR spec defines "The palm joint is located at the center of the
	// middle finger's metacarpal bone", so we'll use that for the position.
	hand.palm.position =
		(hand.fingers[2][0].position +
		 hand.fingers[2][1].position) * 0.5f;
	hand.palm.orientation  = palm_rot;
	hand.wrist.orientation = hand.fingers[2][0].orientation;
	hand.wrist.position    = (hand.fingers[1][0].position + hand.fingers[4][0].position) / 2 + (hand.wrist.orientation*vec3_forward*-0.03f);
}

///////////////////////////////////////////

void input_hand_sim(handed_ handedness, bool center_on_finger, vec3 hand_pos, quat orientation, bool tracked) {
	hand_t &hand = hand_state[handedness].info;

	// Update tracking state
	bool was_tracked = hand.tracked_state & button_state_active;
	hand.tracked_state = button_make_state(was_tracked, tracked);

	// only sim it if it's tracked
	if (!tracked)
		return;

	// Switch pose based on what buttons are pressed
	const pose_t* dest_pose = nullptr;
	hand_sim_id_t pose_id         = -1;
	int32_t       pose_idx        = -1;
	float         pose_blend_curr = 0;
	
	for (int32_t i = 0; i < hand_sim_poses.count; i++) {
		float amt1 = 0, amt2 = 0;
		hand_sim_t *p = &hand_sim_poses[i];
		// If this pose's input conditions are met
		if (((p->button1 != controller_key_none && input_controller_key(handedness, p->button1, &amt1)) &&
			 (p->button2 == controller_key_none || input_controller_key(handedness, p->button2, &amt2))) ||
			((p->hotkey1 != key_none && (input_key(p->hotkey1) & button_state_active) > 0) &&
			 (p->hotkey2 == key_none || (input_key(p->hotkey2) & button_state_active) > 0))) {
			pose_id         = p->id;
			pose_idx        = i;
			pose_blend_curr = (p->button2 != controller_key_none || p->hotkey2 != key_none)
				? fminf(amt1, amt2)
				: amt1;
		}

		// If this is a neutral pose
		if (pose_idx == -1 &&
			p->button1 == controller_key_none && p->button2 == controller_key_none &&
			p->hotkey1 == key_none            && p->hotkey2 == key_none) {
			pose_id  = p->id;
			pose_idx = i;
		}
	}
	if (hand_state[handedness].pose_prev_id != pose_id) {
		hand_state[handedness].pose_prev_id   = pose_id;
		hand_state[handedness].pose_prev_time = time_totalf();
		memcpy(hand_state[handedness].pose_prev, hand_state[handedness].pose_blend, sizeof(pose_t) * 25);
	}
	if (pose_idx != -1) {
		dest_pose = hand_sim_poses[pose_idx].hand_joints;
	}

	// Blend our active pose with our desired pose, for smooth transitions
	// between poses
	if (dest_pose != nullptr) {
		if (pose_blend_curr == 0) {
			pose_blend_curr = 1 - fminf(1,(time_totalf() - hand_state[handedness].pose_prev_time) / 0.1f);
			pose_blend_curr = 1 - (pose_blend_curr * pose_blend_curr);
		}
		for (int32_t f = 0; f < 5; f++) {
		for (int32_t j = 0; j < 5; j++) {
			pose_t *p     = &hand_state[handedness].pose_blend[f][j];
			int32_t joint = f * 5 + j;
			p->position    = vec3_lerp (hand_state[handedness].pose_prev[f][j].position,    dest_pose[joint].position,    pose_blend_curr);
			p->orientation = quat_slerp(hand_state[handedness].pose_prev[f][j].orientation, dest_pose[joint].orientation, pose_blend_curr);
		} }
	}

	// Update all the hand joints now that we have a pose to work from.
	input_hand_sim_poses(handedness, center_on_finger, hand_pos, orientation);
}

///////////////////////////////////////////

hand_sim_id_t input_hand_sim_pose_add(const pose_t* in_arr_palm_relative_hand_joints_25, controller_key_ button1, controller_key_ and_button2, key_ or_hotkey1, key_ and_hotkey2) {
	hand_sim_id_t result = hand_sim_next_id;
	hand_sim_next_id += 1;

	hand_sim_t hand_sim = {};
	hand_sim.id              = result;
	hand_sim.button1         = button1;
	hand_sim.button2         = and_button2;
	hand_sim.hotkey1         = or_hotkey1;
	hand_sim.hotkey2         = and_hotkey2;
	memcpy(hand_sim.hand_joints, in_arr_palm_relative_hand_joints_25, sizeof(pose_t) * 25);
	hand_sim_poses.add(hand_sim);

	return result;
}

///////////////////////////////////////////

void input_hand_sim_pose_remove(hand_sim_id_t id) {
	for (int32_t i = 0; i < hand_sim_poses.count; i++) {
		if (hand_sim_poses[i].id == id) {
			hand_sim_poses.remove(i);
			return;
		}
	}
}

///////////////////////////////////////////

void input_hand_sim_pose_clear() {
	hand_sim_poses.clear();
}

///////////////////////////////////////////

const vec3 sincos[] = { 
	{cosf(162*deg2rad), sinf(162*deg2rad), 0},
	{cosf(90 *deg2rad), sinf(90 *deg2rad), 0},
	{cosf(18 *deg2rad), sinf(18 *deg2rad), 0},
	{cosf(18 *deg2rad), sinf(18 *deg2rad), 0},
	{cosf(306*deg2rad), sinf(306*deg2rad), 0},
	{cosf(234*deg2rad), sinf(234*deg2rad), 0},
	{cosf(162*deg2rad), sinf(162*deg2rad), 0},};

const vec3 sincos_norm[] = { 
	{cosf(126*deg2rad), sinf(126*deg2rad), 0},
	{cosf(90 *deg2rad), sinf(90 *deg2rad), 0},
	{cosf(54 *deg2rad), sinf(54 *deg2rad), 0},
	{cosf(18 *deg2rad), sinf(18 *deg2rad), 0},
	{cosf(306*deg2rad), sinf(306*deg2rad), 0},
	{cosf(234*deg2rad), sinf(234*deg2rad), 0},
	{cosf(162*deg2rad), sinf(162*deg2rad), 0},};

const float texcoord_v[SK_FINGERJOINTS + 1] = { 1, 1-0.44f, 1-0.69f, 1-0.85f, 1-0.96f, 1-0.99f };

void input_gen_fallback_mesh(const hand_joint_t fingers[][5], mesh_t mesh, vert_t **ref_verts, vind_t **ref_inds) {
	assert(mesh != nullptr);

	const int32_t  ring_count  = _countof(sincos);
	const int32_t  slice_count = SK_FINGERJOINTS + 1;
	const uint32_t vert_count  = (_countof(sincos) * slice_count + 1) * SK_FINGERS; // verts: per joint, per finger

	vert_t *verts = *ref_verts;
	vind_t *inds  = *ref_inds;

	// if this mesh hasn't been initialized yet
	if (*ref_verts == nullptr) {
		const uint32_t ind_count = (3 * 5 * 2 * (slice_count - 1) + (8 * 3)) * (SK_FINGERS); // inds: per face, per connecting faces, per joint section, per finger, plus 2 caps
		*ref_verts = sk_malloc_t(vert_t, vert_count);
		*ref_inds  = sk_malloc_t(vind_t, ind_count );
		verts      = *ref_verts;
		inds       = *ref_inds;

		int32_t ind = 0;
		for (vind_t f = 0; f < SK_FINGERS; f++) {
			vind_t start_vert =  f    * (ring_count * slice_count + 1);
			vind_t end_vert   = (f+1) * (ring_count * slice_count + 1) - (ring_count + 1);

			// start cap
			inds[ind++] = start_vert+2;
			inds[ind++] = start_vert+1;
			inds[ind++] = start_vert+0;

			inds[ind++] = start_vert+4;
			inds[ind++] = start_vert+3;
			inds[ind++] = start_vert+6;

			inds[ind++] = start_vert+5;
			inds[ind++] = start_vert+4;
			inds[ind++] = start_vert+6;
		
			// tube faces
			for (vind_t j = 0; j < slice_count-1; j++) {
			for (vind_t c = 0; c < ring_count-1; c++) {
				if (c == 2) c++;
				vind_t curr1 = start_vert +  j    * ring_count + c;
				vind_t next1 = start_vert + (j+1) * ring_count + c;
				vind_t curr2 = start_vert +  j    * ring_count + (c+1);
				vind_t next2 = start_vert + (j+1) * ring_count + (c+1);
				inds[ind++] = next2;
				inds[ind++] = next1;
				inds[ind++] = curr1;

				inds[ind++] = curr2;
				inds[ind++] = next2;
				inds[ind++] = curr1;
			} }

			// end cap
			inds[ind++] = end_vert+0;
			inds[ind++] = end_vert+1;
			inds[ind++] = end_vert+7;

			inds[ind++] = end_vert+1;
			inds[ind++] = end_vert+2;
			inds[ind++] = end_vert+7;

			inds[ind++] = end_vert+3;
			inds[ind++] = end_vert+4;
			inds[ind++] = end_vert+7;

			inds[ind++] = end_vert+4;
			inds[ind++] = end_vert+5;
			inds[ind++] = end_vert+7;

			inds[ind++] = end_vert+5;
			inds[ind++] = end_vert+6;
			inds[ind++] = end_vert+7;
		}

		// Generate uvs and colors for the mesh
		int32_t v = 0;
		for (int32_t f = 0; f < SK_FINGERS; f++) {
			float x = ((float)f / SK_FINGERS) + (0.5f/SK_FINGERS);
		for (int32_t j = 0; j < slice_count; j++) {
			float y = texcoord_v[f==0 ? maxi(0,j-1) : j];
			
			verts[v  ].uv  = { x,y };
			verts[v++].col = { 255,255,255,255 };
			verts[v  ].uv  = { x,y };
			verts[v++].col = { 255,255,255,255 };
			verts[v  ].uv  = { x,y };
			verts[v++].col = { 255,255,255,255 };
			verts[v  ].uv  = { x,y };
			verts[v++].col = { 200,200,200,255 };
			verts[v  ].uv  = { x,y };
			verts[v++].col = { 200,200,200,255 };
			verts[v  ].uv  = { x,y };
			verts[v++].col = { 200,200,200,255 };
			verts[v  ].uv  = { x,y };
			verts[v++].col = { 200,200,200,255 };
		} 
		verts[v  ].uv  = { x,0 };
		verts[v++].col = { 255,255,255,255 };
		}

		mesh_set_inds(mesh, inds, ind_count);
	}

	int32_t v = 0;
	for (int32_t f = 0; f < SK_FINGERS; f++) {
		const hand_joint_t pose_last = fingers[f][SK_FINGERJOINTS-1];
		const vec3         tip_fwd   = pose_last.orientation * vec3_forward;
		const vec3         tip_up    = pose_last.orientation * vec3_up;
		for (int32_t j = 0; j < SK_FINGERJOINTS; j++) {
			const hand_joint_t pose_prev   = fingers[f][maxi(0,j-1)];
			const hand_joint_t pose        = fingers[f][j];
			const quat         orientation = quat_slerp(pose_prev.orientation, pose.orientation, 0.5f);

			// Find a scaling offset to preserve the volume of the joint while
			// bending
			float skew_scale = 1.0f;
			if (j < SK_FINGERJOINTS-1 && j > 0 && (f != 0 || j > 1)) {
				vec3  fwd_a = pose_prev.orientation * vec3_forward;
				vec3  fwd_b = pose.orientation      * vec3_forward;
				float angle = fminf(MATH_PI/2.5f, acosf(fminf(1,vec3_dot(fwd_a, fwd_b))) / 2.0f);
				skew_scale = 1.0f / cosf(angle);
			}

			// Make local right and up axis vectors
			vec3 right = orientation * vec3_right;
			vec3 up    = orientation * vec3{ 0, skew_scale, 0 };

			// Find the scale for this joint
			float scale = pose.radius;
			if (f == 0 && j < 2) scale *= 0.5f; // thumb is too fat at the bottom

			// Use the local axis to create a ring of verts
			for (int32_t i = 0; i < ring_count; i++) {
				verts[v].norm = (up*sincos_norm[i].y + right*sincos_norm[i].x) * SK_SQRT2;
				verts[v].pos  = pose.position + (up*sincos[i].y + right*sincos[i].x)*scale;
				v++;
			}

			// Last ring to blunt the fingertip
			if (j == SK_FINGERJOINTS - 1) {
				scale = scale * 0.75f;
				for (int32_t i = 0; i < ring_count; i++) {
					vec3 at = pose.position + tip_fwd * pose.radius * 0.65f;
					verts[v].norm = (up*sincos_norm[i].y + right*sincos_norm[i].x) * SK_SQRT2;
					verts[v].pos  = at + (up*sincos[i].y + right*sincos[i].x)*scale + tip_up * pose.radius*0.25f;
					v++;
				}
			}
		}
		verts[v].norm = tip_fwd;
		verts[v].pos  = pose_last.position + verts[v].norm * pose_last.radius + tip_up * pose_last.radius * 0.9f;
		v++;
	}

	// And update the mesh vertices!
	mesh_set_verts(mesh, verts, vert_count);
}

///////////////////////////////////////////

void input_hand_visible(handed_ hand, bool32_t visible) {
	if (hand == handed_max) {
		input_hand_visible(handed_left,  visible);
		input_hand_visible(handed_right, visible);
		return;
	}

	hand_state[hand].visible = visible;
}

} // namespace sk