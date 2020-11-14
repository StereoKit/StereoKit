#pragma once
#include "../../stereokit.h"

namespace sk {

bool platform_init          ();
void platform_shutdown      ();
void platform_set_window    (void *window);
void platform_set_window_xam(void *window);
bool platform_set_mode      (runtime_ mode);
void platform_step_begin    ();
void platform_step_end      ();
void platform_present       ();
void platform_stop_mode     ();

} // namespace sk