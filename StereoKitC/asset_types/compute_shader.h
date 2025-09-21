// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

#pragma once

#include "../stereokit.h"
#include "assets.h"

namespace sk {

struct compute_tex_bind_t {
	tex_t texture;
};

struct compute_buffer_bind_t {
	material_buffer_t buffer;
};

struct _compute_shader_t {
	asset_header_t header;
	shader_t       shader;
	compute_tex_bind_t *textures;
	int32_t             texture_count;
};

void compute_shader_destroy(compute_shader_t shader);

}; // namespace sk