#pragma once

#include "stereokit.h"

namespace sk {

bool android_init      (const char *app_name);
void android_shutdown  ();
void android_step_begin();
void android_step_end  ();
void android_vsync     ();

} // namespace sk