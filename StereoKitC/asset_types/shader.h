#pragma once

#include <sk_gpu.h>
#include "../stereokit.h"
#include "assets.h"

namespace sk {

extern const size_t shaderarg_sz[];

struct _shader_t {
	asset_header_t header;
	skg_shader_t   shader;
};

void shader_destroy(shader_t shader);

} // namespace sk