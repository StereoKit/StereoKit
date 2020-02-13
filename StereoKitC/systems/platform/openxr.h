#pragma once

#include <stdint.h>

namespace sk {

bool openxr_init          (const char *app_name);
void openxr_shutdown      ();
void openxr_step_begin    ();
void openxr_step_end      ();
void openxr_poll_events   ();
void openxr_render_frame  ();
void openxr_make_actions  ();
void openxr_poll_actions  ();

int64_t openxr_get_time();

} // namespace sk