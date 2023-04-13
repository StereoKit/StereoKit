#pragma once

#include "../platforms/platform_utils.h"
#if defined(SK_XR_OPENXR)

#include "../stereokit.h"
#include "../libraries/array.h"

#include <openxr/openxr.h>
#include <stdint.h>

namespace sk {

void anchor_oxr_msft_init();
void anchor_oxr_msft_shutdown();

} // namespace sk

#endif