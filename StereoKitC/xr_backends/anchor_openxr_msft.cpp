#include "../platforms/platform_utils.h"
#if defined(SK_XR_OPENXR)

#include "anchor_openxr_msft.h"
#include "openxr_extensions.h"
#include "../asset_types/anchor.h"

#include <stdio.h>

namespace sk {

typedef struct oxr_msft_world_anchor_sys_t {
	anchor_type_id id;
	XrSpatialAnchorStoreConnectionMSFT store;
} oxr_msft_world_anchor_sys_t;

typedef struct oxr_msft_world_anchor_t {
	XrSpatialAnchorMSFT anchor;
	XrSpace             space;
} oxr_msft_world_anchor_t;

oxr_msft_world_anchor_sys_t oxr_msft_anchor_sys = {};

///////////////////////////////////////////

bool32_t anchor_oxr_msft_init() {
	if (!xr_ext_available.MSFT_spatial_anchor) return false;

	oxr_msft_anchor_sys = {};

	// Check on the anchor store for any persisted anchors
	if (!xr_ext_available.MSFT_spatial_anchor_persistence) return true;

	XrResult result = xr_extensions.xrCreateSpatialAnchorStoreConnectionMSFT(xr_session, &oxr_msft_anchor_sys.store);
	if (XR_FAILED(result)) {
		log_warnf("xrCreateSpatialAnchorStoreConnectionMSFT failed: %s", openxr_string(result));
		return false;
	}

	uint32_t count = 0;
	xr_extensions.xrEnumeratePersistedSpatialAnchorNamesMSFT(oxr_msft_anchor_sys.store, 0, &count, nullptr);
	if (count == 0) return false;

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
		pose_t pose = pose_identity;
		openxr_get_space(space, &pose);

		// Create a StereoKit anchor
		oxr_msft_world_anchor_t* anchor_data = sk_malloc_t(oxr_msft_world_anchor_t, 1);
		anchor_data->anchor = anchor;
		anchor_data->space  = space;
		anchor_t sk_anchor = anchor_create_manual(oxr_msft_anchor_sys.id, pose, names[i].name, (void*)anchor_data);
		sk_anchor->persisted = true;
		anchor_notify_discovery(sk_anchor);
	}
	return true;
}

///////////////////////////////////////////

void anchor_oxr_msft_shutdown() {
	if (oxr_msft_anchor_sys.store != XR_NULL_HANDLE)
		xr_extensions.xrDestroySpatialAnchorStoreConnectionMSFT(oxr_msft_anchor_sys.store);
}

///////////////////////////////////////////

anchor_props_ anchor_oxr_msft_properties() {
	anchor_props_ result = {};
	if (xr_ext_available.MSFT_spatial_anchor            ) result |= anchor_props_stability;
	if (xr_ext_available.MSFT_spatial_anchor_persistence) result |= anchor_props_storable;
	return result;
}

///////////////////////////////////////////

void anchor_oxr_msft_clear_stored() {
	if (xr_ext_available.MSFT_spatial_anchor_persistence)
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

	// Create a StereoKit anchor
	oxr_msft_world_anchor_t* anchor_data = sk_malloc_t(oxr_msft_world_anchor_t, 1);
	anchor_data->anchor = anchor;
	anchor_data->space  = space;
	anchor_t sk_anchor = anchor_create_manual(oxr_msft_anchor_sys.id, pose, name_utf8, (void*)anchor_data);
	anchor_notify_discovery(sk_anchor);
	return sk_anchor;
}

///////////////////////////////////////////

void anchor_oxr_msft_destroy(anchor_t anchor) {
	oxr_msft_world_anchor_t* data = (oxr_msft_world_anchor_t*)anchor->data;
	xrDestroySpace(data->space);
	xr_extensions.xrDestroySpatialAnchorMSFT(data->anchor);
	free(data);
}

///////////////////////////////////////////

bool32_t anchor_oxr_msft_persist(anchor_t anchor, bool32_t persist) {
	if (anchor->persisted == persist || !xr_ext_available.MSFT_spatial_anchor_persistence) return anchor->persisted == persist;

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

} // namespace sk
#endif