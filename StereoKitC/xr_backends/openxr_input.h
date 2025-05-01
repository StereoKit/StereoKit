#pragma once

namespace sk {

typedef enum xr_top_level_ {
	xr_top_level_none,
	xr_top_level_head,
	xr_top_level_hand_left,
	xr_top_level_hand_right,
	xr_top_level_gamepad,
	xr_top_level_eyes,
	xr_top_level_max
} xr_top_level_;

typedef enum xra_type_ {
	xra_type_pose,
	xra_type_float,
	xra_type_bool,
	xra_type_xy,

	xra_type_max
} xr_action_type_;

typedef enum xraction_ {
	xr_action_pose_eyes,

	xr_action_r_pose_grip,
	xr_action_r_pose_palm,
	xr_action_r_pose_aim,
	xr_action_r_float_trigger,
	xr_action_r_float_grip,
	xr_action_r_xy_stick,
	xr_action_r_bool_aim_ready,
	xr_action_r_bool_stick,
	xr_action_r_bool_x1,
	xr_action_r_bool_x2,
	xr_action_r_bool_menu,

	xr_action_l_pose_grip,
	xr_action_l_pose_palm,
	xr_action_l_pose_aim,
	xr_action_l_float_trigger,
	xr_action_l_float_grip,
	xr_action_l_xy_stick,
	xr_action_l_bool_aim_ready,
	xr_action_l_bool_stick,
	xr_action_l_bool_x1,
	xr_action_l_bool_x2,
	xr_action_l_bool_menu,

	xr_action_max,
} xr_action_;

typedef struct xr_interaction_bind_t {
	uint16_t    xra_type;
	uint16_t    xra_type_val;
	const char* path;
} xr_interaction_bind_t;

typedef struct xr_interaction_profile_t {
	const char*           name;
	const char*           top_level_path;
	xr_interaction_bind_t binding[xr_action_max];
	int32_t               binding_ct;
	pose_t                palm_offset;
	bool                  is_hand;
} xr_interaction_profile_t;

void oxri_register_profile(xr_interaction_profile_t profile);

void oxri_register    ();
void oxri_update_poses();

} // namespace sk
