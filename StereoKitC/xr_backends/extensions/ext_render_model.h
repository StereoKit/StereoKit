// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

#pragma once

#include "../../stereokit.h"
#include <openxr/openxr.h>

namespace sk {

typedef struct xr_render_model_t {
	XrRenderModelIdEXT    id;
	XrRenderModelEXT      render_model;
	model_t               model;
	XrSpace               space;
	model_node_id*        anim_nodes;
	int32_t               anim_node_count;
	XrRenderModelStateEXT state_query;
} xr_render_model_t;

void              xr_ext_render_model_register();
xr_render_model_t xr_ext_render_model_get     (XrRenderModelIdEXT id);
void              xr_ext_render_model_update  (xr_render_model_t* ref_model);
void              xr_ext_render_model_destroy (xr_render_model_t* ref_model);

}