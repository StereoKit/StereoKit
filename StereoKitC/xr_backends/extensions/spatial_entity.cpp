/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

// This implements XR_EXT_spatial_entity
// https://registry.khronos.org/OpenXR/specs/1.1/html/xrspec.html#XR_EXT_spatial_entity

#include "spatial_entity.h"
#include "ext_management.h"
#include "future.h"
#include "../openxr.h"

#include <string.h>

///////////////////////////////////////////
// OpenXR function pointers              //
///////////////////////////////////////////

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

#define XR_ANCHOR_FUNCTIONS( X ) \
	X(xrCreateSpatialAnchorEXT)
OPENXR_DEFINE_FN_STATIC(XR_ANCHOR_FUNCTIONS);

namespace sk {

///////////////////////////////////////////
// Mapping tables                        //
///////////////////////////////////////////

struct cap_mapping_t {
	spatial_capability_    sk_bit;
	XrSpatialCapabilityEXT xr_value;
	XrStructureType        config_type;
	size_t                 config_size;
	const char*            ext_name;
};

static const cap_mapping_t cap_mappings[] = {
	{ spatial_capability_anchor,         XR_SPATIAL_CAPABILITY_ANCHOR_EXT,                        XR_TYPE_SPATIAL_CAPABILITY_CONFIGURATION_ANCHOR_EXT,         sizeof(XrSpatialCapabilityConfigurationAnchorEXT),        XR_EXT_SPATIAL_ANCHOR_EXTENSION_NAME         },
	{ spatial_capability_plane_tracking, XR_SPATIAL_CAPABILITY_PLANE_TRACKING_EXT,                XR_TYPE_SPATIAL_CAPABILITY_CONFIGURATION_PLANE_TRACKING_EXT, sizeof(XrSpatialCapabilityConfigurationPlaneTrackingEXT), XR_EXT_SPATIAL_PLANE_TRACKING_EXTENSION_NAME },
	{ spatial_capability_qr_code,        XR_SPATIAL_CAPABILITY_MARKER_TRACKING_QR_CODE_EXT,       XR_TYPE_SPATIAL_CAPABILITY_CONFIGURATION_QR_CODE_EXT,        sizeof(XrSpatialCapabilityConfigurationQrCodeEXT),        XR_EXT_SPATIAL_MARKER_TRACKING_EXTENSION_NAME},
	{ spatial_capability_micro_qr,       XR_SPATIAL_CAPABILITY_MARKER_TRACKING_MICRO_QR_CODE_EXT, XR_TYPE_SPATIAL_CAPABILITY_CONFIGURATION_MICRO_QR_CODE_EXT,  sizeof(XrSpatialCapabilityConfigurationMicroQrCodeEXT),   XR_EXT_SPATIAL_MARKER_TRACKING_EXTENSION_NAME},
	{ spatial_capability_aruco,          XR_SPATIAL_CAPABILITY_MARKER_TRACKING_ARUCO_MARKER_EXT,  XR_TYPE_SPATIAL_CAPABILITY_CONFIGURATION_ARUCO_MARKER_EXT,   sizeof(XrSpatialCapabilityConfigurationArucoMarkerEXT),   XR_EXT_SPATIAL_MARKER_TRACKING_EXTENSION_NAME},
	{ spatial_capability_april_tag,      XR_SPATIAL_CAPABILITY_MARKER_TRACKING_APRIL_TAG_EXT,     XR_TYPE_SPATIAL_CAPABILITY_CONFIGURATION_APRIL_TAG_EXT,      sizeof(XrSpatialCapabilityConfigurationAprilTagEXT),      XR_EXT_SPATIAL_MARKER_TRACKING_EXTENSION_NAME},
};
static const int32_t cap_mapping_count = sizeof(cap_mappings) / sizeof(cap_mappings[0]);

struct comp_mapping_t {
	spatial_component_         sk_bit;
	XrSpatialComponentTypeEXT  xr_value;
};

static const comp_mapping_t comp_mappings[] = {
	{ spatial_component_bounded_2d,      XR_SPATIAL_COMPONENT_TYPE_BOUNDED_2D_EXT          },
	{ spatial_component_bounded_3d,      XR_SPATIAL_COMPONENT_TYPE_BOUNDED_3D_EXT          },
	{ spatial_component_parent,          XR_SPATIAL_COMPONENT_TYPE_PARENT_EXT              },
	{ spatial_component_mesh_3d,         XR_SPATIAL_COMPONENT_TYPE_MESH_3D_EXT             },
	{ spatial_component_anchor,          XR_SPATIAL_COMPONENT_TYPE_ANCHOR_EXT              },
	{ spatial_component_persistence,     XR_SPATIAL_COMPONENT_TYPE_PERSISTENCE_EXT         },
	{ spatial_component_plane_alignment, XR_SPATIAL_COMPONENT_TYPE_PLANE_ALIGNMENT_EXT     },
	{ spatial_component_mesh_2d,         XR_SPATIAL_COMPONENT_TYPE_MESH_2D_EXT             },
	{ spatial_component_polygon_2d,      XR_SPATIAL_COMPONENT_TYPE_POLYGON_2D_EXT          },
	{ spatial_component_plane_label,     XR_SPATIAL_COMPONENT_TYPE_PLANE_SEMANTIC_LABEL_EXT},
	{ spatial_component_marker,          XR_SPATIAL_COMPONENT_TYPE_MARKER_EXT              },
};
static const int32_t comp_mapping_count = sizeof(comp_mappings) / sizeof(comp_mappings[0]);

///////////////////////////////////////////
// State                                 //
///////////////////////////////////////////

struct cap_support_t {
	XrSpatialCapabilityEXT xr_cap;
	spatial_capability_    sk_cap;
	spatial_component_     supported_components;
};

struct xr_spatial_entity_state_t {
	bool                available;
	bool                ready;
	bool                anchor_ext;
	bool                plane_ext;
	bool                marker_ext;

	spatial_capability_  supported_caps;
	array_t<cap_support_t> cap_supports;

	XrSpatialContextEXT spatial_context;
	spatial_capability_  active_caps;
	spatial_component_   active_comps;

	spatial_on_discover_t on_discover;
	void*                 on_discover_ctx;

	bool                  pending_permission;
	spatial_capability_   pending_caps;
	spatial_component_    pending_comps;
	spatial_on_discover_t pending_on_discover;
	void*                 pending_on_discover_ctx;

