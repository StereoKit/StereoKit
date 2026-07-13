#pragma once

#include <sk_renderer.h>
#include "../stereokit.h"
#include "assets.h"

namespace sk {

struct _compute_buffer_t {
	asset_header_t       header;
	skr_buffer_t         gpu_buffer;
	compute_buffer_type_ type;
	int32_t              element_count;
	int32_t              element_size;
};

void compute_buffer_destroy(compute_buffer_t buffer);

} // namespace sk
