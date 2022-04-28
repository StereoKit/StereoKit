#pragma once

#include "stereokit.h"

namespace sk {

extern const char   *sk_app_name;
extern sk_settings_t sk_settings;
extern system_info_t sk_info;
extern display_mode_ sk_display_mode;
extern bool          sk_no_flatscreen_fallback;

extern float    sk_timevf;
extern double   sk_timev;
extern double   sk_timev_elapsed;
extern float    sk_timev_elapsedf;
extern uint64_t sk_timev_raw;

extern app_focus_ sk_focus;
extern bool32_t   sk_running;
extern bool32_t   sk_initialized;
extern bool32_t   sk_first_step;

void sk_update_timer();
void sk_assert_thread_valid();

} // namespace sk