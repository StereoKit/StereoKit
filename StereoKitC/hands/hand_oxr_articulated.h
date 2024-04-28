#pragma once

namespace sk {

#include "input_hand.h"

bool hand_oxra_available      ();
void hand_oxra_init           ();
void hand_oxra_shutdown       ();
void hand_oxra_update_inactive();
void hand_oxra_update_frame   ();
void hand_oxra_update_poses   ();

void hand_oxra_update_system_mesh(handed_ handed, hand_mesh_t *hand_mesh);

}