#pragma once

#include <stereokit.h>

// Subscribes to the log. Safe (and best) to call before sk_init so the window
// captures logs emitted during startup. Styles are created lazily on the first
// log_window_update, once StereoKit is running.
void log_window_init();
void log_window_update(sk::pose_t* pose);
void log_window_shutdown();
