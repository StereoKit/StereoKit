#pragma once

#include "../stereokit.h"

namespace sk {

bool simulator_init();
void simulator_step_begin();
void simulator_step_end();
void simulator_shutdown();

void simulator_set_origin_offset(pose_t offset);

}