	array_t<XrSpatialEntityEXT> anchor_entities;
};
static xr_spatial_entity_state_t local = {};

///////////////////////////////////////////
// Helpers                               //
///////////////////////////////////////////

static inline pose_t xr_to_pose(const XrPosef& xr_pose) {
	pose_t result;
	result.position    = { xr_pose.position.x,    xr_pose.position.y,    xr_pose.position.z    };
	result.orientation = { xr_pose.orientation.x, xr_pose.orientation.y, xr_pose.orientation.z, xr_pose.orientation.w };
	return result;
}

static inline XrPosef pose_to_xr(pose_t pose) {
	XrPosef result;
	result.position    = { pose.position.x,    pose.position.y,    pose.position.z    };
	result.orientation = { pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w };
	return result;
}

static spatial_capability_ xr_to_sk_cap(XrSpatialCapabilityEXT xr_cap) {
	for (int32_t i = 0; i < cap_mapping_count; i++) {
		if (cap_mappings[i].xr_value == xr_cap)
			return cap_mappings[i].sk_bit;
	}
	return spatial_capability_none;
}

static spatial_component_ xr_to_sk_comp(XrSpatialComponentTypeEXT xr_comp) {
	for (int32_t i = 0; i < comp_mapping_count; i++) {
		if (comp_mappings[i].xr_value == xr_comp)
			return comp_mappings[i].sk_bit;
	}
	return spatial_component_none;
}

// Convert a spatial_component_ bitmask to an array of XrSpatialComponentTypeEXT.
// Returns the count written into out_types (which must have room for
// comp_mapping_count entries).
static int32_t sk_comps_to_xr(spatial_component_ mask, XrSpatialComponentTypeEXT* out_types) {
	int32_t count = 0;
	for (int32_t i = 0; i < comp_mapping_count; i++) {
		if ((int)mask & (int)comp_mappings[i].sk_bit)
			out_types[count++] = comp_mappings[i].xr_value;
	}
	return count;
}

static spatial_tracking_ xr_to_sk_tracking(XrSpatialEntityTrackingStateEXT xr_state) {
	switch (xr_state) {
	case XR_SPATIAL_ENTITY_TRACKING_STATE_TRACKING_EXT: return spatial_tracking_tracking;
	case XR_SPATIAL_ENTITY_TRACKING_STATE_PAUSED_EXT:   return spatial_tracking_paused;
	default:                                            return spatial_tracking_stopped;
	}
}

static spatial_marker_type_ xr_cap_to_marker_type(XrSpatialCapabilityEXT cap) {
	switch (cap) {
	case XR_SPATIAL_CAPABILITY_MARKER_TRACKING_QR_CODE_EXT:       return spatial_marker_type_qr_code;
	case XR_SPATIAL_CAPABILITY_MARKER_TRACKING_MICRO_QR_CODE_EXT: return spatial_marker_type_micro_qr;
	case XR_SPATIAL_CAPABILITY_MARKER_TRACKING_ARUCO_MARKER_EXT:  return spatial_marker_type_aruco;
	case XR_SPATIAL_CAPABILITY_MARKER_TRACKING_APRIL_TAG_EXT:     return spatial_marker_type_april_tag;
	default:                                                      return spatial_marker_type_qr_code;
	}
}

///////////////////////////////////////////
// Forward declarations                  //
///////////////////////////////////////////

static xr_system_ xr_ext_spatial_entity_initialize(void*);
static void       xr_ext_spatial_entity_shutdown  (void*);
static void       xr_ext_spatial_entity_step_begin(void*);
static void       xr_ext_spatial_entity_event_poll(void* context, XrEventDataBuffer* event_data);

static void       spatial_entity_begin_discovery   ();
static bool       build_snapshot_from_xr           (XrSpatialSnapshotEXT xr_snapshot, spatial_component_ comps, spatial_snapshot_t* out);
static void       query_single_component           (XrSpatialSnapshotEXT xr_snapshot, spatial_component_ comp_bit, XrSpatialComponentTypeEXT xr_comp, int32_t entity_count, const XrSpatialEntityIdEXT* all_ids, spatial_snapshot_t* snapshot);

///////////////////////////////////////////
// Registration                          //
///////////////////////////////////////////

void xr_ext_spatial_entity_register() {
	xr_system_t sys = {};
	sys.request_exts    [sys.request_ext_count    ++] = XR_EXT_SPATIAL_ENTITY_EXTENSION_NAME;
	sys.request_opt_exts[sys.request_opt_ext_count++] = XR_EXT_SPATIAL_ANCHOR_EXTENSION_NAME;
	sys.request_opt_exts[sys.request_opt_ext_count++] = XR_EXT_SPATIAL_PLANE_TRACKING_EXTENSION_NAME;
	sys.request_opt_exts[sys.request_opt_ext_count++] = XR_EXT_SPATIAL_MARKER_TRACKING_EXTENSION_NAME;
	sys.evt_initialize = { xr_ext_spatial_entity_initialize };
	sys.evt_shutdown   = { xr_ext_spatial_entity_shutdown   };
	sys.evt_step_begin = { xr_ext_spatial_entity_step_begin };
	sys.evt_poll       = { (void(*)(void*, void*))xr_ext_spatial_entity_event_poll };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////
// Initialization                        //
///////////////////////////////////////////

static xr_system_ xr_ext_spatial_entity_initialize(void*) {
	if (!backend_openxr_ext_enabled(XR_EXT_SPATIAL_ENTITY_EXTENSION_NAME))
		return xr_system_fail;

	OPENXR_LOAD_FN_RETURN(XR_EXT_FUNCTIONS, xr_system_fail);

	local.anchor_ext = backend_openxr_ext_enabled(XR_EXT_SPATIAL_ANCHOR_EXTENSION_NAME);
	local.plane_ext  = backend_openxr_ext_enabled(XR_EXT_SPATIAL_PLANE_TRACKING_EXTENSION_NAME);
	local.marker_ext = backend_openxr_ext_enabled(XR_EXT_SPATIAL_MARKER_TRACKING_EXTENSION_NAME);

	if (local.anchor_ext) {
		OPENXR_LOAD_FN_RETURN(XR_ANCHOR_FUNCTIONS, xr_system_fail);
	}

	// Enumerate supported capabilities
	uint32_t cap_count = 0;
	XrResult result = xrEnumerateSpatialCapabilitiesEXT(xr_instance, xr_system_id, 0, &cap_count, nullptr);
	if (XR_FAILED(result)) { log_warnf("%s [%s]", "xrEnumerateSpatialCapabilitiesEXT", openxr_string(result)); return xr_system_fail; }

	XrSpatialCapabilityEXT* xr_caps = sk_malloc_t(XrSpatialCapabilityEXT, cap_count);
	result = xrEnumerateSpatialCapabilitiesEXT(xr_instance, xr_system_id, cap_count, &cap_count, xr_caps);
	if (XR_FAILED(result)) { log_warnf("%s [%s]", "xrEnumerateSpatialCapabilitiesEXT", openxr_string(result)); sk_free(xr_caps); return xr_system_fail; }

	log_diagf("Spatial entity: %u capabilities", cap_count);

	for (uint32_t i = 0; i < cap_count; i++) {
		spatial_capability_ sk_cap = xr_to_sk_cap(xr_caps[i]);
		if (sk_cap == spatial_capability_none) {
			log_diagf("  capability %d (xr value %d): unmapped, skipping", i, (int)xr_caps[i]);
			continue;
		}
		log_diagf("  capability %d (xr value %d): sk_bit=0x%x", i, (int)xr_caps[i], (int)sk_cap);

		// Enumerate components for this capability
		XrSpatialCapabilityComponentTypesEXT comp_types = { XR_TYPE_SPATIAL_CAPABILITY_COMPONENT_TYPES_EXT };
		xrEnumerateSpatialCapabilityComponentTypesEXT(xr_instance, xr_system_id, xr_caps[i], &comp_types);
		comp_types.componentTypeCapacityInput = comp_types.componentTypeCountOutput;
		comp_types.componentTypes             = sk_malloc_t(XrSpatialComponentTypeEXT, comp_types.componentTypeCountOutput);
		result = xrEnumerateSpatialCapabilityComponentTypesEXT(xr_instance, xr_system_id, xr_caps[i], &comp_types);
		if (XR_FAILED(result)) { sk_free(comp_types.componentTypes); continue; }

		spatial_component_ supported_comps = spatial_component_none;
		for (uint32_t c = 0; c < comp_types.componentTypeCountOutput; c++)
			supported_comps = (spatial_component_)((int)supported_comps | (int)xr_to_sk_comp(comp_types.componentTypes[c]));
		sk_free(comp_types.componentTypes);

		cap_support_t support = {};
		support.xr_cap               = xr_caps[i];
		support.sk_cap               = sk_cap;
		support.supported_components = supported_comps;
		local.cap_supports.add(support);

		local.supported_caps = (spatial_capability_)((int)local.supported_caps | (int)sk_cap);
	}
	sk_free(xr_caps);

	log_diagf("Spatial entity: supported_caps=0x%x, %d mapped", (int)local.supported_caps, local.cap_supports.count);
	log_diagf("  anchor ext: %s, plane ext: %s, marker ext: %s",
		local.anchor_ext ? "yes" : "no", local.plane_ext ? "yes" : "no", local.marker_ext ? "yes" : "no");

	local.available = true;

	// TODO: temporary debug - start discovery with everything and log results
	{
		spatial_component_ all_comps = spatial_component_none;
		for (int32_t i = 0; i < local.cap_supports.count; i++)
			all_comps = (spatial_component_)((int)all_comps | (int)local.cap_supports[i].supported_components);

		spatial_entity_start(local.supported_caps, all_comps, [](const spatial_snapshot_t* snapshot, void*) {
			log_infof("Spatial discovery: %d entities", snapshot->entity_count);
			for (int32_t i = 0; i < snapshot->entity_count; i++) {
				const spatial_entity_t* e = &snapshot->entities[i];
				log_infof("  [%d] id=%llu tracking=%d components=0x%x",
					i, (unsigned long long)e->id, e->tracking, (int)e->components);

				if ((int)e->components & (int)spatial_component_bounded_2d)
					log_infof("    bounded_2d: pos(%.2f,%.2f,%.2f) ext(%.2f,%.2f)",
						snapshot->bounds_2d[i].center.position.x, snapshot->bounds_2d[i].center.position.y, snapshot->bounds_2d[i].center.position.z,
						snapshot->bounds_2d[i].extents.x, snapshot->bounds_2d[i].extents.y);

				if ((int)e->components & (int)spatial_component_bounded_3d)
					log_infof("    bounded_3d: pos(%.2f,%.2f,%.2f) ext(%.2f,%.2f,%.2f)",
						snapshot->bounds_3d[i].center.position.x, snapshot->bounds_3d[i].center.position.y, snapshot->bounds_3d[i].center.position.z,
						snapshot->bounds_3d[i].extents.x, snapshot->bounds_3d[i].extents.y, snapshot->bounds_3d[i].extents.z);

				if ((int)e->components & (int)spatial_component_anchor)
					log_infof("    anchor: pos(%.2f,%.2f,%.2f)",
						snapshot->anchor_poses[i].position.x, snapshot->anchor_poses[i].position.y, snapshot->anchor_poses[i].position.z);

				if ((int)e->components & (int)spatial_component_plane_alignment)
					log_infof("    plane_align: %d", (int)snapshot->plane_alignments[i]);

				if ((int)e->components & (int)spatial_component_plane_label)
					log_infof("    plane_label: %d", (int)snapshot->plane_labels[i]);

				if ((int)e->components & (int)spatial_component_mesh_3d)
					log_infof("    mesh_3d: %d verts, %d indices",
						snapshot->meshes_3d[i].vertex_count, snapshot->meshes_3d[i].index_count);

				if ((int)e->components & (int)spatial_component_mesh_2d)
					log_infof("    mesh_2d: %d verts, %d indices",
						snapshot->meshes_2d[i].vertex_count, snapshot->meshes_2d[i].index_count);

				if ((int)e->components & (int)spatial_component_polygon_2d)
					log_infof("    polygon_2d: %d verts", snapshot->polygons_2d[i].vertex_count);

				if ((int)e->components & (int)spatial_component_marker)
					log_infof("    marker: type=%d id=%u data=%s",
						(int)snapshot->markers[i].type, snapshot->markers[i].marker_id,
						snapshot->markers[i].data ? snapshot->markers[i].data : "(null)");

				if ((int)e->components & (int)spatial_component_parent)
					log_infof("    parent: %llu", (unsigned long long)snapshot->parents[i]);
			}
			spatial_snapshot_release((spatial_snapshot_t*)snapshot);
		}, nullptr);
	}

	return xr_system_succeed;
}

///////////////////////////////////////////
// Public accessors                      //
///////////////////////////////////////////

bool spatial_entity_available() {
	return local.available;
}

bool spatial_entity_ready() {
	return local.ready;
}

spatial_capability_ spatial_entity_supported_capabilities() {
	return local.supported_caps;
}

spatial_component_ spatial_entity_supported_components(spatial_capability_ capability) {
	for (int32_t i = 0; i < local.cap_supports.count; i++) {
		if (local.cap_supports[i].sk_cap == capability)
			return local.cap_supports[i].supported_components;
	}
	return spatial_component_none;
}

///////////////////////////////////////////
// Lifecycle                             //
///////////////////////////////////////////

// Data that must survive until the context creation future completes.
struct start_context_t {
	uint8_t*                                             config_memory;
	XrSpatialComponentTypeEXT*                           comp_arrays;
	const XrSpatialCapabilityConfigurationBaseHeaderEXT** config_ptrs;
	int32_t                                              config_count;
};

static void spatial_entity_start_internal(spatial_capability_ capabilities, spatial_component_ components, spatial_on_discover_t on_discover, void* context);

void spatial_entity_start(spatial_capability_ capabilities, spatial_component_ components, spatial_on_discover_t on_discover, void* context) {
	if (!local.available) {
		log_warn("spatial_entity_start: extension not available");
		return;
	}

	// Scene permission is required for spatial entity access on most
	// platforms. Request it and defer the actual start until granted.
	permission_state_ perm = permission_state(permission_type_scene);
	if (perm == permission_state_capable) {
		permission_request(permission_type_scene);
		local.pending_permission     = true;
		local.pending_caps           = capabilities;
		local.pending_comps          = components;
		local.pending_on_discover    = on_discover;
		local.pending_on_discover_ctx = context;
		log_diag("spatial_entity_start: waiting for scene permission");
		return;
	}
	if (perm != permission_state_granted && perm != permission_state_unknown) {
		log_warn("spatial_entity_start: scene permission not granted");
		return;
	}

	spatial_entity_start_internal(capabilities, components, on_discover, context);
}

static void spatial_entity_start_internal(spatial_capability_ capabilities, spatial_component_ components, spatial_on_discover_t on_discover, void* context) {
	if (local.ready || local.spatial_context != XR_NULL_HANDLE)
		spatial_entity_stop();

	local.on_discover     = on_discover;
	local.on_discover_ctx = context;
	local.active_caps     = capabilities;
	local.active_comps    = components;

	// Build capability configs. We allocate everything in a start_context_t
	// that gets freed when the future completes.

	// First pass: count valid capabilities
	int32_t valid_count = 0;
	for (int32_t i = 0; i < cap_mapping_count; i++) {
		if (!((int)capabilities & (int)cap_mappings[i].sk_bit)) continue;
		if (!((int)local.supported_caps & (int)cap_mappings[i].sk_bit)) continue;
		if (cap_mappings[i].ext_name && !backend_openxr_ext_enabled(cap_mappings[i].ext_name)) continue;

		// Check that at least one requested component is supported
		spatial_component_ cap_comps = spatial_entity_supported_components(cap_mappings[i].sk_bit);
		spatial_component_ enabled   = (spatial_component_)((int)components & (int)cap_comps);
		if (enabled == spatial_component_none) continue;
		valid_count++;
	}

	if (valid_count == 0) {
		log_warn("spatial_entity_start: no valid capabilities to configure");
		return;
	}

	// Allocate start context
	start_context_t* start_ctx = sk_malloc_t(start_context_t, 1);
	memset(start_ctx, 0, sizeof(start_context_t));
	start_ctx->config_count = valid_count;
	start_ctx->config_ptrs  = sk_malloc_t(const XrSpatialCapabilityConfigurationBaseHeaderEXT*, valid_count);

	// Each capability config needs its own component type array
	// Worst case: comp_mapping_count entries per config
	start_ctx->comp_arrays = sk_malloc_t(XrSpatialComponentTypeEXT, valid_count * comp_mapping_count);

	// Allocate one block for all config structs (they vary in size)
	size_t total_config_size = 0;
	for (int32_t i = 0; i < cap_mapping_count; i++) {
		if (!((int)capabilities & (int)cap_mappings[i].sk_bit)) continue;
		if (!((int)local.supported_caps & (int)cap_mappings[i].sk_bit)) continue;
		if (cap_mappings[i].ext_name && !backend_openxr_ext_enabled(cap_mappings[i].ext_name)) continue;
		spatial_component_ cap_comps = spatial_entity_supported_components(cap_mappings[i].sk_bit);
		if (((int)components & (int)cap_comps) == 0) continue;
		total_config_size += cap_mappings[i].config_size;
	}
	start_ctx->config_memory = sk_malloc_t(uint8_t, total_config_size);
	memset(start_ctx->config_memory, 0, total_config_size);

	// Second pass: fill in configs
	int32_t  cfg_idx    = 0;
	uint8_t* cfg_cursor = start_ctx->config_memory;
	for (int32_t i = 0; i < cap_mapping_count; i++) {
		if (!((int)capabilities & (int)cap_mappings[i].sk_bit)) continue;
		if (!((int)local.supported_caps & (int)cap_mappings[i].sk_bit)) continue;
		if (cap_mappings[i].ext_name && !backend_openxr_ext_enabled(cap_mappings[i].ext_name)) continue;

		spatial_component_ cap_comps = spatial_entity_supported_components(cap_mappings[i].sk_bit);
		spatial_component_ enabled   = (spatial_component_)((int)components & (int)cap_comps);
		if (enabled == spatial_component_none) continue;

		// Build component type array for this config
		XrSpatialComponentTypeEXT* comp_arr = &start_ctx->comp_arrays[cfg_idx * comp_mapping_count];
		int32_t comp_count = sk_comps_to_xr(enabled, comp_arr);

		// Fill the config struct (all derive from base header)
		XrSpatialCapabilityConfigurationBaseHeaderEXT* cfg = (XrSpatialCapabilityConfigurationBaseHeaderEXT*)cfg_cursor;
		cfg->type                  = cap_mappings[i].config_type;
		cfg->next                  = nullptr;
		cfg->capability            = cap_mappings[i].xr_value;
		cfg->enabledComponentCount = comp_count;
		cfg->enabledComponents     = comp_arr;

		// ArUco has an extra field after the base header
		if (cap_mappings[i].sk_bit == spatial_capability_aruco) {
			XrSpatialCapabilityConfigurationArucoMarkerEXT* aruco = (XrSpatialCapabilityConfigurationArucoMarkerEXT*)cfg;
			aruco->arUcoDict = XR_SPATIAL_MARKER_ARUCO_DICT_4X4_50_EXT;
		}
		// AprilTag has an extra field too
		if (cap_mappings[i].sk_bit == spatial_capability_april_tag) {
			XrSpatialCapabilityConfigurationAprilTagEXT* april = (XrSpatialCapabilityConfigurationAprilTagEXT*)cfg;
			april->aprilDict = XR_SPATIAL_MARKER_APRIL_TAG_DICT_36H11_EXT;
		}

		start_ctx->config_ptrs[cfg_idx] = cfg;
		cfg_cursor += cap_mappings[i].config_size;
		cfg_idx++;
	}

	// Create spatial context
	XrSpatialContextCreateInfoEXT create_info = { XR_TYPE_SPATIAL_CONTEXT_CREATE_INFO_EXT };
	create_info.capabilityConfigCount = valid_count;
	create_info.capabilityConfigs     = start_ctx->config_ptrs;

	XrFutureEXT future = XR_NULL_FUTURE_EXT;
	XrResult result = xrCreateSpatialContextAsyncEXT(xr_session, &create_info, &future);
	if (XR_FAILED(result)) {
		log_warnf("%s [%s]", "xrCreateSpatialContextAsyncEXT", openxr_string(result));
		sk_free(start_ctx->config_memory);
		sk_free(start_ctx->comp_arrays);
		sk_free(start_ctx->config_ptrs);
		sk_free(start_ctx);
		return;
	}

	xr_ext_future_on_finish(future, [](void* ctx, XrFutureEXT future) {
		start_context_t* start_ctx = (start_context_t*)ctx;

		XrCreateSpatialContextCompletionEXT completion = { XR_TYPE_CREATE_SPATIAL_CONTEXT_COMPLETION_EXT };
		XrResult result = xrCreateSpatialContextCompleteEXT(xr_session, future, &completion);

		// Free config data regardless of result
		sk_free(start_ctx->config_memory);
		sk_free(start_ctx->comp_arrays);
		sk_free(start_ctx->config_ptrs);
		sk_free(start_ctx);

		if (XR_FAILED(result)) {
			log_warnf("%s [%s]", "xrCreateSpatialContextCompleteEXT", openxr_string(result));
			return;
		}
		if (XR_FAILED(completion.futureResult)) {
			log_warnf("%s async [%s]", "xrCreateSpatialContextAsyncEXT", openxr_string(completion.futureResult));
			return;
		}

		log_diag("Spatial entity context created");
		local.spatial_context = completion.spatialContext;
		local.ready           = true;
	}, start_ctx);
}

///////////////////////////////////////////

void spatial_entity_stop() {
	if (!local.available) return;

	for (int32_t i = 0; i < local.anchor_entities.count; i++)
		xrDestroySpatialEntityEXT(local.anchor_entities[i]);
	local.anchor_entities.clear();

	if (local.spatial_context != XR_NULL_HANDLE) {
		xrDestroySpatialContextEXT(local.spatial_context);
		local.spatial_context = XR_NULL_HANDLE;
	}

	local.ready              = false;
	local.active_caps        = spatial_capability_none;
	local.active_comps       = spatial_component_none;
	local.on_discover        = nullptr;
	local.on_discover_ctx    = nullptr;
	local.pending_permission = false;
}

///////////////////////////////////////////
// Event handling                        //
///////////////////////////////////////////

static void xr_ext_spatial_entity_event_poll(void* context, XrEventDataBuffer* event_data) {
	if (event_data->type != XR_TYPE_EVENT_DATA_SPATIAL_DISCOVERY_RECOMMENDED_EXT)
		return;
	if (!local.ready || !local.on_discover)
		return;

	XrEventDataSpatialDiscoveryRecommendedEXT* recommendation = (XrEventDataSpatialDiscoveryRecommendedEXT*)event_data;
	if (recommendation->spatialContext != local.spatial_context)
		return;

	spatial_entity_begin_discovery();
}

///////////////////////////////////////////
// Discovery                             //
///////////////////////////////////////////

static void spatial_entity_begin_discovery() {
	if (!local.ready || !local.on_discover) return;

	XrSpatialDiscoverySnapshotCreateInfoEXT info = { XR_TYPE_SPATIAL_DISCOVERY_SNAPSHOT_CREATE_INFO_EXT };
	info.componentTypeCount = 0;
	info.componentTypes     = nullptr;

	XrFutureEXT future = XR_NULL_FUTURE_EXT;
	XrResult result = xrCreateSpatialDiscoverySnapshotAsyncEXT(local.spatial_context, &info, &future);
	if (XR_FAILED(result)) {
		log_warnf("%s [%s]", "xrCreateSpatialDiscoverySnapshotAsyncEXT", openxr_string(result));
		return;
	}

	xr_ext_future_on_finish(future, [](void*, XrFutureEXT future) {
		XrCreateSpatialDiscoverySnapshotCompletionEXT     completion      = { XR_TYPE_CREATE_SPATIAL_DISCOVERY_SNAPSHOT_COMPLETION_EXT      };
		XrCreateSpatialDiscoverySnapshotCompletionInfoEXT completion_info = { XR_TYPE_CREATE_SPATIAL_DISCOVERY_SNAPSHOT_COMPLETION_INFO_EXT };
		completion_info.baseSpace = xr_app_space;
		completion_info.time      = xr_time;
		completion_info.future    = future;

		XrResult result = xrCreateSpatialDiscoverySnapshotCompleteEXT(local.spatial_context, &completion_info, &completion);
		if (XR_FAILED(result)) {
			log_warnf("%s [%s]", "xrCreateSpatialDiscoverySnapshotCompleteEXT", openxr_string(result));
			return;
		}
		if (XR_FAILED(completion.futureResult)) {
			log_warnf("%s async [%s]", "xrCreateSpatialDiscoverySnapshotAsyncEXT", openxr_string(completion.futureResult));
			return;
		}

		spatial_snapshot_t* snapshot = sk_malloc_t(spatial_snapshot_t, 1);
		memset(snapshot, 0, sizeof(spatial_snapshot_t));

		if (build_snapshot_from_xr(completion.snapshot, local.active_comps, snapshot)) {
			if (local.on_discover)
				local.on_discover(snapshot, local.on_discover_ctx);
		} else {
			sk_free(snapshot);
		}

		xrDestroySpatialSnapshotEXT(completion.snapshot);
	}, nullptr);
}

///////////////////////////////////////////
// Snapshot building                     //
///////////////////////////////////////////

static bool build_snapshot_from_xr(XrSpatialSnapshotEXT xr_snapshot, spatial_component_ comps, spatial_snapshot_t* out) {
	// Phase 1: Get all entity IDs and tracking states (componentTypeCount=0)
	XrSpatialComponentDataQueryConditionEXT condition = { XR_TYPE_SPATIAL_COMPONENT_DATA_QUERY_CONDITION_EXT };
	condition.componentTypeCount = 0;
	condition.componentTypes     = nullptr;

	XrSpatialComponentDataQueryResultEXT result_info = { XR_TYPE_SPATIAL_COMPONENT_DATA_QUERY_RESULT_EXT };
	XrResult result = xrQuerySpatialComponentDataEXT(xr_snapshot, &condition, &result_info);
	if (XR_FAILED(result)) { log_warnf("%s [%s]", "xrQuerySpatialComponentDataEXT", openxr_string(result)); return false; }

	int32_t count = (int32_t)result_info.entityIdCountOutput;
	if (count == 0) {
		out->entity_count = 0;
		out->entities     = nullptr;
		return true;
	}

	// Allocate and fetch entity IDs and states
	XrSpatialEntityIdEXT*            xr_ids    = sk_malloc_t(XrSpatialEntityIdEXT,            count);
	XrSpatialEntityTrackingStateEXT* xr_states = sk_malloc_t(XrSpatialEntityTrackingStateEXT, count);

	result_info.entityIdCapacityInput    = count;
	result_info.entityIds                = xr_ids;
	result_info.entityStateCapacityInput = count;
	result_info.entityStates             = xr_states;
	result_info.next                     = nullptr;

	result = xrQuerySpatialComponentDataEXT(xr_snapshot, &condition, &result_info);
	if (XR_FAILED(result)) {
		log_warnf("%s [%s]", "xrQuerySpatialComponentDataEXT", openxr_string(result));
		sk_free(xr_ids);
		sk_free(xr_states);
		return false;
	}

	// Build output entity array
	out->entity_count = count;
	out->entities     = sk_malloc_t(spatial_entity_t, count);
	memset(out->entities, 0, sizeof(spatial_entity_t) * count);
	for (int32_t i = 0; i < count; i++) {
		out->entities[i].id         = (spatial_entity_id)xr_ids[i];
		out->entities[i].tracking   = xr_to_sk_tracking(xr_states[i]);
		out->entities[i].components = spatial_component_none;
	}

	// Phase 2: For each enabled component, query individually
	for (int32_t c = 0; c < comp_mapping_count; c++) {
		if (!((int)comps & (int)comp_mappings[c].sk_bit)) continue;
		query_single_component(xr_snapshot, comp_mappings[c].sk_bit, comp_mappings[c].xr_value, count, xr_ids, out);
	}

	sk_free(xr_ids);
	sk_free(xr_states);
	return true;
}

///////////////////////////////////////////

static void query_single_component(XrSpatialSnapshotEXT xr_snapshot, spatial_component_ comp_bit, XrSpatialComponentTypeEXT xr_comp, int32_t all_count, const XrSpatialEntityIdEXT* all_ids, spatial_snapshot_t* snapshot) {
	// Query for just this one component type
	XrSpatialComponentDataQueryConditionEXT condition = { XR_TYPE_SPATIAL_COMPONENT_DATA_QUERY_CONDITION_EXT };
	condition.componentTypeCount = 1;
	condition.componentTypes     = &xr_comp;

	XrSpatialComponentDataQueryResultEXT result_info = { XR_TYPE_SPATIAL_COMPONENT_DATA_QUERY_RESULT_EXT };
	XrResult result = xrQuerySpatialComponentDataEXT(xr_snapshot, &condition, &result_info);
	if (XR_FAILED(result)) return;

	int32_t comp_count = (int32_t)result_info.entityIdCountOutput;
	if (comp_count == 0) return;

	// Allocate XR result arrays
	XrSpatialEntityIdEXT*            comp_ids    = sk_malloc_t(XrSpatialEntityIdEXT,            comp_count);
	XrSpatialEntityTrackingStateEXT* comp_states = sk_malloc_t(XrSpatialEntityTrackingStateEXT, comp_count);
	result_info.entityIdCapacityInput    = comp_count;
	result_info.entityIds                = comp_ids;
	result_info.entityStateCapacityInput = comp_count;
	result_info.entityStates             = comp_states;
	result_info.next                     = nullptr;

	// Allocate the component-specific list struct and chain it
	// Each component type has its own XR list struct type

	// Bounded 2D
	XrSpatialComponentBounded2DListEXT         bounded_2d_list = { XR_TYPE_SPATIAL_COMPONENT_BOUNDED_2D_LIST_EXT };
	XrSpatialBounded2DDataEXT*                 xr_bounds_2d    = nullptr;
	// Bounded 3D
	XrSpatialComponentBounded3DListEXT         bounded_3d_list = { XR_TYPE_SPATIAL_COMPONENT_BOUNDED_3D_LIST_EXT };
	XrBoxf*                                    xr_bounds_3d    = nullptr;
	// Parent
	XrSpatialComponentParentListEXT            parent_list     = { XR_TYPE_SPATIAL_COMPONENT_PARENT_LIST_EXT };
	XrSpatialEntityIdEXT*                      xr_parents      = nullptr;
	// Mesh 3D
	XrSpatialComponentMesh3DListEXT            mesh_3d_list    = { XR_TYPE_SPATIAL_COMPONENT_MESH_3D_LIST_EXT };
	XrSpatialMeshDataEXT*                      xr_meshes_3d    = nullptr;
	// Anchor
	XrSpatialComponentAnchorListEXT            anchor_list     = { XR_TYPE_SPATIAL_COMPONENT_ANCHOR_LIST_EXT };
	XrPosef*                                   xr_anchors      = nullptr;
	// Plane alignment
	XrSpatialComponentPlaneAlignmentListEXT    align_list      = { XR_TYPE_SPATIAL_COMPONENT_PLANE_ALIGNMENT_LIST_EXT };
	XrSpatialPlaneAlignmentEXT*                xr_alignments   = nullptr;
	// Mesh 2D
	XrSpatialComponentMesh2DListEXT            mesh_2d_list    = { XR_TYPE_SPATIAL_COMPONENT_MESH_2D_LIST_EXT };
	XrSpatialMeshDataEXT*                      xr_meshes_2d    = nullptr;
	// Polygon 2D
	XrSpatialComponentPolygon2DListEXT         poly_2d_list    = { XR_TYPE_SPATIAL_COMPONENT_POLYGON_2D_LIST_EXT };
	XrSpatialPolygon2DDataEXT*                 xr_polys_2d     = nullptr;
	// Plane label
	XrSpatialComponentPlaneSemanticLabelListEXT label_list     = { XR_TYPE_SPATIAL_COMPONENT_PLANE_SEMANTIC_LABEL_LIST_EXT };
	XrSpatialPlaneSemanticLabelEXT*            xr_labels       = nullptr;
	// Marker
	XrSpatialComponentMarkerListEXT            marker_list     = { XR_TYPE_SPATIAL_COMPONENT_MARKER_LIST_EXT };
	XrSpatialMarkerDataEXT*                    xr_markers      = nullptr;

	switch (comp_bit) {
	case spatial_component_bounded_2d:
		xr_bounds_2d = sk_malloc_t(XrSpatialBounded2DDataEXT, comp_count);
		memset(xr_bounds_2d, 0, sizeof(XrSpatialBounded2DDataEXT) * comp_count);
		bounded_2d_list.boundCount = comp_count;
		bounded_2d_list.bounds     = xr_bounds_2d;
		xr_insert_next((XrBaseHeader*)&result_info, (XrBaseHeader*)&bounded_2d_list);
		break;
	case spatial_component_bounded_3d:
		xr_bounds_3d = sk_malloc_t(XrBoxf, comp_count);
		memset(xr_bounds_3d, 0, sizeof(XrBoxf) * comp_count);
		bounded_3d_list.boundCount = comp_count;
		bounded_3d_list.bounds     = xr_bounds_3d;
		xr_insert_next((XrBaseHeader*)&result_info, (XrBaseHeader*)&bounded_3d_list);
		break;
	case spatial_component_parent:
		xr_parents = sk_malloc_t(XrSpatialEntityIdEXT, comp_count);
		memset(xr_parents, 0, sizeof(XrSpatialEntityIdEXT) * comp_count);
		parent_list.parentCount = comp_count;
		parent_list.parents     = xr_parents;
		xr_insert_next((XrBaseHeader*)&result_info, (XrBaseHeader*)&parent_list);
		break;
	case spatial_component_mesh_3d:
		xr_meshes_3d = sk_malloc_t(XrSpatialMeshDataEXT, comp_count);
		memset(xr_meshes_3d, 0, sizeof(XrSpatialMeshDataEXT) * comp_count);
		mesh_3d_list.meshCount = comp_count;
		mesh_3d_list.meshes    = xr_meshes_3d;
		xr_insert_next((XrBaseHeader*)&result_info, (XrBaseHeader*)&mesh_3d_list);
		break;
	case spatial_component_anchor:
		xr_anchors = sk_malloc_t(XrPosef, comp_count);
		memset(xr_anchors, 0, sizeof(XrPosef) * comp_count);
		anchor_list.locationCount = comp_count;
		anchor_list.locations     = xr_anchors;
		xr_insert_next((XrBaseHeader*)&result_info, (XrBaseHeader*)&anchor_list);
		break;
	case spatial_component_plane_alignment:
		xr_alignments = sk_malloc_t(XrSpatialPlaneAlignmentEXT, comp_count);
		memset(xr_alignments, 0, sizeof(XrSpatialPlaneAlignmentEXT) * comp_count);
		align_list.planeAlignmentCount = comp_count;
		align_list.planeAlignments     = xr_alignments;
		xr_insert_next((XrBaseHeader*)&result_info, (XrBaseHeader*)&align_list);
		break;
	case spatial_component_mesh_2d:
		xr_meshes_2d = sk_malloc_t(XrSpatialMeshDataEXT, comp_count);
		memset(xr_meshes_2d, 0, sizeof(XrSpatialMeshDataEXT) * comp_count);
		mesh_2d_list.meshCount = comp_count;
		mesh_2d_list.meshes    = xr_meshes_2d;
		xr_insert_next((XrBaseHeader*)&result_info, (XrBaseHeader*)&mesh_2d_list);
		break;
	case spatial_component_polygon_2d:
		xr_polys_2d = sk_malloc_t(XrSpatialPolygon2DDataEXT, comp_count);
		memset(xr_polys_2d, 0, sizeof(XrSpatialPolygon2DDataEXT) * comp_count);
		poly_2d_list.polygonCount = comp_count;
		poly_2d_list.polygons     = xr_polys_2d;
		xr_insert_next((XrBaseHeader*)&result_info, (XrBaseHeader*)&poly_2d_list);
		break;
	case spatial_component_plane_label:
		xr_labels = sk_malloc_t(XrSpatialPlaneSemanticLabelEXT, comp_count);
		memset(xr_labels, 0, sizeof(XrSpatialPlaneSemanticLabelEXT) * comp_count);
		label_list.semanticLabelCount = comp_count;
		label_list.semanticLabels     = xr_labels;
		xr_insert_next((XrBaseHeader*)&result_info, (XrBaseHeader*)&label_list);
		break;
	case spatial_component_marker:
		xr_markers = sk_malloc_t(XrSpatialMarkerDataEXT, comp_count);
		memset(xr_markers, 0, sizeof(XrSpatialMarkerDataEXT) * comp_count);
		marker_list.markerCount = comp_count;
		marker_list.markers     = xr_markers;
		xr_insert_next((XrBaseHeader*)&result_info, (XrBaseHeader*)&marker_list);
		break;
	default: break;
	}

	result = xrQuerySpatialComponentDataEXT(xr_snapshot, &condition, &result_info);
	if (XR_FAILED(result)) {
		sk_free(comp_ids);    sk_free(comp_states);
		sk_free(xr_bounds_2d); sk_free(xr_bounds_3d); sk_free(xr_parents);
		sk_free(xr_meshes_3d); sk_free(xr_anchors);   sk_free(xr_alignments);
		sk_free(xr_meshes_2d); sk_free(xr_polys_2d);  sk_free(xr_labels);
		sk_free(xr_markers);
		return;
	}

	// Map component results back to the master entity array.
	// Build a quick lookup: entity_id -> index in all_ids.
	// For small counts a linear scan is fine.
	auto find_index = [&](XrSpatialEntityIdEXT id) -> int32_t {
		for (int32_t i = 0; i < all_count; i++)
			if (all_ids[i] == id) return i;
		return -1;
	};

	// Ensure the snapshot component arrays are allocated
	int32_t total = snapshot->entity_count;

	if (xr_bounds_2d && !snapshot->bounds_2d) {
		snapshot->bounds_2d = sk_malloc_t(spatial_bounds_2d_t, total);
		memset(snapshot->bounds_2d, 0, sizeof(spatial_bounds_2d_t) * total);
	}
	if (xr_bounds_3d && !snapshot->bounds_3d) {
		snapshot->bounds_3d = sk_malloc_t(spatial_bounds_3d_t, total);
		memset(snapshot->bounds_3d, 0, sizeof(spatial_bounds_3d_t) * total);
	}
	if (xr_parents && !snapshot->parents) {
		snapshot->parents = sk_malloc_t(spatial_entity_id, total);
		memset(snapshot->parents, 0, sizeof(spatial_entity_id) * total);
	}
	if (xr_meshes_3d && !snapshot->meshes_3d) {
		snapshot->meshes_3d = sk_malloc_t(spatial_mesh_t, total);
		memset(snapshot->meshes_3d, 0, sizeof(spatial_mesh_t) * total);
	}
	if (xr_anchors && !snapshot->anchor_poses) {
		snapshot->anchor_poses = sk_malloc_t(pose_t, total);
		memset(snapshot->anchor_poses, 0, sizeof(pose_t) * total);
	}
	if (xr_alignments && !snapshot->plane_alignments) {
		snapshot->plane_alignments = sk_malloc_t(spatial_plane_align_, total);
		memset(snapshot->plane_alignments, 0, sizeof(spatial_plane_align_) * total);
	}
	if (xr_meshes_2d && !snapshot->meshes_2d) {
		snapshot->meshes_2d = sk_malloc_t(spatial_mesh_t, total);
		memset(snapshot->meshes_2d, 0, sizeof(spatial_mesh_t) * total);
	}
	if (xr_polys_2d && !snapshot->polygons_2d) {
		snapshot->polygons_2d = sk_malloc_t(spatial_polygon_2d_t, total);
		memset(snapshot->polygons_2d, 0, sizeof(spatial_polygon_2d_t) * total);
	}
	if (xr_labels && !snapshot->plane_labels) {
		snapshot->plane_labels = sk_malloc_t(spatial_plane_label_, total);
		memset(snapshot->plane_labels, 0, sizeof(spatial_plane_label_) * total);
	}
	if (xr_markers && !snapshot->markers) {
		snapshot->markers = sk_malloc_t(spatial_marker_t, total);
		memset(snapshot->markers, 0, sizeof(spatial_marker_t) * total);
	}

	// Copy data into the snapshot
	for (int32_t i = 0; i < comp_count; i++) {
		int32_t idx = find_index(comp_ids[i]);
		if (idx < 0) continue;

		snapshot->entities[idx].components = (spatial_component_)((int)snapshot->entities[idx].components | (int)comp_bit);

		switch (comp_bit) {
		case spatial_component_bounded_2d:
			snapshot->bounds_2d[idx].center  = xr_to_pose(xr_bounds_2d[i].center);
			snapshot->bounds_2d[idx].extents = { xr_bounds_2d[i].extents.width, xr_bounds_2d[i].extents.height };
			break;

		case spatial_component_bounded_3d:
			snapshot->bounds_3d[idx].center  = xr_to_pose(xr_bounds_3d[i].center);
			snapshot->bounds_3d[idx].extents = { xr_bounds_3d[i].extents.width, xr_bounds_3d[i].extents.height, xr_bounds_3d[i].extents.depth };
			break;

		case spatial_component_parent:
			snapshot->parents[idx] = (spatial_entity_id)xr_parents[i];
			break;

		case spatial_component_mesh_3d: {
			spatial_mesh_t* mesh = &snapshot->meshes_3d[idx];
			mesh->origin = xr_to_pose(xr_meshes_3d[i].origin);

			// Retrieve vertex buffer (Vector3f)
			XrSpatialBufferGetInfoEXT buf_info = { XR_TYPE_SPATIAL_BUFFER_GET_INFO_EXT };
			buf_info.bufferId = xr_meshes_3d[i].vertexBuffer.bufferId;
			uint32_t vert_count = 0;
			xrGetSpatialBufferVector3fEXT(xr_snapshot, &buf_info, 0, &vert_count, nullptr);
			if (vert_count > 0) {
				mesh->vertex_count = (int32_t)vert_count;
				mesh->vertices     = sk_malloc_t(vec3, vert_count);
				xrGetSpatialBufferVector3fEXT(xr_snapshot, &buf_info, vert_count, &vert_count, (XrVector3f*)mesh->vertices);
			}

			// Retrieve index buffer (Uint32)
			buf_info.bufferId = xr_meshes_3d[i].indexBuffer.bufferId;
			uint32_t idx_count = 0;
			xrGetSpatialBufferUint32EXT(xr_snapshot, &buf_info, 0, &idx_count, nullptr);
			if (idx_count > 0) {
				mesh->index_count = (int32_t)idx_count;
				mesh->indices     = sk_malloc_t(uint32_t, idx_count);
				xrGetSpatialBufferUint32EXT(xr_snapshot, &buf_info, idx_count, &idx_count, mesh->indices);
			}
		} break;

		case spatial_component_anchor:
			snapshot->anchor_poses[idx] = xr_to_pose(xr_anchors[i]);
			break;

		case spatial_component_plane_alignment:
			snapshot->plane_alignments[idx] = (spatial_plane_align_)xr_alignments[i];
			break;

		case spatial_component_mesh_2d: {
			spatial_mesh_t* mesh = &snapshot->meshes_2d[idx];
			mesh->origin = xr_to_pose(xr_meshes_2d[i].origin);

			// Vertex buffer is Vector2f — widen to vec3 (z=0)
			XrSpatialBufferGetInfoEXT buf_info = { XR_TYPE_SPATIAL_BUFFER_GET_INFO_EXT };
			buf_info.bufferId = xr_meshes_2d[i].vertexBuffer.bufferId;
			uint32_t vert_count = 0;
			xrGetSpatialBufferVector2fEXT(xr_snapshot, &buf_info, 0, &vert_count, nullptr);
			if (vert_count > 0) {
				XrVector2f* verts_2d = sk_malloc_t(XrVector2f, vert_count);
				xrGetSpatialBufferVector2fEXT(xr_snapshot, &buf_info, vert_count, &vert_count, verts_2d);
				mesh->vertex_count = (int32_t)vert_count;
				mesh->vertices     = sk_malloc_t(vec3, vert_count);
				for (uint32_t v = 0; v < vert_count; v++)
					mesh->vertices[v] = { verts_2d[v].x, verts_2d[v].y, 0 };
				sk_free(verts_2d);
			}

			// Index buffer is Uint16 — widen to uint32
			buf_info.bufferId = xr_meshes_2d[i].indexBuffer.bufferId;
			uint32_t idx_count = 0;
			xrGetSpatialBufferUint16EXT(xr_snapshot, &buf_info, 0, &idx_count, nullptr);
			if (idx_count > 0) {
				uint16_t* inds_16 = sk_malloc_t(uint16_t, idx_count);
				xrGetSpatialBufferUint16EXT(xr_snapshot, &buf_info, idx_count, &idx_count, inds_16);
				mesh->index_count = (int32_t)idx_count;
				mesh->indices     = sk_malloc_t(uint32_t, idx_count);
				for (uint32_t j = 0; j < idx_count; j++)
					mesh->indices[j] = inds_16[j];
				sk_free(inds_16);
			}
		} break;

		case spatial_component_polygon_2d: {
			spatial_polygon_2d_t* poly = &snapshot->polygons_2d[idx];
			poly->origin = xr_to_pose(xr_polys_2d[i].origin);

			XrSpatialBufferGetInfoEXT buf_info = { XR_TYPE_SPATIAL_BUFFER_GET_INFO_EXT };
			buf_info.bufferId = xr_polys_2d[i].vertexBuffer.bufferId;
			uint32_t vert_count = 0;
			xrGetSpatialBufferVector2fEXT(xr_snapshot, &buf_info, 0, &vert_count, nullptr);
			if (vert_count > 0) {
				poly->vertex_count = (int32_t)vert_count;
				poly->vertices     = sk_malloc_t(vec2, vert_count);
				xrGetSpatialBufferVector2fEXT(xr_snapshot, &buf_info, vert_count, &vert_count, (XrVector2f*)poly->vertices);
			}
		} break;

		case spatial_component_plane_label:
			snapshot->plane_labels[idx] = (spatial_plane_label_)xr_labels[i];
			break;

		case spatial_component_marker: {
			spatial_marker_t* marker = &snapshot->markers[idx];
			marker->type      = xr_cap_to_marker_type(xr_markers[i].capability);
			marker->marker_id = xr_markers[i].markerId;
			marker->data      = nullptr;
			marker->data_raw  = nullptr;
			marker->data_size = 0;

			// Retrieve decoded data if available
			if (xr_markers[i].data.bufferId != XR_NULL_SPATIAL_BUFFER_ID_EXT) {
				XrSpatialBufferGetInfoEXT buf_info = { XR_TYPE_SPATIAL_BUFFER_GET_INFO_EXT };
				buf_info.bufferId = xr_markers[i].data.bufferId;

				if (xr_markers[i].data.bufferType == XR_SPATIAL_BUFFER_TYPE_STRING_EXT) {
					uint32_t str_len = 0;
					xrGetSpatialBufferStringEXT(xr_snapshot, &buf_info, 0, &str_len, nullptr);
					if (str_len > 0) {
						char* str = sk_malloc_t(char, str_len);
						xrGetSpatialBufferStringEXT(xr_snapshot, &buf_info, str_len, &str_len, str);
						marker->data = str;
					}
				} else if (xr_markers[i].data.bufferType == XR_SPATIAL_BUFFER_TYPE_UINT8_EXT) {
					uint32_t byte_count = 0;
					xrGetSpatialBufferUint8EXT(xr_snapshot, &buf_info, 0, &byte_count, nullptr);
					if (byte_count > 0) {
						uint8_t* bytes = sk_malloc_t(uint8_t, byte_count);
						xrGetSpatialBufferUint8EXT(xr_snapshot, &buf_info, byte_count, &byte_count, bytes);
						marker->data_raw  = bytes;
						marker->data_size = (int32_t)byte_count;
					}
				}
			}
		} break;

		default: break;
		}
	}

	// Free temporary XR arrays
	sk_free(comp_ids);
	sk_free(comp_states);
	sk_free(xr_bounds_2d);
	sk_free(xr_bounds_3d);
	sk_free(xr_parents);
	sk_free(xr_meshes_3d);
	sk_free(xr_anchors);
	sk_free(xr_alignments);
	sk_free(xr_meshes_2d);
	sk_free(xr_polys_2d);
	sk_free(xr_labels);
	sk_free(xr_markers);
}

///////////////////////////////////////////
// Update snapshot                       //
///////////////////////////////////////////

bool spatial_entity_request_update(const spatial_entity_id* entity_ids, int32_t entity_count, spatial_component_ components, spatial_snapshot_t* out_snapshot) {
	if (!local.ready || entity_count <= 0) return false;

	// Create entity handles from IDs
	XrSpatialEntityEXT* xr_entities = sk_malloc_t(XrSpatialEntityEXT, entity_count);
	for (int32_t i = 0; i < entity_count; i++) {
		XrSpatialEntityFromIdCreateInfoEXT create_info = { XR_TYPE_SPATIAL_ENTITY_FROM_ID_CREATE_INFO_EXT };
		create_info.entityId = (XrSpatialEntityIdEXT)entity_ids[i];
		XrResult result = xrCreateSpatialEntityFromIdEXT(local.spatial_context, &create_info, &xr_entities[i]);
		if (XR_FAILED(result)) {
			// Clean up already-created handles
			for (int32_t j = 0; j < i; j++)
				xrDestroySpatialEntityEXT(xr_entities[j]);
			sk_free(xr_entities);
			return false;
		}
	}

	// Build component type array
	XrSpatialComponentTypeEXT comp_types[comp_mapping_count];
	int32_t comp_count = sk_comps_to_xr(components, comp_types);

	XrSpatialUpdateSnapshotCreateInfoEXT create_info = { XR_TYPE_SPATIAL_UPDATE_SNAPSHOT_CREATE_INFO_EXT };
	create_info.entityCount       = entity_count;
	create_info.entities          = xr_entities;
	create_info.componentTypeCount = comp_count;
	create_info.componentTypes     = comp_count > 0 ? comp_types : nullptr;
	create_info.baseSpace          = xr_app_space;
	create_info.time               = xr_time;

	XrSpatialSnapshotEXT xr_snapshot = XR_NULL_HANDLE;
	XrResult result = xrCreateSpatialUpdateSnapshotEXT(local.spatial_context, &create_info, &xr_snapshot);

	// Destroy entity handles
	for (int32_t i = 0; i < entity_count; i++)
		xrDestroySpatialEntityEXT(xr_entities[i]);
	sk_free(xr_entities);

	if (XR_FAILED(result)) {
		log_warnf("%s [%s]", "xrCreateSpatialUpdateSnapshotEXT", openxr_string(result));
		return false;
	}

	memset(out_snapshot, 0, sizeof(spatial_snapshot_t));
	bool success = build_snapshot_from_xr(xr_snapshot, components, out_snapshot);
	xrDestroySpatialSnapshotEXT(xr_snapshot);
	return success;
}

///////////////////////////////////////////
// Anchor creation                       //
///////////////////////////////////////////

spatial_entity_id spatial_entity_create_anchor(pose_t pose) {
	if (!local.ready || !local.anchor_ext) return 0;
	if (!((int)local.active_caps & (int)spatial_capability_anchor)) return 0;

	XrSpatialAnchorCreateInfoEXT create_info = { XR_TYPE_SPATIAL_ANCHOR_CREATE_INFO_EXT };
	create_info.baseSpace = xr_app_space;
	create_info.time      = xr_time;
	create_info.pose      = pose_to_xr(pose);

	XrSpatialEntityIdEXT anchor_id     = XR_NULL_SPATIAL_ENTITY_ID_EXT;
	XrSpatialEntityEXT   anchor_entity = XR_NULL_HANDLE;
	XrResult result = xrCreateSpatialAnchorEXT(local.spatial_context, &create_info, &anchor_id, &anchor_entity);
	if (XR_FAILED(result)) {
		log_warnf("%s [%s]", "xrCreateSpatialAnchorEXT", openxr_string(result));
		return 0;
	}

	// Keep the entity handle alive so the runtime tracks it
	local.anchor_entities.add(anchor_entity);

	return (spatial_entity_id)anchor_id;
}

///////////////////////////////////////////
// Snapshot release                      //
///////////////////////////////////////////

void spatial_snapshot_release(spatial_snapshot_t* snapshot) {
	if (!snapshot) return;

	int32_t count = snapshot->entity_count;

	// Free per-entity nested allocations
	for (int32_t i = 0; i < count; i++) {
		if (snapshot->meshes_3d) {
			sk_free(snapshot->meshes_3d[i].vertices);
			sk_free(snapshot->meshes_3d[i].indices);
		}
		if (snapshot->meshes_2d) {
			sk_free(snapshot->meshes_2d[i].vertices);
			sk_free(snapshot->meshes_2d[i].indices);
		}
		if (snapshot->polygons_2d) {
			sk_free(snapshot->polygons_2d[i].vertices);
		}
		if (snapshot->markers) {
			_sk_free((void*)snapshot->markers[i].data);
			_sk_free((void*)snapshot->markers[i].data_raw);
		}
	}

	// Free component arrays
	sk_free(snapshot->bounds_2d);
	sk_free(snapshot->bounds_3d);
	sk_free(snapshot->parents);
	sk_free(snapshot->meshes_3d);
	sk_free(snapshot->anchor_poses);
	sk_free(snapshot->plane_alignments);
	sk_free(snapshot->meshes_2d);
	sk_free(snapshot->polygons_2d);
	sk_free(snapshot->plane_labels);
	sk_free(snapshot->markers);
	sk_free(snapshot->entities);
}

///////////////////////////////////////////
// Shutdown and step_begin               //
///////////////////////////////////////////

static void xr_ext_spatial_entity_shutdown(void*) {
	spatial_entity_stop();
	local.anchor_entities.free();
	local.cap_supports.free();
	local = {};

	OPENXR_CLEAR_FN(XR_EXT_FUNCTIONS);
	OPENXR_CLEAR_FN(XR_ANCHOR_FUNCTIONS);
}

static void xr_ext_spatial_entity_step_begin(void*) {
	if (local.pending_permission) {
		permission_state_ perm = permission_state(permission_type_scene);
		if (perm == permission_state_granted) {
			local.pending_permission = false;
			log_diag("spatial_entity: scene permission granted, starting");
			spatial_entity_start_internal(local.pending_caps, local.pending_comps, local.pending_on_discover, local.pending_on_discover_ctx);
		} else if (perm != permission_state_capable) {
			local.pending_permission = false;
			log_warn("spatial_entity: scene permission denied");
		}
	}
}

} // namespace sk
