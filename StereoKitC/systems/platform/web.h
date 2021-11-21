#pragma once
#include "platform_utils.h"

#if defined(SK_OS_WEB)

namespace sk {

bool web_init           ();
bool web_start_pre_xr   ();
bool web_start_post_xr  ();
bool web_start_flat     ();
void web_step_begin_xr  ();
void web_step_begin_flat();
void web_step_end_flat  ();
void web_stop_flat      ();
void web_shutdown       ();

void web_start_main_loop(void (*app_update)(void), void (*app_shutdown)(void));

} // namespace sk

#endif // defined(SK_OS_WEB)