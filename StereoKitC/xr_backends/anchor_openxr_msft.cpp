#include "../platforms/platform.h"
#if defined(SK_XR_OPENXR)

#include "anchor_openxr_msft.h"
#include "openxr_extensions.h"
#include "../asset_types/anchor.h"
#include "../systems/input.h"

#include <stdio.h>

namespace sk {

typedef struct oxr_msft_world_anchor_sys_t {
	anchor_type_id                     id;
	XrSpatialAnchorStoreConnectionMSFT store;
	array_t<anchor_t>                  anchors;
} oxr_msft_world_anchor_sys_t;

typedef struct oxr_msft_world_anchor_t {
	XrSpatialAnchorMSFT anchor;
	XrSpace             space;
} oxr_msft_world_anchor_t;

oxr_msft_world_anchor_sys_t oxr_msft_anchor_sys = {};

///////////////////////////////////////////

bool32_t anchor_oxr_msft_init() {
	if (xr_ext.MSFT_spatial_anchor != xr_ext_active) return false;

	oxr_msft_anchor_sys = {};

	// Check on the anchor store for any persisted anchors
	if (xr_ext.MSFT_spatial_anchor_persistence != xr_ext_active) return true;

	XrResult result = xr_extensions.xrCreateSpatialAnchorStoreConnectionMSFT(xr_session, &oxr_msft_anchor_sys.store);
	if (XR_FAILED(result)) {
		log_warnf("xrCreateSpatialAnchorStoreConnectionMSFT failed: %s", openxr_string(result));
		return false;
	}

	uint32_t count = 0;
	xr_extensions.xrEnumeratePersistedSpatialAnchorNamesMSFT(oxr_msft_anchor_sys.store, 0, &count, nullptr);
	if (count == 0) return true;

	XrSpatialAnchorPersistenceNameMSFT* names = sk_malloc_t(XrSpatialAnchorPersistenceNameMSFT, count);
	xr_extensions.xrEnumeratePersistedSpatialAnchorNamesMSFT(oxr_msft_anchor_sys.store, count, &count, names);

	// Create all anchors in the store!
	for (uint32_t i=0; i < count; i+=1) {
		XrSpatialAnchorMSFT anchor;
		XrSpatialAnchorFromPersistedAnchorCreateInfoMSFT info = { XR_TYPE_SPATIAL_ANCHOR_FROM_PERSISTED_ANCHOR_CREATE_INFO_MSFT };
		info.spatialAnchorPersistenceName = names[i];
		info.spatialAnchorStore           = oxr_msft_anchor_sys.store;
		result = xr_extensions.xrCreateSpatialAnchorFromPersistedNameMSFT(xr_session, &info, &anchor);
		if (XR_FAILED(result)) {
			log_warnf("xrCreateSpatialAnchorFromPersistedNameMSFT failed: %s", openxr_string(result));
			continue;
		}

		// Create a space for getting the position of the anchor
		XrSpatialAnchorSpaceCreateInfoMSFT space_info = { XR_TYPE_SPATIAL_ANCHOR_SPACE_CREATE_INFO_MSFT };
		space_info.anchor            = anchor;
		space_info.poseInAnchorSpace = { {0,0,0,1}, {0,0,0} };
		XrSpace space;
		result = xr_extensions.xrCreateSpatialAnchorSpaceMSFT(xr_session, &space_info, &space);
		if (XR_FAILED(result)) {
			log_warnf("xrCreateSpatialAnchorSpaceMSFT failed: %s", openxr_string(result));
			xr_extensions.xrDestroySpatialAnchorMSFT(anchor);
			continue;
		}

		// We'll need to fetch the pose real quick
		pose_t   pose    = pose_identity;
		bool32_t tracked = openxr_get_space(space, &pose);

		// Create a StereoKit anchor
		oxr_msft_world_anchor_t* anchor_data = sk_malloc_t(oxr_msft_world_anchor_t, 1);
		anchor_data->anchor = anchor;
		anchor_data->space  = space;
		anchor_t sk_anchor = anchor_create_manual(oxr_msft_anchor_sys.id, pose, names[i].name, (void*)anchor_data);
		sk_anchor->persisted = true;
		sk_anchor->tracked   = tracked ? button_state_active : button_state_inactive;
		oxr_msft_anchor_sys.anchors.add(sk_anchor);
	}
	return true;
}

///////////////////////////////////////////

void anchor_oxr_msft_shutdown() {
	for (int32_t i = oxr_msft_anchor_sys.anchors.count-1; i >= 0; i--)
		anchor_release(oxr_msft_anchor_sys.anchors[i]);

	if (oxr_msft_anchor_sys.store != XR_NULL_HANDLE)
		xr_extensions.xrDestroySpatialAnchorStoreConnectionMSFT(oxr_msft_anchor_sys.store);
	oxr_msft_anchor_sys.anchors.free();
}

///////////////////////////////////////////

void anchor_oxr_msft_step() {
	for(int32_t i=0; i<oxr_msft_anchor_sys.anchors.count; i+=1) {
		anchor_t                 anchor = oxr_msft_anchor_sys.anchors[i];
		oxr_msft_world_anchor_t* data   = (oxr_msft_world_anchor_t*)anchor->data;

		anchor->tracked = button_make_state((anchor->tracked & button_state_active) != 0, openxr_get_space(data->space, &anchor->pose));
	}
}

///////////////////////////////////////////

anchor_caps_ anchor_oxr_msft_capabilities() {
	anchor_caps_ result = {};
	if (xr_ext.MSFT_spatial_anchor             == xr_ext_active) result |= anchor_caps_stability;
	if (xr_ext.MSFT_spatial_anchor_persistence == xr_ext_active) result |= anchor_caps_storable;
	return result;
}

///////////////////////////////////////////

void anchor_oxr_msft_clear_stored() {
	if (xr_ext.MSFT_spatial_anchor_persistence == xr_ext_active)
		xr_extensions.xrClearSpatialAnchorStoreMSFT(oxr_msft_anchor_sys.store);
}

///////////////////////////////////////////

anchor_t anchor_oxr_msft_create(pose_t pose, const char* name_utf8) {
	// Create the anchor object
	XrSpatialAnchorCreateInfoMSFT info = { XR_TYPE_SPATIAL_ANCHOR_CREATE_INFO_MSFT };
	memcpy(&info.pose.position,    &pose.position,    sizeof(vec3));
	memcpy(&info.pose.orientation, &pose.orientation, sizeof(quat));
	info.space = xr_app_space;
	info.time  = xr_time;
	XrSpatialAnchorMSFT anchor;
	XrResult result = xr_extensions.xrCreateSpatialAnchorMSFT(xr_session, &info, &anchor);
	if (XR_FAILED(result)) {
		log_warnf("xrCreateSpatialAnchorMSFT failed: %s", openxr_string(result));
		return nullptr;
	}

	// Create a space for getting the position of the anchor
	XrSpatialAnchorSpaceCreateInfoMSFT space_info = { XR_TYPE_SPATIAL_ANCHOR_SPACE_CREATE_INFO_MSFT };
	space_info.anchor            = anchor;
	space_info.poseInAnchorSpace = { {0,0,0,1}, {0,0,0} };
	XrSpace space;
	result = xr_extensions.xrCreateSpatialAnchorSpaceMSFT(xr_session, &space_info, &space);
	if (XR_FAILED(result)) {
		log_warnf("xrCreateSpatialAnchorSpaceMSFT failed: %s", openxr_string(result));
		xr_extensions.xrDestroySpatialAnchorMSFT(anchor);
		return nullptr;
	}

	// We'll need to fetch the pose real quick
	bool32_t tracked = openxr_get_space(space, &pose);

	// Create a StereoKit anchor
	oxr_msft_world_anchor_t* anchor_data = sk_malloc_t(oxr_msft_world_anchor_t, 1);
	anchor_data->anchor = anchor;
	anchor_data->space  = space;
	anchor_t sk_anchor = anchor_create_manual(oxr_msft_anchor_sys.id, pose, name_utf8, (void*)anchor_data);
	sk_anchor->tracked = tracked ? button_state_active : button_state_inactive;
	oxr_msft_anchor_sys.anchors.add(sk_anchor);
	anchor_addref(sk_anchor);
	return sk_anchor;
}

///////////////////////////////////////////

void anchor_oxr_msft_destroy(anchor_t anchor) {
	int32_t idx = oxr_msft_anchor_sys.anchors.index_of(anchor);
	if (idx >= 0) oxr_msft_anchor_sys.anchors.remove(idx);

	oxr_msft_world_anchor_t* data = (oxr_msft_world_anchor_t*)anchor->data;
	xrDestroySpace(data->space);
	xr_extensions.xrDestroySpatialAnchorMSFT(data->anchor);
	free(data);
}

///////////////////////////////////////////

bool32_t anchor_oxr_msft_persist(anchor_t anchor, bool32_t persist) {
	if (anchor->persisted == persist || xr_ext.MSFT_spatial_anchor_persistence != xr_ext_active) return anchor->persisted == persist;

	if (persist) {
		oxr_msft_world_anchor_t* anchor_data = (oxr_msft_world_anchor_t*)anchor->data;
		XrSpatialAnchorPersistenceInfoMSFT persist_info = { XR_TYPE_SPATIAL_ANCHOR_PERSISTENCE_INFO_MSFT };
		strncpy(persist_info.spatialAnchorPersistenceName.name, anchor->name, sizeof(persist_info.spatialAnchorPersistenceName.name));
		persist_info.spatialAnchor = anchor_data->anchor;
		XrResult result = xr_extensions.xrPersistSpatialAnchorMSFT(oxr_msft_anchor_sys.store, &persist_info);
		if (XR_FAILED(result)) {
			log_warnf("xrPersistSpatialAnchorMSFT failed: %s", openxr_string(result));
			return false;
		}
	} else {
		XrSpatialAnchorPersistenceNameMSFT name_info = { };
		strncpy(name_info.name, anchor->name, sizeof(name_info.name));
		XrResult result = xr_extensions.xrUnpersistSpatialAnchorMSFT(oxr_msft_anchor_sys.store, &name_info);
		if (XR_FAILED(result)) {
			log_warnf("xrUnpersistSpatialAnchorMSFT failed: %s", openxr_string(result));
			return false;
		}
	}
	anchor->persisted = persist;
	return true;
}

///////////////////////////////////////////

bool32_t anchor_oxr_get_perception_anchor(anchor_t anchor, void **perception_spatial_anchor) {
#if defined(SK_OS_WINDOWS_UWP)
	if (xr_ext.MSFT_perception_anchor_interop != xr_ext_active) return false;
	oxr_msft_world_anchor_t* anchor_data = (oxr_msft_world_anchor_t*)anchor->data;
	XrResult result = xr_extensions.xrTryGetPerceptionAnchorFromSpatialAnchorMSFT(xr_session, anchor_data->anchor, (IUnknown**)perception_spatial_anchor);
	if (XR_FAILED(result)) {
		log_warnf("xrTryGetPerceptionAnchorFromSpatialAnchorMSFT failed: %s", openxr_string(result));
		return false;
	}
	return true;
#else
	log_warn("anchor_oxr_get_perception_anchor not available outside of Windows UWP!");
	return false;
#endif
}

} // namespace sk
#endif