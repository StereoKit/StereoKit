#pragma once
#include "../stereokit.h"

namespace sk {

void flatscreen_input_init();
void flatscreen_input_shutdown();
void flatscreen_input_update();

bool flatscreen_is_simulating_movement();

extern matrix fltscr_transform;

} // namespace sk