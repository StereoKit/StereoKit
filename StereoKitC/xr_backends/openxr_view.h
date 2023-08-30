#pragma once

#include "../platforms/platform.h"
#if defined(SK_XR_OPENXR)

#include "../stereokit.h"
#include "../libraries/array.h"

#include <openxr/openxr.h>
#include <stdint.h>

namespace sk {

bool openxr_views_create    ();
void openxr_views_destroy   ();
void openxr_views_update_fov();

void     xr_extension_structs_clear();
bool32_t xr_set_blend              (display_blend_ blend);
bool32_t xr_blend_valid            (display_blend_ blend);

extern array_t<XrViewConfigurationType> xr_display_types;

} // namespace sk

#endif