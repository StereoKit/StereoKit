#pragma once

#ifdef __linux__

namespace sk {

bool linux_setup     (void *from_window);
bool linux_init      ();
void linux_shutdown  ();
void linux_step_begin();
void linux_step_end  ();
void linux_vsync     ();

} // namespace sk

#endif
