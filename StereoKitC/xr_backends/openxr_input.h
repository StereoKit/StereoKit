#pragma once

namespace sk {

typedef enum xr_action_ {
	xr_action_pose_eyes,
	xr_action_pose_grip,
	xr_action_pose_palm,
	xr_action_pose_aim,
	xr_action_float_trigger,
	xr_action_float_grip,
	xr_action_xy_stick,
	xr_action_bool_aim_ready,
	xr_action_bool_stick,
	xr_action_bool_x1,
	xr_action_bool_x2,
	xr_action_bool_menu,
	xr_action_max,
} xr_action_;

typedef struct xr_interaction_bind_t {
	xr_action_ action;
	const char* paths[2];
} xr_interaction_bind_t;

typedef struct xr_interaction_profile_t {
	const char* name;
	bool        use_shorthand_names;
	bool        is_hand;
	xr_interaction_bind_t binding[xr_action_max];
	int32_t     binding_ct;
	pose_t      palm_offset[2];
} xr_interaction_profile_t;

void oxri_register_profile(xr_interaction_profile_t profile);

void oxri_register    ();
void oxri_update_poses();
void oxri_update_interaction_profile();

extern bool xrc_aim_ready[2];

} // namespace sk
