#pragma once

#ifndef SK_NO_FLATSCREEN
#ifndef WINDOWS_UWP

namespace sk {

extern float win32_scroll;

bool win32_init(const char *app_name);
void win32_shutdown();
void win32_step_begin();
void win32_step_end();
void win32_vsync();
void*win32_hwnd();

} // namespace sk

#endif // WINDOWS_UWP
#endif // SK_NO_FLATSCREEN