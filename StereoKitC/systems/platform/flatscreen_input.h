#pragma once

#ifndef SK_NO_FLATSCREEN

namespace sk {

void flatscreen_input_init();
void flatscreen_input_shutdown();
void flatscreen_input_update();

bool flatscreen_is_simulating_movement();

} // namespace sk

#endif // SK_NO_FLATSCREEN