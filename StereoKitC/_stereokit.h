#pragma once

#include "stereokit.h"

namespace sk {

extern char          sk_sys_name[256];
extern const char   *sk_app_name;
extern sk_settings_t sk_settings;
extern system_info_t sk_info;
extern display_mode_ sk_display_mode;
extern bool          sk_no_flatscreen_fallback;

extern app_focus_ sk_focus;
extern bool32_t   sk_running;
extern bool32_t   sk_initialized;
extern bool32_t   sk_first_step;

void sk_update_timer();
void sk_assert_thread_valid();

} // namespace sk