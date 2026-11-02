// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2024 Nick Klingensmith
// Copyright (c) 2024 Qualcomm Technologies, Inc.

#pragma once

#include "render.h"

#include "../stereokit.h"
#include "../sk_math_dx.h"
#include "../libraries/array.h"
#include "../asset_types/assets.h"

using namespace DirectX;

namespace sk {

struct render_item_t {
	XMMATRIX    transform;
	color128    color;
	mesh_t      mesh;
	material_t  material;
	int32_t     mesh_inds;
	uint16_t    layer;
};

struct _render_list_t {
	asset_header_t         header;
	array_t<render_item_t> queue;
	int32_t                prev_count;
	render_list_refs_      refs;
};


} // namespace sk