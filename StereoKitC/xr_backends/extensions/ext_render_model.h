// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

#pragma once

#include "../../stereokit.h"
#include <openxr/openxr.h>

namespace sk {

typedef struct xr_render_model_t {
	int32_t               asset_idx;
	model_t               model;
	XrSpace               space;
	XrRenderModelStateEXT state_query;
} xr_render_model_t;

void              xr_ext_render_model_register ();
bool              xr_ext_render_model_get      (XrRenderModelIdEXT id, xr_render_model_t* out_model);
void              xr_ext_render_model_update   (xr_render_model_t* ref_model);
void              xr_ext_render_model_destroy  (xr_render_model_t* ref_model);
XrRenderModelEXT  xr_ext_render_model_get_model(const xr_render_model_t* model);

}