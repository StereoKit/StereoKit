/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

// This implements XR_MSFT_spatial_anchor and XR_MSFT_spatial_anchor_persistence
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_MSFT_spatial_anchor
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_MSFT_spatial_anchor_persistence

#include "msft_anchors.h"
#include "ext_management.h"

#include "../../stereokit.h"
#include "../../asset_types/anchor.h"
#include "../../systems/input.h"

#define XR_EXT_ANCHOR_FUNCTIONS( X )             \
	X(xrCreateSpatialAnchorMSFT)                 \
	X(xrCreateSpatialAnchorSpaceMSFT)            \
	X(xrDestroySpatialAnchorMSFT)
#define XR_EXT_PERSISTENCE_FUNCTIONS( X )        \
	X(xrCreateSpatialAnchorStoreConnectionMSFT)  \
	X(xrDestroySpatialAnchorStoreConnectionMSFT) \
	X(xrEnumeratePersistedSpatialAnchorNamesMSFT)\
	X(xrCreateSpatialAnchorFromPersistedNameMSFT)\
	X(xrPersistSpatialAnchorMSFT)                \
	X(xrUnpersistSpatialAnchorMSFT)              \
	X(xrClearSpatialAnchorStoreMSFT)
OPENXR_DEFINE_FN_STATIC(XR_EXT_ANCHOR_FUNCTIONS);
OPENXR_DEFINE_FN_STATIC(XR_EXT_PERSISTENCE_FUNCTIONS);

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

typedef struct xr_spatial_anchors_state_t {
	bool                               available;
	bool                               persistence;
	anchor_type_id                     id;
	XrSpatialAnchorStoreConnectionMSFT store;
	array_t<anchor_t>                  anchors;
} xr_spatial_anchors_state_t;
static xr_spatial_anchors_state_t local = { };

typedef struct oxr_msft_world_anchor_t {
	XrSpatialAnchorMSFT anchor;
	XrSpace             space;
} oxr_msft_world_anchor_t;

///////////////////////////////////////////

xr_system_ xr_ext_msft_spatial_anchors_initialize             (void*);
void       xr_ext_msft_spatial_anchors_step_begin             (void*);
void       xr_ext_msft_spatial_anchors_shutdown               (void*);
bool       xr_ext_msft_spatial_anchors_load_persistent_anchors();

///////////////////////////////////////////

