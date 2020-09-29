#ifdef __ANDROID__

#include "android.h"
#include "_stereokit.h"

#include <android/native_activity.h>

namespace sk {

ANativeActivity* android_activity;
ANativeWindow*   android_window;

bool android_init(const char *app_name) {
}
void android_shutdown() {}
void android_step_begin() {}
void android_step_end() {}
void android_vsync() {}

} // namespace sk

#endif