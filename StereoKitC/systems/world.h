#pragma once

#include "../stereokit.h"

namespace sk {

bool world_init();
void world_update();
void world_shutdown();
void world_refresh_transforms();

} // namespace sk