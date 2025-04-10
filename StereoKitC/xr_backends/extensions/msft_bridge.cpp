/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#include "../openxr_platform.h"
#include "ext_management.h"
#include "msft_bridge.h"

#ifdef XR_USE_PLATFORM_WIN32

#include "../../_stereokit.h"
#include <string.h>

///////////////////////////////////////////

#define XR_EXT_FUNCTIONS( X )                    \
	X(xrCreateSpatialGraphNodeSpaceMSFT        ) \
	X(xrConvertWin32PerformanceCounterToTimeKHR)
OPENXR_DEFINE_FN_STATIC(XR_EXT_FUNCTIONS);

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

xr_system_ xr_ext_msft_bridge_init(void*);

///////////////////////////////////////////

void xr_ext_msft_bridge_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_MSFT_SPATIAL_GRAPH_BRIDGE_EXTENSION_NAME;
	sys.request_exts[sys.request_ext_count++] = XR_KHR_WIN32_CONVERT_PERFORMANCE_COUNTER_TIME_EXTENSION_NAME;
	sys.func_initialize = { xr_ext_msft_bridge_init };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_msft_bridge_init(void*) {
	// Check if we got our extension
	if (!backend_openxr_ext_enabled(XR_MSFT_SPATIAL_GRAPH_BRIDGE_EXTENSION_NAME) ||
		!backend_openxr_ext_enabled(XR_KHR_WIN32_CONVERT_PERFORMANCE_COUNTER_TIME_EXTENSION_NAME))
		return xr_system_fail;

	// Load all extension functions
	OPENXR_LOAD_FN(XR_EXT_FUNCTIONS, xr_system_fail);

	sk_get_info_ref()->spatial_bridge_present = true;
	return xr_system_succeed;
}

///////////////////////////////////////////

pose_t world_from_spatial_graph(uint8_t spatial_graph_node_id[16], bool32_t dynamic, int64_t qpc_time) {
	if (!xr_session) {
		log_warn("No OpenXR session available for converting spatial graph nodes!");
		return pose_identity;
	}
	if (!sk_get_info_ref()->spatial_bridge_present) {
		log_warn("This system doesn't support the spatial bridge! Check SK.System.spatialBridgePresent.");
		return pose_identity;
	}

	pose_t result = pose_identity;
	if (!world_try_from_spatial_graph(spatial_graph_node_id, dynamic, qpc_time, &result))
		log_warn("world_from_spatial_graph call failed, maybe a bad spatial node?");
	return result;
}

///////////////////////////////////////////

bool32_t world_try_from_spatial_graph(uint8_t spatial_graph_node_id[16], bool32_t dynamic, int64_t qpc_time, pose_t *out_pose) {
	if (!xr_session || !sk_get_info_ref()->spatial_bridge_present)
		return false;

	XrSpace                               space;
	XrSpatialGraphNodeSpaceCreateInfoMSFT space_info = { XR_TYPE_SPATIAL_GRAPH_NODE_SPACE_CREATE_INFO_MSFT };
	space_info.nodeType = dynamic ? XR_SPATIAL_GRAPH_NODE_TYPE_DYNAMIC_MSFT : XR_SPATIAL_GRAPH_NODE_TYPE_STATIC_MSFT;
	space_info.pose     = { {0,0,0,1}, {0,0,0} };
	memcpy(space_info.nodeId, spatial_graph_node_id, sizeof(space_info.nodeId));

	if (XR_FAILED(xrCreateSpatialGraphNodeSpaceMSFT(xr_session, &space_info, &space)))
		return false;

	XrTime time = 0;
	if (qpc_time > 0) {
		LARGE_INTEGER li;
		li.QuadPart = qpc_time;
		xrConvertWin32PerformanceCounterToTimeKHR(xr_instance, &li, &time);
	}
	return openxr_get_space(space, out_pose, time);
}

///////////////////////////////////////////

} // namespace sk

#else

namespace sk {

///////////////////////////////////////////

void xr_ext_msft_bridge_register() { }

///////////////////////////////////////////

pose_t world_from_spatial_graph(uint8_t spatial_graph_node_id[16], bool32_t dynamic, int64_t qpc_time) {
	log_warn("world_from_spatial_graph call failed, invalid platform.");
	return pose_identity;
}

///////////////////////////////////////////

bool32_t world_try_from_spatial_graph(uint8_t spatial_graph_node_id[16], bool32_t dynamic, int64_t qpc_time, pose_t* out_pose) {
	*out_pose = pose_identity;
	return false;
}

///////////////////////////////////////////

} // namespace sk

#endif