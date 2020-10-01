#pragma once

#ifdef __ANDROID__

#include <android/native_activity.h>

namespace sk {

extern ANativeActivity* android_activity;
extern ANativeWindow*   android_window;

bool android_init      (void *from_window);
void android_shutdown  ();
void android_step_begin();
void android_step_end  ();
void android_vsync     ();

} // namespace sk

#endif