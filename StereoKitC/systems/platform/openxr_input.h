#pragma once

#include "../../stereokit.h"

namespace sk {

bool oxri_init();
void oxri_shutdown();
void oxri_update_frame();

extern XrAction xrc_pose_action;
extern XrAction xrc_point_action;
extern XrAction xrc_select_action;
extern XrAction xrc_grip_action;
extern XrPath   xrc_hand_subaction_path[2];
extern XrSpace  xrc_point_space        [2];
extern XrSpace  xr_hand_space          [2];

} // namespace sk