#pragma once

namespace sk {

bool physics_init(void *arg);
void physics_shutdown();
void physics_update();

extern double physics_sim_time;

} // namespace sk