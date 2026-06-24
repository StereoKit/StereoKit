/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 */

// This implements XR_ANDROID_trackables anchor spaces
// https://registry.khronos.org/OpenXR/specs/1.1/html/xrspec.html#XR_ANDROID_trackables

#include "android_anchors.h"
#include "ext_management.h"

#include "../../stereokit.h"
#include "../../asset_types/anchor.h"
#include "../../systems/input.h"

#define XR_EXT_ANCHOR_FUNCTIONS( X ) \
	X(xrCreateAnchorSpaceANDROID)
OPENXR_DEFINE_FN_STATIC(XR_EXT_ANCHOR_FUNCTIONS);

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

typedef struct xr_android_anchors_state_t {
	bool              available;
	anchor_type_id    id;
	array_t<anchor_t> anchors;
} xr_android_anchors_state_t;
static xr_android_anchors_state_t local = { };

typedef struct oxr_android_world_anchor_t {
	XrSpace space;
} oxr_android_world_anchor_t;

///////////////////////////////////////////

xr_system_ xr_ext_android_spatial_anchors_initialize(void*);
void       xr_ext_android_spatial_anchors_step_begin(void*);
void       xr_ext_android_spatial_anchors_shutdown  (void*);

///////////////////////////////////////////

void xr_ext_android_spatial_anchors_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_ANDROID_TRACKABLES_EXTENSION_NAME;
	sys.evt_initialize = { xr_ext_android_spatial_anchors_initialize };
	sys.evt_step_begin = { xr_ext_android_spatial_anchors_step_begin };
	sys.evt_shutdown   = { xr_ext_android_spatial_anchors_shutdown };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_android_spatial_anchors_initialize(void*) {
	if (!backend_openxr_ext_enabled(XR_ANDROID_TRACKABLES_EXTENSION_NAME))
		return xr_system_fail;

	OPENXR_LOAD_FN_RETURN(XR_EXT_ANCHOR_FUNCTIONS, xr_system_fail);

	local.available = true;
	return xr_system_succeed;
}

///////////////////////////////////////////

void xr_ext_android_spatial_anchors_shutdown(void*) {
	for (int32_t i = local.anchors.count - 1; i >= 0; i--)
		anchor_release(local.anchors[i]);

	local.anchors.free();
	local = {};

	OPENXR_CLEAR_FN(XR_EXT_ANCHOR_FUNCTIONS);
}

///////////////////////////////////////////

void xr_ext_android_spatial_anchors_step_begin(void*) {
	for (int32_t i = 0; i < local.anchors.count; i += 1) {
		anchor_t                    anchor = local.anchors[i];
		oxr_android_world_anchor_t* data   = (oxr_android_world_anchor_t*)anchor->data;

		anchor->tracked = button_make_state(
			(anchor->tracked & button_state_active) != 0,
			openxr_get_space(data->space, &anchor->pose));
	}
}

///////////////////////////////////////////

bool xr_ext_android_spatial_anchors_available() {
	return local.available;
}

///////////////////////////////////////////

anchor_caps_ xr_ext_android_spatial_anchors_capabilities() {
	anchor_caps_ result = {};
	if (local.available) result |= anchor_caps_stability;
	return result;
}

///////////////////////////////////////////

void xr_ext_android_spatial_anchors_clear_stored() {
}

///////////////////////////////////////////

anchor_t xr_ext_android_spatial_anchors_create(pose_t pose, const char* name_utf8) {
	XrAnchorSpaceCreateInfoANDROID info = { XR_TYPE_ANCHOR_SPACE_CREATE_INFO_ANDROID };
	memcpy(&info.pose.position,    &pose.position,    sizeof(vec3));
	memcpy(&info.pose.orientation, &pose.orientation, sizeof(quat));
	info.space     = xr_app_space;
	info.time      = xr_time;
	info.trackable = XR_NULL_TRACKABLE_ANDROID;

	XrSpace  space;
	XrResult result = xrCreateAnchorSpaceANDROID(xr_session, &info, &space);
	if (XR_FAILED(result)) {
		log_warnf("xrCreateAnchorSpaceANDROID failed: %s", openxr_string(result));
		return nullptr;
	}

	bool32_t tracked = openxr_get_space(space, &pose);

	oxr_android_world_anchor_t* anchor_data = sk_malloc_t(oxr_android_world_anchor_t, 1);
	anchor_data->space = space;
	anchor_t sk_anchor = anchor_create_manual(local.id, pose, name_utf8, (void*)anchor_data);
	sk_anchor->tracked = tracked ? button_state_active : button_state_inactive;
	local.anchors.add(sk_anchor);
	anchor_addref(sk_anchor);
	return sk_anchor;
}

///////////////////////////////////////////

void xr_ext_android_spatial_anchors_destroy(anchor_t anchor) {
	int32_t idx = local.anchors.index_of(anchor);
	if (idx >= 0) local.anchors.remove(idx);

	oxr_android_world_anchor_t* data = (oxr_android_world_anchor_t*)anchor->data;
	xrDestroySpace(data->space);
	sk_free(data);
}

///////////////////////////////////////////

bool32_t xr_ext_android_spatial_anchors_persist(anchor_t anchor, bool32_t persist) {
	return anchor->persisted == persist;
}

///////////////////////////////////////////

} // namespace sk
