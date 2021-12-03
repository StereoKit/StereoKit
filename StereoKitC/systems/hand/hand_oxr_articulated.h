#pragma once

namespace sk {

bool hand_oxra_available      ();
void hand_oxra_init           ();
void hand_oxra_shutdown       ();
void hand_oxra_update_inactive();
void hand_oxra_update_frame   ();
void hand_oxra_update_poses   (bool update_visuals);

}