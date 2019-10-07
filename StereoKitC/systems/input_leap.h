#ifndef SK_NO_LEAP_MOTION

#pragma once

namespace sk {

extern bool leap_has_device;

bool input_leap_init();
void input_leap_shutdown();
void input_leap_update();

} // namespace sk

#endif