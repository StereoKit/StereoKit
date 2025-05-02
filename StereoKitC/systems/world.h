#pragma once

#include "../stereokit.h"

namespace sk {

bool world_init();
void world_step();
void world_shutdown();

extern origin_mode_ world_origin_mode;
extern pose_t       world_origin_offset;

} // namespace sk