#pragma once

namespace sk {

bool hand_mouse_available   ();
void hand_mouse_init        ();
void hand_mouse_shutdown    ();
void hand_mouse_update_frame();
void hand_mouse_update_poses(bool update_visuals);

}