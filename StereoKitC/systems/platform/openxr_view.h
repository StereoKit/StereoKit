#pragma once

#include "../../stereokit.h"

#include <openxr/openxr.h>
#include <stdint.h>

namespace sk {

bool openxr_views_create ();
void openxr_views_destroy();
void openxr_views_render (XrTime time);

extern uint32_t                 xr_display_count;
extern XrViewConfigurationType *xr_display_types;

} // namespace sk