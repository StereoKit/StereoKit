/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

 // This implements XR_EXT_spatial_entity
 // https://registry.khronos.org/OpenXR/specs/1.1/html/xrspec.html#XR_EXT_spatial_entity

#include "ext_management.h"
#include "future.h"
#include "../openxr.h"

#define XR_EXT_FUNCTIONS( X )                        \
	X(xrEnumerateSpatialCapabilitiesEXT)             \
	X(xrEnumerateSpatialCapabilityComponentTypesEXT) \
	X(xrEnumerateSpatialCapabilityFeaturesEXT)       \
	X(xrCreateSpatialContextAsyncEXT)                \
	X(xrCreateSpatialContextCompleteEXT)             \
	X(xrDestroySpatialContextEXT)                    \
	X(xrCreateSpatialDiscoverySnapshotAsyncEXT)      \
	X(xrCreateSpatialDiscoverySnapshotCompleteEXT)   \
	X(xrQuerySpatialComponentDataEXT)                \
	X(xrDestroySpatialSnapshotEXT)                   \
	X(xrCreateSpatialEntityFromIdEXT)                \
	X(xrDestroySpatialEntityEXT)                     \
	X(xrCreateSpatialUpdateSnapshotEXT)              \
	X(xrGetSpatialBufferStringEXT)                   \
	X(xrGetSpatialBufferUint8EXT)                    \
	X(xrGetSpatialBufferUint16EXT)                   \
	X(xrGetSpatialBufferUint32EXT)                   \
	X(xrGetSpatialBufferFloatEXT)                    \
	X(xrGetSpatialBufferVector2fEXT)                 \
	X(xrGetSpatialBufferVector3fEXT)
OPENXR_DEFINE_FN_STATIC(XR_EXT_FUNCTIONS);

struct xr_spatial_entity_state_t {
	bool available;
	bool ready;
	XrSpatialContextEXT spatial_context;
};
static xr_spatial_entity_state_t local = { };

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

xr_system_ xr_ext_spatial_entity_initialize(void*);
void       xr_ext_spatial_entity_shutdown  (void*);
void       xr_ext_spatial_entity_step_begin(void*);
void       xr_ext_spatial_entity_event_poll(void* context, XrEventDataBuffer *event_data);

///////////////////////////////////////////

