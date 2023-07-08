#pragma once
#include "../stereokit.h"

namespace sk {

// Platform lifecycle:
//
//// Initialization
// platform_init()
// Mixed Reality:
//    platform_start_pre_xr()
//    openxr_init()
//    platform_start_post_xr()
// Flatscreen:
//    platform_start_flat()
//    simulator_init();
//
//// Main loop
// Mixed Reality:
//    platform_step_begin_xr()
// Flatscreen:
//    platform_step_begin_flat()
//    simulator_step_begin();
//    simulator_step_end();
//    platform_step_end_flat()
//
//// Shutdown
// Mixed Reality:
//    openxr_shutdown()
// Flatscreen:
//    simulator_shutdown();
//    platform_stop_flat()
// platform_shutdown()

bool platform_init          ();
void platform_shutdown      ();
void platform_set_window    (void *window);
void platform_set_window_xam(void *window);
bool platform_set_mode      (display_type_ mode);
void platform_step_begin    ();
void platform_step_end      ();
void platform_stop_mode     ();

} // namespace sk