#pragma once

#include "../../stereokit.h"
#include "../../libraries/array.h"

#include <openxr/openxr.h>
#include <stdint.h>

namespace sk {

bool openxr_views_create ();
void openxr_views_destroy();
void openxr_views_render (XrTime time);

extern array_t<XrViewConfigurationType> xr_display_types;

} // namespace sk