// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith

#pragma once
#include "_platform.h"

#if defined(SK_OS_WEB)

#include <stdint.h>
#include <stddef.h>

namespace sk {

// The browser supplies everything platform-shaped here, by way of sk_app, so
// this backend mostly fills in the platform_ contract from platform.h.

// Decodes an image with the browser's own decoder, which runs off-thread
// where wasm would stall the frame. Results queue up for the main thread;
// poll one per call until it returns false. The Blob snapshot means `bytes`
// only has to live until begin returns. rgba comes back sk_malloc'd, or null
// when the decode failed.
void web_image_decode_begin(uint64_t id, const void *bytes, size_t size);
bool web_image_decode_poll (uint64_t *out_id, void **out_rgba, int32_t *out_width, int32_t *out_height);

} // namespace sk
#endif // defined(SK_OS_WEB)
