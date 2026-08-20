/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 */

#pragma once

#include "../../platforms/platform.h"
#include "../../stereokit.h"
#if defined(SK_XR_OPENXR)
#include "../openxr.h"
#endif

namespace sk {

#if defined(SK_XR_OPENXR)

void xr_ext_meta_environment_depth_register();
void xr_ext_meta_environment_depth_update_frame(XrTime display_time);

bool               xr_ext_meta_environment_depth_available();
bool               xr_ext_meta_environment_depth_running();
sensor_depth_caps_ xr_ext_meta_environment_depth_get_capabilities();
bool               xr_ext_meta_environment_depth_start(sensor_depth_caps_ flags);
void               xr_ext_meta_environment_depth_stop();
bool               xr_ext_meta_environment_depth_set_caps(sensor_depth_caps_ flags);
tex_t              xr_ext_meta_environment_depth_get_texture();
bool               xr_ext_meta_environment_depth_try_get_latest(sensor_depth_frame_t* out_info);

#else

// Stubs, so callers can stay readable inline rather than #if at each use
inline bool               xr_ext_meta_environment_depth_available()                   { return false; }
inline bool               xr_ext_meta_environment_depth_running()                     { return false; }
inline sensor_depth_caps_ xr_ext_meta_environment_depth_get_capabilities()            { return sensor_depth_caps_none; }
inline bool               xr_ext_meta_environment_depth_start(sensor_depth_caps_)     { return false; }
inline void               xr_ext_meta_environment_depth_stop()                        {}
inline bool               xr_ext_meta_environment_depth_set_caps(sensor_depth_caps_)  { return false; }
inline tex_t              xr_ext_meta_environment_depth_get_texture()                 { return nullptr; }
inline bool               xr_ext_meta_environment_depth_try_get_latest(sensor_depth_frame_t*) { return false; }

#endif

}
