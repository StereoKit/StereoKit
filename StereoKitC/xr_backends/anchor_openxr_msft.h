#pragma once

#include "../platforms/platform_utils.h"
#if defined(SK_XR_OPENXR)

#include "../stereokit.h"
#include "../libraries/array.h"

#include <openxr/openxr.h>
#include <stdint.h>

namespace sk {

bool32_t anchor_oxr_msft_init        ();
void     anchor_oxr_msft_shutdown    ();
void     anchor_oxr_msft_step        ();
anchor_t anchor_oxr_msft_create      (pose_t pose, const char* name_utf8);
void     anchor_oxr_msft_destroy     (anchor_t anchor);
void     anchor_oxr_msft_clear_stored();
bool32_t anchor_oxr_msft_persist     (anchor_t anchor, bool32_t persist);
anchor_props_ anchor_oxr_msft_properties();

} // namespace sk

#endif