void xr_ext_msft_spatial_anchors_register() {
	xr_system_t sys = {};
	sys.request_exts    [sys.request_ext_count    ++] = XR_MSFT_SPATIAL_ANCHOR_EXTENSION_NAME;
	sys.request_opt_exts[sys.request_opt_ext_count++] = XR_MSFT_SPATIAL_ANCHOR_PERSISTENCE_EXTENSION_NAME;
	sys.evt_initialize = { xr_ext_msft_spatial_anchors_initialize };
	sys.evt_step_begin = { xr_ext_msft_spatial_anchors_step_begin };
	sys.evt_shutdown   = { xr_ext_msft_spatial_anchors_shutdown };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_msft_spatial_anchors_initialize(void*) {
	// Check if we got our extension
	if (!backend_openxr_ext_enabled(XR_MSFT_SPATIAL_ANCHOR_EXTENSION_NAME))
		return xr_system_fail;
	local.persistence = backend_openxr_ext_enabled(XR_MSFT_SPATIAL_ANCHOR_PERSISTENCE_EXTENSION_NAME);

	// Load all the main anchor extension functions
	OPENXR_LOAD_FN_RETURN(XR_EXT_ANCHOR_FUNCTIONS, xr_system_fail);

	if (local.persistence) {
		// Load the persistence extension functions
		OPENXR_LOAD_FN_RETURN(XR_EXT_PERSISTENCE_FUNCTIONS, xr_system_fail);

		// Load all the persistent anchors from storage
		if (xr_ext_msft_spatial_anchors_load_persistent_anchors() == false)
			return xr_system_fail;
	}

	local.available = true;
	return xr_system_succeed;
}

///////////////////////////////////////////

void xr_ext_msft_spatial_anchors_shutdown(void*) {
	for (int32_t i = local.anchors.count - 1; i >= 0; i--)
		anchor_release(local.anchors[i]);

	if (local.store != XR_NULL_HANDLE) xrDestroySpatialAnchorStoreConnectionMSFT(local.store);
	local.anchors.free();
	local = {};

	OPENXR_CLEAR_FN(XR_EXT_ANCHOR_FUNCTIONS);
	OPENXR_CLEAR_FN(XR_EXT_PERSISTENCE_FUNCTIONS);
}

///////////////////////////////////////////

void xr_ext_msft_spatial_anchors_step_begin(void*) {
	for (int32_t i = 0; i < local.anchors.count; i += 1) {
		anchor_t                 anchor = local.anchors[i];
		oxr_msft_world_anchor_t* data   = (oxr_msft_world_anchor_t*)anchor->data;

		anchor->tracked = button_make_state(
			(anchor->tracked & button_state_active) != 0,
			openxr_get_space(data->space, &anchor->pose));
	}
}

///////////////////////////////////////////

bool xr_ext_msft_spatial_anchors_available() {
	return local.available;
}

///////////////////////////////////////////

bool xr_ext_msft_spatial_anchors_load_persistent_anchors() {
	XrResult result = xrCreateSpatialAnchorStoreConnectionMSFT(xr_session, &local.store);
	if (XR_FAILED(result)) {
		log_warnf("xrCreateSpatialAnchorStoreConnectionMSFT failed: %s", openxr_string(result));
		return false;
	}

	uint32_t count = 0;
	xrEnumeratePersistedSpatialAnchorNamesMSFT(local.store, 0, &count, nullptr);
	if (count == 0) return true;

	XrSpatialAnchorPersistenceNameMSFT* names = sk_malloc_t(XrSpatialAnchorPersistenceNameMSFT, count);
	xrEnumeratePersistedSpatialAnchorNamesMSFT(local.store, count, &count, names);

	// Create all anchors in the store!
	for (uint32_t i=0; i < count; i+=1) {
		XrSpatialAnchorMSFT anchor;
		XrSpatialAnchorFromPersistedAnchorCreateInfoMSFT info = { XR_TYPE_SPATIAL_ANCHOR_FROM_PERSISTED_ANCHOR_CREATE_INFO_MSFT };
		info.spatialAnchorPersistenceName = names[i];
		info.spatialAnchorStore           = local.store;
		result = xrCreateSpatialAnchorFromPersistedNameMSFT(xr_session, &info, &anchor);
		if (XR_FAILED(result)) {
			log_warnf("xrCreateSpatialAnchorFromPersistedNameMSFT failed: %s", openxr_string(result));
			continue;
		}

		// Create a space for getting the position of the anchor
		XrSpatialAnchorSpaceCreateInfoMSFT space_info = { XR_TYPE_SPATIAL_ANCHOR_SPACE_CREATE_INFO_MSFT };
		space_info.anchor            = anchor;
		space_info.poseInAnchorSpace = { {0,0,0,1}, {0,0,0} };
		XrSpace space;
		result = xrCreateSpatialAnchorSpaceMSFT(xr_session, &space_info, &space);
		if (XR_FAILED(result)) {
			log_warnf("xrCreateSpatialAnchorSpaceMSFT failed: %s", openxr_string(result));
			xrDestroySpatialAnchorMSFT(anchor);
			continue;
		}

		// We'll need to fetch the pose real quick
		pose_t   pose    = pose_identity;
		bool32_t tracked = openxr_get_space(space, &pose);

		// Create a StereoKit anchor
		oxr_msft_world_anchor_t* anchor_data = sk_malloc_t(oxr_msft_world_anchor_t, 1);
		anchor_data->anchor = anchor;
		anchor_data->space  = space;
		anchor_t sk_anchor = anchor_create_manual(local.id, pose, names[i].name, (void*)anchor_data);
		sk_anchor->persisted = true;
		sk_anchor->tracked   = tracked ? button_state_active : button_state_inactive;
		local.anchors.add(sk_anchor);
	}
	return true;
}

///////////////////////////////////////////

anchor_caps_ xr_ext_msft_spatial_anchors_capabilities() {
	anchor_caps_ result = {};
	if (local.available  ) result |= anchor_caps_stability;
	if (local.persistence) result |= anchor_caps_storable;
	return result;
}

///////////////////////////////////////////

void xr_ext_msft_spatial_anchors_clear_stored() {
	if (local.persistence)
		xrClearSpatialAnchorStoreMSFT(local.store);
}

///////////////////////////////////////////

anchor_t xr_ext_msft_spatial_anchors_create(pose_t pose, const char* name_utf8) {
	// Create the anchor object
	XrSpatialAnchorCreateInfoMSFT info = { XR_TYPE_SPATIAL_ANCHOR_CREATE_INFO_MSFT };
	memcpy(&info.pose.position,    &pose.position,    sizeof(vec3));
	memcpy(&info.pose.orientation, &pose.orientation, sizeof(quat));
	info.space = xr_app_space;
	info.time  = xr_time;
	XrSpatialAnchorMSFT anchor;
	XrResult result = xrCreateSpatialAnchorMSFT(xr_session, &info, &anchor);
	if (XR_FAILED(result)) {
		log_warnf("xrCreateSpatialAnchorMSFT failed: %s", openxr_string(result));
		return nullptr;
	}

	// Create a space for getting the position of the anchor
	XrSpatialAnchorSpaceCreateInfoMSFT space_info = { XR_TYPE_SPATIAL_ANCHOR_SPACE_CREATE_INFO_MSFT };
	space_info.anchor            = anchor;
	space_info.poseInAnchorSpace = { {0,0,0,1}, {0,0,0} };
	XrSpace space;
	result = xrCreateSpatialAnchorSpaceMSFT(xr_session, &space_info, &space);
	if (XR_FAILED(result)) {
		log_warnf("xrCreateSpatialAnchorSpaceMSFT failed: %s", openxr_string(result));
		xrDestroySpatialAnchorMSFT(anchor);
		return nullptr;
	}

	// We'll need to fetch the pose real quick
	bool32_t tracked = openxr_get_space(space, &pose);

	// Create a StereoKit anchor
	oxr_msft_world_anchor_t* anchor_data = sk_malloc_t(oxr_msft_world_anchor_t, 1);
	anchor_data->anchor = anchor;
	anchor_data->space  = space;
	anchor_t sk_anchor = anchor_create_manual(local.id, pose, name_utf8, (void*)anchor_data);
	sk_anchor->tracked = tracked ? button_state_active : button_state_inactive;
	local.anchors.add(sk_anchor);
	anchor_addref(sk_anchor);
	return sk_anchor;
}

///////////////////////////////////////////

void xr_ext_msft_spatial_anchors_destroy(anchor_t anchor) {
	int32_t idx = local.anchors.index_of(anchor);
	if (idx >= 0) local.anchors.remove(idx);

	oxr_msft_world_anchor_t* data = (oxr_msft_world_anchor_t*)anchor->data;
	xrDestroySpace(data->space);
	xrDestroySpatialAnchorMSFT(data->anchor);
	sk_free(data);
}

///////////////////////////////////////////

bool32_t xr_ext_msft_spatial_anchors_persist(anchor_t anchor, bool32_t persist) {
	if (anchor->persisted == persist || local.persistence == false) return anchor->persisted == persist;

	if (persist) {
		oxr_msft_world_anchor_t* anchor_data = (oxr_msft_world_anchor_t*)anchor->data;
		XrSpatialAnchorPersistenceInfoMSFT persist_info = { XR_TYPE_SPATIAL_ANCHOR_PERSISTENCE_INFO_MSFT };
		strncpy(persist_info.spatialAnchorPersistenceName.name, anchor->name, sizeof(persist_info.spatialAnchorPersistenceName.name));
		persist_info.spatialAnchor = anchor_data->anchor;
		XrResult result = xrPersistSpatialAnchorMSFT(local.store, &persist_info);
		if (XR_FAILED(result)) {
			log_warnf("%s [%s]", "xrPersistSpatialAnchorMSFT", openxr_string(result));
			return false;
		}
	} else {
		XrSpatialAnchorPersistenceNameMSFT name_info = { };
		strncpy(name_info.name, anchor->name, sizeof(name_info.name));
		XrResult result = xrUnpersistSpatialAnchorMSFT(local.store, &name_info);
		if (XR_FAILED(result)) {
			log_warnf("%s [%s]", "xrUnpersistSpatialAnchorMSFT", openxr_string(result));
			return false;
		}
	}
	anchor->persisted = persist;
	return true;
}

///////////////////////////////////////////

} // namespace sk