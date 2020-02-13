#ifndef SK_NO_LEAP_MOTION

#pragma once

namespace sk {

extern bool leap_has_device;

bool hand_leap_present();
void hand_leap_init();
void hand_leap_shutdown();
void hand_leap_update_frame();
void hand_leap_update_predicted();

} // namespace sk

#endif