#pragma once

namespace sk {

bool hand_override_available   ();
void hand_override_init        ();
void hand_override_shutdown    ();
void hand_override_update_frame(handed_ hand);

}