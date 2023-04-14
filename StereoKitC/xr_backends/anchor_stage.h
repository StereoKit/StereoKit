#pragma once

#include "../stereokit.h"
#include "../libraries/array.h"

#include <stdint.h>

namespace sk {

bool32_t anchor_stage_init        ();
void     anchor_stage_shutdown    ();
anchor_t anchor_stage_create      (pose_t pose, const char* name_utf8);
void     anchor_stage_clear_stored();
bool32_t anchor_stage_persist     (anchor_t anchor, bool32_t persist);

} // namespace sk