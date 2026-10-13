/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 */

#pragma once

#include "../stereokit.h"
#include <sk_renderer.h>
#include <string.h>

namespace sk {

// future_t is skr_future_t with private fields, so these are plain copies
static_assert(sizeof(future_t) == sizeof(skr_future_t), "future_t must mirror skr_future_t");

inline future_t     future_from_skr(skr_future_t future) { future_t     result; memcpy(&result, &future, sizeof(result)); return result; }
inline skr_future_t future_to_skr  (future_t     future) { skr_future_t result; memcpy(&result, &future, sizeof(result)); return result; }

}
