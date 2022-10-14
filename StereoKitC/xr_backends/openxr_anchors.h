#pragma once

#include "../platforms/platform_utils.h"
#if defined(SK_XR_OPENXR)

#include "../stereokit.h"
#include "../libraries/array.h"

#include <openxr/openxr.h>
#include <stdint.h>

namespace sk {

void oxr_anchor_msft_init();
void oxr_anchor_msft_shutdown();

} // namespace sk

#endif