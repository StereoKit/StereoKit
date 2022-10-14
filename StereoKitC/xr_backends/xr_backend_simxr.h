#pragma once

namespace sk {

bool xr_backend_simxr_init      ();
void xr_backend_simxr_shutdown  ();
void xr_backend_simxr_step_begin();
void xr_backend_simxr_step_end  ();

} // namespace sk
