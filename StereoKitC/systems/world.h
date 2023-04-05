#pragma once

#include "../stereokit.h"

namespace sk {

bool world_init();
void world_step();
void world_shutdown();
void world_refresh_transforms();

} // namespace sk