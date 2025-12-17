#pragma once

#include <sk_renderer.h>
#include "../stereokit.h"
#include "assets.h"

namespace sk {

extern const size_t shaderarg_sz[];

struct _shader_t {
	asset_header_t header;
	skr_shader_t   gpu_shader;
};

void shader_destroy(shader_t shader);

} // namespace sk