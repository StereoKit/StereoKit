/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

#include "../platforms/platform.h"
#if defined(SK_XR_OPENXR)

#include "anchor_openxr_fb.h"
#include "openxr_fb_entity.h"
#include "openxr_extensions.h"
#include "../asset_types/anchor.h"
#include "../systems/input.h"

#include <stdio.h>

namespace sk {

typedef struct oxr_fb_world_anchor_sys_t {
	anchor_type_id                     id;
	array_t<anchor_t>                  anchors;
} oxr_fb_world_anchor_sys_t;

typedef struct oxr_fb_world_anchor_t {
	XrSpace space;
} oxr_fb_world_anchor_t;

oxr_fb_world_anchor_sys_t oxr_fb_anchor_sys = {};

///////////////////////////////////////////

bool32_t anchor_oxr_fb_init() {
	if (!xr_ext_available.FB_spatial_entity) return false;

	oxr_fb_anchor_sys = {};

	// Check on the anchor store for any persisted anchors
	if (!xr_ext_available.FB_spatial_entity_storage) return true;

	return true;
}

///////////////////////////////////////////

void anchor_oxr_fb_shutdown() {
	oxr_fb_anchor_sys.anchors.free();
}

///////////////////////////////////////////

void anchor_oxr_fb_step() {
	for(int32_t i=0; i<oxr_fb_anchor_sys.anchors.count; i+=1) {
		anchor_t               anchor = oxr_fb_anchor_sys.anchors[i];
		oxr_fb_world_anchor_t* data   = (oxr_fb_world_anchor_t*)anchor->data;

		anchor->tracked = button_make_state(openxr_get_space(data->space, &anchor->pose), (anchor->tracked & button_state_active) != 0);
	}
}

///////////////////////////////////////////

anchor_caps_ anchor_oxr_fb_capabilities() {
	anchor_caps_ result = {};
	if (xr_ext_available.FB_spatial_entity        ) result |= anchor_caps_stability;
	if (xr_ext_available.FB_spatial_entity_storage) result |= anchor_caps_storable;
	return result;
}

///////////////////////////////////////////

void anchor_oxr_fb_clear_stored() {
	//if (xr_ext_available.FB_spatial_entity_storage)
	//	xr_extensions.FB
}

///////////////////////////////////////////

anchor_t anchor_oxr_fb_create(pose_t pose, const char* name_utf8) {
	return nullptr;
}

///////////////////////////////////////////

void anchor_oxr_fb_destroy(anchor_t anchor) {
	int32_t idx = oxr_fb_anchor_sys.anchors.index_of(anchor);
	if (idx >= 0) oxr_fb_anchor_sys.anchors.remove(idx);

	oxr_fb_world_anchor_t* data = (oxr_fb_world_anchor_t*)anchor->data;
	xrDestroySpace(data->space);
	free(data);
}

///////////////////////////////////////////

bool32_t anchor_oxr_fb_persist(anchor_t anchor, bool32_t persist) {
	if (anchor->persisted == persist || !xr_ext_available.FB_spatial_entity_storage) return anchor->persisted == persist;

	if (persist) {
		oxr_fb_world_anchor_t* anchor_data = (oxr_fb_world_anchor_t*)anchor->data;
	} else {
	}
	anchor->persisted = persist;
	return true;
}

} // namespace sk
#endif