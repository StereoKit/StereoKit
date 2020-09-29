#pragma once

#ifndef WINDOWS_UWP

namespace sk {

extern float win32_scroll;

bool win32_init(void *from_window);
void win32_shutdown();
void win32_step_begin();
void win32_step_end();
void win32_vsync();
void*win32_hwnd();

} // namespace sk

#endif // WINDOWS_UWP