#pragma once

#ifndef SK_NO_FLATSCREEN

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace sk {

extern HWND  win32_window;
extern float win32_scroll;

bool win32_init(const char *app_name);
void win32_shutdown();
void win32_step_begin();
void win32_step_end();
void win32_vsync();

} // namespace sk

#endif // SK_NO_FLATSCREEN