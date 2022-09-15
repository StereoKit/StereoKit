#pragma once

#include "../stereokit.h"

namespace sk {

extern mouse_t       input_mouse_data;
extern pose_t        input_head_pose_world;
extern pose_t        input_head_pose_local;
extern pose_t        input_eyes_pose_world;
extern pose_t        input_eyes_pose_local;
extern button_state_ input_eyes_track_state;
extern controller_t  input_controllers[2];
extern button_state_ input_controller_menubtn;

int32_t    input_add_pointer(input_source_ source);
pointer_t *input_get_pointer(int32_t id);

bool input_init        ();
void input_shutdown    ();
void input_update      ();
void input_update_poses(bool update_visuals);

inline button_state_ button_make_state(bool32_t was, bool32_t is) { 
	button_state_ result = is ? button_state_active : button_state_inactive;
	if (was && !is)
		result |= button_state_just_inactive;
	if (is && !was)
		result |= button_state_just_active;
	return result;
}

} // namespace sk