void xr_ext_spatial_entity_register() {
	xr_system_t sys = {};
	//sys.request_exts[sys.request_ext_count++] = XR_EXT_SPATIAL_ENTITY_EXTENSION_NAME;
	sys.request_exts[sys.request_ext_count++] = XR_EXT_SPATIAL_ENTITY_EXTENSION_NAME;
	sys.request_exts[sys.request_ext_count++] = XR_EXT_SPATIAL_ANCHOR_EXTENSION_NAME;
	sys.request_exts[sys.request_ext_count++] = XR_EXT_SPATIAL_PERSISTENCE_EXTENSION_NAME;
	sys.request_exts[sys.request_ext_count++] = XR_EXT_SPATIAL_PERSISTENCE_OPERATIONS_EXTENSION_NAME;
	//sys.request_exts[sys.request_ext_count++] = XR_EXT_SPATIAL_PLANE_TRACKING_EXTENSION_NAME;
	sys.evt_initialize = { xr_ext_spatial_entity_initialize };
	sys.evt_shutdown   = { xr_ext_spatial_entity_shutdown   };
	sys.evt_step_begin = { xr_ext_spatial_entity_step_begin };
	sys.evt_poll       = { (void(*)(void*, void*))xr_ext_spatial_entity_event_poll };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_spatial_entity_initialize(void*) {
	// Check if we got our extension
	if (!backend_openxr_ext_enabled(XR_EXT_SPATIAL_ENTITY_EXTENSION_NAME))
		return xr_system_fail;

	// Load all extension functions
	OPENXR_LOAD_FN_RETURN(XR_EXT_FUNCTIONS, xr_system_fail);

	// Check our capabilities
	uint32_t capability_count = 0;
	XrResult result = xrEnumerateSpatialCapabilitiesEXT(xr_instance, xr_system_id, 0, &capability_count, nullptr);
	if (XR_FAILED(result)) log_warnf("%s [%s]", "xrEnumerateSpatialCapabilitiesEXT", openxr_string(result));
	XrSpatialCapabilityEXT* capabilities = sk_malloc_t(XrSpatialCapabilityEXT, capability_count);
	result = xrEnumerateSpatialCapabilitiesEXT(xr_instance, xr_system_id, capability_count, &capability_count, capabilities);
	if (XR_FAILED(result)) log_warnf("%s [%s]", "xrEnumerateSpatialCapabilitiesEXT", openxr_string(result));

	log_info("Spatial Entity Capabilities:");
	for (int32_t i = 0; i < capability_count; i++) {
		switch (capabilities[i]) {
		case XR_SPATIAL_CAPABILITY_ANCHOR_EXT:                        log_info("\tANCHOR");        break;
		case XR_SPATIAL_CAPABILITY_MARKER_TRACKING_APRIL_TAG_EXT:     log_info("\tAPRIL TAG");     break;
		case XR_SPATIAL_CAPABILITY_MARKER_TRACKING_ARUCO_MARKER_EXT:  log_info("\tARUCO MARKER");  break;
		case XR_SPATIAL_CAPABILITY_MARKER_TRACKING_MICRO_QR_CODE_EXT: log_info("\tMICRO QR CODE"); break;
		case XR_SPATIAL_CAPABILITY_MARKER_TRACKING_QR_CODE_EXT:       log_info("\tQR CODE");       break;
		case XR_SPATIAL_CAPABILITY_PLANE_TRACKING_EXT:                log_info("\tPLANE TRACKING"); break;
		default: log_infof("\t%d", capabilities[i]);
		}

		XrSpatialCapabilityComponentTypesEXT capability_components = { XR_TYPE_SPATIAL_CAPABILITY_COMPONENT_TYPES_EXT };
		xrEnumerateSpatialCapabilityComponentTypesEXT(xr_instance, xr_system_id, capabilities[i], &capability_components);
		capability_components.componentTypes = sk_malloc_t(XrSpatialComponentTypeEXT, capability_components.componentTypeCapacityInput);
		xrEnumerateSpatialCapabilityComponentTypesEXT(xr_instance, xr_system_id, capabilities[i], &capability_components);
		if (XR_FAILED(result)) log_warnf("%s [%s]", "xrEnumerateSpatialCapabilityComponentTypesEXT", openxr_string(result));

		for (int32_t c = 0; c < capability_components.componentTypeCountOutput; c++) {
			switch (capability_components.componentTypes[c]) {
			case XR_SPATIAL_COMPONENT_TYPE_BOUNDED_2D_EXT:          log_info("\t\tcomponent - BOUNDED 2D");           break;
			case XR_SPATIAL_COMPONENT_TYPE_BOUNDED_3D_EXT:          log_info("\t\tcomponent - BOUNDED 3D");           break;
			case XR_SPATIAL_COMPONENT_TYPE_PARENT_EXT:              log_info("\t\tcomponent - PARENT"    );           break;
			case XR_SPATIAL_COMPONENT_TYPE_MESH_3D_EXT:             log_info("\t\tcomponent - MESH 3D");              break;
			case XR_SPATIAL_COMPONENT_TYPE_ANCHOR_EXT:              log_info("\t\tcomponent - ANCHOR");               break;
			case XR_SPATIAL_COMPONENT_TYPE_PERSISTENCE_EXT:         log_info("\t\tcomponent - PERSISTENCE");          break;
			case XR_SPATIAL_COMPONENT_TYPE_PLANE_ALIGNMENT_EXT:     log_info("\t\tcomponent - PLANE ALIGNMENT");      break;
			case XR_SPATIAL_COMPONENT_TYPE_MESH_2D_EXT:             log_info("\t\tcomponent - MESH 2D");              break;
			case XR_SPATIAL_COMPONENT_TYPE_POLYGON_2D_EXT:          log_info("\t\tcomponent - POLYGON 2D");           break;
			case XR_SPATIAL_COMPONENT_TYPE_PLANE_SEMANTIC_LABEL_EXT:log_info("\t\tcomponent - PLANE SEMANTIC LABEL"); break;
			default: log_infof("\t\tcomponent - %d", capability_components.componentTypes[c]); break;
			}
		}
		sk_free(capability_components.componentTypes);

		uint32_t feature_count = 0;
		result = xrEnumerateSpatialCapabilityFeaturesEXT(xr_instance, xr_system_id, capabilities[i], 0, &feature_count, nullptr);
		if (XR_FAILED(result)) log_warnf("%s [%s]", openxr_string(result));
		XrSpatialCapabilityFeatureEXT* capability_features = sk_malloc_t(XrSpatialCapabilityFeatureEXT, feature_count);
		result = xrEnumerateSpatialCapabilityFeaturesEXT(xr_instance, xr_system_id, capabilities[i], feature_count, &feature_count, capability_features);
		if (XR_FAILED(result)) log_warnf("%s [%s]", openxr_string(result));

		for (int32_t f = 0; f < feature_count; f++) {
			switch (capability_features[f]) {
			case XR_SPATIAL_CAPABILITY_FEATURE_MARKER_TRACKING_FIXED_SIZE_MARKERS_EXT: log_info("\t\tfeature - FIXED SIZE MARKERS"); break;
			case XR_SPATIAL_CAPABILITY_FEATURE_MARKER_TRACKING_STATIC_MARKERS_EXT:     log_info("\t\tfeature - STATIC MARKERS");     break;
			default: log_infof("\t\tfeature - %d", capability_features[f]); break;
			}
		}

		sk_free(capability_features);
	}
	sk_free(capabilities);

	// Create a spatial context
	XrFutureEXT                   future = XR_NULL_FUTURE_EXT;
	XrSpatialContextCreateInfoEXT info   = { XR_TYPE_SPATIAL_CONTEXT_CREATE_INFO_EXT };
	info.capabilityConfigs = nullptr;

	result = xrCreateSpatialContextAsyncEXT(xr_session, &info, &future);
	if (XR_FAILED(result)) {
		log_warnf("%s [%s]", "xrCreateSpatialContextAsyncEXT", openxr_string(result));
		return xr_system_fail;
	}

	// Async callback for spatial context creation
	xr_ext_future_on_finish(future, [](void*, XrFutureEXT future) {
		XrCreateSpatialContextCompletionEXT completion = { XR_TYPE_CREATE_SPATIAL_CONTEXT_COMPLETION_EXT };
		XrResult result = xrCreateSpatialContextCompleteEXT(xr_session, future, &completion);
		if (XR_FAILED(result)) {
			log_warnf("%s [%s]", "xrCreateSpatialContextCompleteEXT", openxr_string(result));
			return;
		}
		if (XR_FAILED(completion.futureResult)) {
			log_warnf("%s async [%s]", "xrCreateSpatialContextAsyncEXT", openxr_string(completion.futureResult));
			return;
		}
		log_info("Got spatial context!");
		local.spatial_context = completion.spatialContext;
		local.ready           = true;
	}, nullptr);

	local.available = true;
	return xr_system_succeed;
}

///////////////////////////////////////////

void xr_ext_spatial_entity_shutdown(void*) {
	xrDestroySpatialContextEXT(local.spatial_context);

	local = {};

	OPENXR_CLEAR_FN(XR_EXT_FUNCTIONS);
}

///////////////////////////////////////////

void xr_ext_spatial_entity_begin_snapshot(XrSpatialContextEXT spatial_context) {
	XrSpatialDiscoverySnapshotCreateInfoEXT info = { XR_TYPE_SPATIAL_DISCOVERY_SNAPSHOT_CREATE_INFO_EXT };
	info.componentTypes;

	XrFutureEXT future = XR_NULL_FUTURE_EXT;
	xrCreateSpatialDiscoverySnapshotAsyncEXT(spatial_context, &info, &future);

	xr_ext_future_on_finish(future, [](void* spatial_context, XrFutureEXT future) {
		// Complete the async call
		XrCreateSpatialDiscoverySnapshotCompletionEXT     completion      = { XR_TYPE_CREATE_SPATIAL_DISCOVERY_SNAPSHOT_COMPLETION_EXT      };
		XrCreateSpatialDiscoverySnapshotCompletionInfoEXT completion_info = { XR_TYPE_CREATE_SPATIAL_DISCOVERY_SNAPSHOT_COMPLETION_INFO_EXT };
		completion_info.baseSpace = xr_app_space;
		completion_info.time      = xr_time;
		completion_info.future    = future;
		xrCreateSpatialDiscoverySnapshotCompleteEXT((XrSpatialContextEXT)spatial_context, &completion_info, &completion);
		if (completion.futureResult != XR_SUCCESS) {
			log_warnf("%s async [%s]", "xrCreateSpatialDiscoverySnapshotAsyncEXT", openxr_string(completion.futureResult));
			return;
		}

		// Get a snapshot of the data for this spatial context, two call idiom
		XrSpatialComponentTypeEXT components[] = { XR_SPATIAL_COMPONENT_TYPE_BOUNDED_2D_EXT };
		XrSpatialComponentDataQueryResultEXT    query_result    = { XR_TYPE_SPATIAL_COMPONENT_DATA_QUERY_RESULT_EXT };
		XrSpatialComponentDataQueryConditionEXT query_condition = { XR_TYPE_SPATIAL_COMPONENT_DATA_QUERY_CONDITION_EXT };
		query_condition.componentTypes     = components;
		query_condition.componentTypeCount = sizeof(components) / sizeof(components[0]);
		xrQuerySpatialComponentDataEXT(completion.snapshot, &query_condition, &query_result);
		
		query_result.entityIds                = sk_malloc_t(XrSpatialEntityIdEXT,            query_result.entityIdCountOutput);
		query_result.entityIdCapacityInput    = query_result.entityIdCountOutput;
		query_result.entityStates             = sk_malloc_t(XrSpatialEntityTrackingStateEXT, query_result.entityStateCountOutput);
		query_result.entityStateCapacityInput = query_result.entityStateCountOutput;
		XrSpatialComponentBounded2DListEXT bounds_list = { XR_TYPE_SPATIAL_COMPONENT_BOUNDED_2D_LIST_EXT };
		bounds_list.boundCount = query_result.entityIdCountOutput;
		bounds_list.bounds     = sk_malloc_t(XrSpatialBounded2DDataEXT, query_result.entityIdCountOutput);
		xr_insert_next((XrBaseHeader*)&query_result, (XrBaseHeader*)&bounds_list);
		xrQuerySpatialComponentDataEXT(completion.snapshot, &query_condition, &query_result);

		// Free up resources
		xrDestroySpatialSnapshotEXT(completion.snapshot);
		sk_free(query_result.entityIds   );
		sk_free(query_result.entityStates);
		sk_free(bounds_list .bounds      );
	}, spatial_context);
}

///////////////////////////////////////////

void xr_ext_spatial_entity_event_poll(void* context, XrEventDataBuffer* event_data) {
	if (event_data->type != XR_TYPE_EVENT_DATA_SPATIAL_DISCOVERY_RECOMMENDED_EXT)
		return;

	XrEventDataSpatialDiscoveryRecommendedEXT* recommendation = (XrEventDataSpatialDiscoveryRecommendedEXT*)event_data;
	xr_ext_spatial_entity_begin_snapshot(recommendation->spatialContext);
}

///////////////////////////////////////////

void xr_ext_spatial_entity_step_begin(void*) {
	if (!local.ready)
		return;

}

}