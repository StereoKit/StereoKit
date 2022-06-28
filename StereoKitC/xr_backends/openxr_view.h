#pragma once

#include "../platforms/platform_utils.h"
#if defined(SK_XR_OPENXR)

#include "../stereokit.h"
#include "../libraries/array.h"

#include <openxr/openxr.h>
#include <stdint.h>

namespace sk {

bool openxr_views_create ();
void openxr_views_destroy();
void openxr_views_render (XrTime time);

void xr_compositor_layers_clear();

extern array_t<XrViewConfigurationType> xr_display_types;

} // namespace sk

#endif