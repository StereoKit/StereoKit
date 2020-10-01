#pragma once

namespace sk {

bool android_init      (void *from_window);
void android_shutdown  ();
void android_step_begin();
void android_step_end  ();
void android_vsync     ();

} // namespace sk