/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 */

// Futures are sk_renderer's, which already handles any thread, slot reuse,
// and shutdown. These just translate the types.

#include "future.h"

namespace sk {

static_assert((int)future_state_failed  == (int)skr_future_state_failed  &&
              (int)future_state_none    == (int)skr_future_state_none    &&
              (int)future_state_pending == (int)skr_future_state_pending &&
              (int)future_state_ready   == (int)skr_future_state_ready, "future_state_ must match skr_future_state_");

///////////////////////////////////////////

future_state_ future_check(future_t future) {
	skr_future_t gpu = future_to_skr(future);
	return (future_state_)skr_future_check(&gpu);
}

///////////////////////////////////////////

future_state_ future_wait(future_t future) {
	skr_future_t gpu = future_to_skr(future);
	return (future_state_)skr_future_wait(&gpu);
}

///////////////////////////////////////////

future_t render_gpu_future() {
	return future_from_skr(skr_future_get_queue());
}

}
