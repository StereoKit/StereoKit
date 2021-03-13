#pragma once
#include "platform_utils.h"

#if defined(SK_OS_WINDOWS)

namespace sk {

extern float win32_scroll;

bool win32_init      ();
void win32_shutdown  ();
bool win32_start     ();
void win32_stop      ();
void win32_step_begin();
void win32_step_end  ();
void win32_vsync     ();
void*win32_hwnd      ();

} // namespace sk

#endif // defined(SK_OS_WINDOWS)