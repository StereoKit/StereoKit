#pragma once
#include "../stereokit.h"

namespace sk {

bool hand_oxrc_available   ();
void hand_oxrc_init        ();
void hand_oxrc_shutdown    ();
void hand_oxrc_update_frame();
void hand_oxrc_update_poses();

}