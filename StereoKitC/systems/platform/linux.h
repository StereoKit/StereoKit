#pragma once
#include "platform_utils.h"

#if defined(SK_OS_LINUX)

#include "../../stereokit.h"

namespace sk {

bool  linux_init           ();
bool  linux_start_pre_xr   ();
bool  linux_start_post_xr  ();
bool  linux_start_flat     ();
void  linux_step_begin_xr  ();
void  linux_step_begin_flat();
void  linux_step_end_flat  ();
void  linux_stop_flat      ();
void  linux_shutdown       ();

void  linux_resize(int width, int height);

bool  linux_get_cursor(vec2 &out_pos);
float linux_get_scroll();
void  linux_set_cursor(vec2 window_pos);

} // namespace sk

#endif // defined(SK_OS_LINUX)