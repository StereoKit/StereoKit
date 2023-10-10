#pragma once
#include "platform_utils.h"

#if defined(SK_OS_ANDROID)

#include <android/native_activity.h>

namespace sk {

extern JavaVM        *android_vm;
extern jobject        android_activity;
extern ANativeWindow *android_window;
extern AAssetManager *android_asset_manager;

bool android_init           ();
void android_set_window     (void *window);
void android_set_window_xam (void *window);
bool android_start_pre_xr   ();
bool android_start_post_xr  ();
bool android_start_flat     ();
void android_step_begin_xr  ();
void android_step_begin_flat();
void android_step_end_flat  ();
void android_stop_flat      ();
void android_shutdown       ();

bool android_check_manifest_permission(const char* permission);
bool android_check_app_permission     (const char* permission);
void android_request_permission       (const char* permission);

} // namespace sk

#endif