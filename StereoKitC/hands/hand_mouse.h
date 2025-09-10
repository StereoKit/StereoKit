#pragma once

namespace sk {

bool hand_mouse_available   ();
void hand_mouse_init        ();
void hand_mouse_shutdown    ();
void hand_mouse_update_frame(handed_ hand);
void hand_mouse_update_poses(handed_ hand);

}