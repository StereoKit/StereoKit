/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2025 Nick Klingensmith
 * Copyright (c) 2024-2025 Qualcomm Technologies, Inc.
 */

#include "../platforms/platform.h"
#if defined(SK_XR_OPENXR)

#include "../stereokit.h"
#include "../_stereokit.h"
#include "../xr_backends/openxr.h"
#include "../xr_backends/openxr_input.h"
#include "../systems/input.h"
#include "../systems/render.h"
#include "input_hand.h"
#include "hand_poses.h"

#include <string.h>
#include <stdlib.h>

namespace sk {

///////////////////////////////////////////

bool hand_oxrc_available() {
	return sk_active_display_mode() == display_mode_mixedreality
		&& xr_session != XR_NULL_HANDLE;
}

///////////////////////////////////////////

void hand_oxrc_init() {
}

///////////////////////////////////////////

void hand_oxrc_shutdown() {
}

///////////////////////////////////////////

void hand_oxrc_update_pose(bool animate) {
	for (uint32_t hand_id = 0; hand_id < handed_max; hand_id++) {
		const controller_t *controller = input_controller ((handed_)hand_id);
		hand_t             *hand       = input_hand_ref   ((handed_)hand_id);
		// Update the hand point pose
		if ((controller->tracked & button_state_active) > 0) {
			hand->aim = controller->aim;
		}

		// Simulate the hand based on the state of the controller
		bool   tracked   = controller->tracked & button_state_active;
		pose_t hand_pose = tracked
			? controller->palm
			: hand->palm;

		// Hand interaction profiles get a real pinch point, so skip the pinch
		// stabilization offset that would otherwise shift the whole skeleton.
		bool stabilize_pinch = !input_controller_is_hand((handed_)hand_id);
		if (animate) input_hand_sim      ((handed_)hand_id, false, hand_pose.position, hand_pose.orientation, tracked, stabilize_pinch);
		else         input_hand_sim_poses((handed_)hand_id, false, hand_pose.position, hand_pose.orientation);

		// Bend the index finger so its tip matches the poke pose. The inner
		// joints are pure X-hinges, leaving an analytic two-bone solve for 'mid'.
		input_pose_ poke_pose = hand_id == handed_left ? input_pose_l_poke : input_pose_r_poke;
		if (input_controller_is_hand((handed_)hand_id) && (input_pose_state(poke_pose) & pose_state_pos_known)) {
			hand_joint_t* idx  = hand->fingers[finger_id_index];
			pose_t        poke = input_pose(poke_pose);
			vec3          base = idx[joint_id_knuckle_major].position;

			// Bone lengths come from the neutral pose; the live blended finger
			// has shortened segments that make the target unreachable mid-curl.
			const pose_t* np = &input_pose_neutral[finger_id_index * 5];
			float l0 = vec3_distance(np[joint_id_knuckle_major].position, np[joint_id_knuckle_mid  ].position);
			float l1 = vec3_distance(np[joint_id_knuckle_mid  ].position, np[joint_id_knuckle_minor].position);
			float l2 = vec3_distance(np[joint_id_knuckle_minor].position, np[joint_id_tip          ].position);

			// The tip's forward fixes the last bone, so 'minor' sits one bone
			// back from it, along the tip's forward.
			vec3 tip_fwd = poke.orientation * vec3_forward;
			vec3 minor   = poke.position - tip_fwd * l2;

			// Bend-plane frame from the base knuckle: local X is the hinge axis,
			// signed so cross(hinge, bone) points dorsal, for either hand.
			quat base_q = idx[joint_id_knuckle_major].orientation;
			vec3 dorsal = base_q * vec3_up;
			vec3 bend_n = base_q * vec3_right;
			if (vec3_dot(vec3_cross(bend_n, base_q * vec3_forward), dorsal) < 0) bend_n = bend_n * -1.0f;

			// Analytic two-bone IK for 'mid', bending in the plane (around bend_n).
			vec3  to_minor = minor - base;
			float d        = fmaxf(vec3_magnitude(to_minor), 0.0001f);
			vec3  dir      = to_minor / d;

			// A real joint can't fold flat. Cap how tight the chain can curl, so
			// an over-curled fist can't drive the elbow onto the base->minor line.
			float d_min   = sqrtf(l0*l0 + l1*l1 - l0*l1); // ~120 deg of bend
			float d_solve = fminf(fmaxf(d, d_min), l0 + l1);
			float a       = (d_solve*d_solve + l0*l0 - l1*l1) / (2*d_solve);
			float h       = sqrtf(fmaxf(0, l0*l0 - a*a));
			// 'mid' is always on the dorsal side of the base->minor line;
			// cross(bend_n, dir) gives that side, stable through the full curl.
			vec3  bend_dir = vec3_normalize(vec3_cross(bend_n, dir));
			vec3  mid      = base + dir*a + bend_dir*h;

			// Orientations: forward down each bone with X locked to bend_n, so
			// the joints can't roll. The tip is set exactly to the poke pose.
			idx[joint_id_knuckle_major].orientation = quat_lookat_up(base,  mid,           vec3_cross(bend_n, vec3_normalize(mid   - base )));
			idx[joint_id_knuckle_mid  ].orientation = quat_lookat_up(mid,   minor,         vec3_cross(bend_n, vec3_normalize(minor - mid  )));
			idx[joint_id_knuckle_minor].orientation = quat_lookat_up(minor, poke.position, vec3_cross(bend_n, tip_fwd));
			idx[joint_id_knuckle_mid  ].position    = mid;
			idx[joint_id_knuckle_minor].position    = minor;
			idx[joint_id_tip          ].position    = poke.position;
			idx[joint_id_tip          ].orientation = poke.orientation;
		}
	}
}

///////////////////////////////////////////

void hand_oxrc_update_frame() {
	hand_oxrc_update_pose(true);

	// Now we'll get the current states of our actions, and store them for later use
	for (uint32_t hand_id = 0; hand_id < handed_max; hand_id++) {
		const controller_t *controller = input_controller((handed_)hand_id);
		hand_t             *hand       = input_hand_ref  ((handed_)hand_id);
		bool                tracked    = controller->tracked & button_state_active;

		hand->pinch_state      = button_make_state((hand->pinch_state & button_state_active) > 0, controller->trigger >= 0.5f);
		hand->grip_state       = button_make_state((hand->grip_state  & button_state_active) > 0, controller->grip    >= 0.5f);
		hand->pinch_activation = fminf(1,controller->trigger/0.5f);
		hand->grip_activation  = fminf(1,controller->grip   /0.5f);
		hand->aim_ready        = controller->tracked;
	}
}

///////////////////////////////////////////

void hand_oxrc_update_poses() {
	hand_oxrc_update_pose(false);
}

}

#endif