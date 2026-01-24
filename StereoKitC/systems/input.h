/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2025 Nick Klingensmith
 * Copyright (c) 2024-2025 Qualcomm Technologies, Inc.
 */

#pragma once

#include "../stereokit.h"

namespace sk {

typedef enum input_pose_ {
	input_pose_eyes,
	input_pose_l_grip,
	input_pose_l_palm,
	input_pose_l_aim,
	input_pose_r_grip,
	input_pose_r_palm,
	input_pose_r_aim,

	input_pose_max
} input_pose_;

typedef enum input_float_ {
	input_float_l_trigger,
	input_float_l_grip,
	input_float_r_trigger,
	input_float_r_grip,

	input_float_max
} input_float_;

typedef enum input_button_ {
	input_button_l_aim_ready,
	input_button_l_stick,
	input_button_l_x1,
	input_button_l_x2,
	input_button_l_menu,
	input_button_r_aim_ready,
	input_button_r_stick,
	input_button_r_x1,
	input_button_r_x2,
	input_button_r_menu,

	input_button_bool_max
} input_button_;


typedef enum input_xy_ {
	input_xy_l_stick,
	input_xy_r_stick,

	input_xy_max
} xra_xy_;

bool          input_init               ();
void          input_shutdown           ();
void          input_step               ();
void          input_step_late          ();

void          input_update_poses       ();
void          input_mouse_override_pos (vec2 override_pos);
controller_t* input_controller_ref     (handed_ handed);
void          input_controller_menu_set(button_state_ state);
bool          input_controller_key     (handed_ hand, controller_key_ key, float* out_amount);
bool          input_controller_is_hand (handed_ hand);
void          input_controller_set_hand(handed_ hand, bool is_hand);
void          input_eyes_tracked_set   (button_state_ state);

void          input_pose_inject        (input_pose_   pose_type, pose_t pose, track_state_ pos_tracked, track_state_ rot_tracked);
pose_t        input_pose_get_local     (input_pose_   pose_type);
pose_t        input_pose_get_world     (input_pose_   pose_type);
void          input_pose_get_state     (input_pose_   pose_type, track_state_* out_pos_tracked, track_state_* out_rot_tracked);
void          input_float_inject       (input_float_  float_type, float value);
float         input_float_get          (input_float_  float_type);
void          input_button_inject      (input_button_ button_type, bool value);
button_state_ input_button_get         (input_button_ button_type);
void          input_xy_inject          (input_xy_     xy_type, vec2 value);
vec2          input_xy_get             (input_xy_     xy_type);
void          input_scroll_inject      (float scroll_delta);
void          input_reset              ();

void          input_set_palm_offset    (handed_ hand, pose_t offset);

void body_make_shoulders(vec3 *out_left, vec3 *out_right);

inline button_state_ button_make_state(bool32_t was, bool32_t is) {
	button_state_ result = is ? button_state_active : button_state_inactive;
	if (was && !is)
		result |= button_state_just_inactive;
	if (is && !was)
		result |= button_state_just_active;
	return result;
}

extern pose_t input_head_pose_local;


} // namespace sk