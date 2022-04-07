#pragma once

#include "../../stereokit.h"

namespace sk {

bool oxri_init        ();
void oxri_shutdown    ();
void oxri_update_frame();
void oxri_update_poses();
void oxri_update_interaction_profile();

extern XrSpace    xr_gaze_space;

} // namespace sk
