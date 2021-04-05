#pragma once

#include "../../stereokit.h"

namespace sk {

bool oxri_init();
void oxri_shutdown();
void oxri_update_frame();
void oxri_update_interaction_profile();

extern XrAction xrc_action_pose_grip;
extern XrAction xrc_action_pose_aim;
extern XrAction xrc_action_trigger;
extern XrAction xrc_action_grip;
extern XrPath   xrc_hand_subaction_path[2];
extern XrSpace  xrc_space_aim        [2];
extern XrSpace  xrc_space_grip          [2];

} // namespace sk
