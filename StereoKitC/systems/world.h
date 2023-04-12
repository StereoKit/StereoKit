#pragma once

#include "../stereokit.h"

namespace sk {

bool world_init();
void world_step();
void world_shutdown();
void world_refresh_transforms();

extern origin_mode_ world_origin_mode;
extern pose_t       world_origin_offset;

} // namespace sk