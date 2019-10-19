#pragma once

#include "stereokit.h"

namespace sk {

extern const char   *sk_app_name;
extern settings_t    sk_settings;
extern system_info_t sk_info;
extern runtime_      sk_runtime;
extern bool          sk_runtime_fallback;

extern float   sk_timevf;
extern double  sk_timev;
extern double  sk_timev_elapsed;
extern float   sk_timev_elapsedf;
extern int64_t sk_timev_raw;

extern bool32_t sk_focused;
extern bool32_t sk_run;

void sk_update_timer();

} // namespace sk