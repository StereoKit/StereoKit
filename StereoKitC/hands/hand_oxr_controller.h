#pragma once
#include "../stereokit.h"

namespace sk {

bool hand_oxrc_available   ();
void hand_oxrc_init        ();
void hand_oxrc_shutdown    ();
void hand_oxrc_update_frame();
void hand_oxrc_update_poses(bool update_visuals);

extern quat xrc_offset_rot[2];
extern vec3 xrc_offset_pos[2];

}