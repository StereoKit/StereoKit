/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

// OpenXR provider for the spatial entity registry, implementing
// XR_EXT_spatial_entity and its satellite extensions.
// https://registry.khronos.org/OpenXR/specs/1.1/html/xrspec.html#XR_EXT_spatial_entity
//
// This uses one XrSpatialContextEXT "slot" per capability, so changing
// one capability never invalidates entities belonging to another, and
// entity IDs are unique across contexts, so all slots feed one registry.

#include "spatial_entity.h"
#include "ext_management.h"
#include "future.h"
#include "../openxr.h"
#include "../../systems/spatial_entity.h"
#include "../../libraries/array.h"
#include "../../sk_memory.h"

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
	X(xrGetSpatialBufferVector2fEXT)                 \
	X(xrGetSpatialBufferVector3fEXT)
OPENXR_DEFINE_FN_STATIC(XR_EXT_FUNCTIONS);

#define XR_ANCHOR_FUNCTIONS( X ) \
	X(xrCreateSpatialAnchorEXT)
OPENXR_DEFINE_FN_STATIC(XR_ANCHOR_FUNCTIONS);

#define XR_PERSIST_FUNCTIONS( X )                    \
	X(xrEnumerateSpatialPersistenceScopesEXT)        \
	X(xrCreateSpatialPersistenceContextAsyncEXT)     \
	X(xrCreateSpatialPersistenceContextCompleteEXT)  \
	X(xrDestroySpatialPersistenceContextEXT)
OPENXR_DEFINE_FN_STATIC(XR_PERSIST_FUNCTIONS);

#define XR_PERSIST_OPS_FUNCTIONS( X )                \
	X(xrPersistSpatialEntityAsyncEXT)                \
	X(xrPersistSpatialEntityCompleteEXT)             \
	X(xrUnpersistSpatialEntityAsyncEXT)              \
	X(xrUnpersistSpatialEntityCompleteEXT)
OPENXR_DEFINE_FN_STATIC(XR_PERSIST_OPS_FUNCTIONS);

namespace sk {

///////////////////////////////////////////
// Mapping tables                        //
///////////////////////////////////////////

struct cap_mapping_t {
	spatial_capability_    sk_bit;
	XrSpatialCapabilityEXT xr_value;
	XrStructureType        config_type;
	const char*            ext_name;
	const char*            name;
	marker_type_           marker_type; // None for capabilities that aren't markers
};

static const cap_mapping_t cap_mappings[] = {
	{ spatial_capability_anchor,         XR_SPATIAL_CAPABILITY_ANCHOR_EXT,                        XR_TYPE_SPATIAL_CAPABILITY_CONFIGURATION_ANCHOR_EXT,         XR_EXT_SPATIAL_ANCHOR_EXTENSION_NAME,          "anchor",         marker_type_none      },
	{ spatial_capability_plane_tracking, XR_SPATIAL_CAPABILITY_PLANE_TRACKING_EXT,                XR_TYPE_SPATIAL_CAPABILITY_CONFIGURATION_PLANE_TRACKING_EXT, XR_EXT_SPATIAL_PLANE_TRACKING_EXTENSION_NAME,  "plane tracking", marker_type_none      },
	{ spatial_capability_qr_code,        XR_SPATIAL_CAPABILITY_MARKER_TRACKING_QR_CODE_EXT,       XR_TYPE_SPATIAL_CAPABILITY_CONFIGURATION_QR_CODE_EXT,        XR_EXT_SPATIAL_MARKER_TRACKING_EXTENSION_NAME, "QR code",        marker_type_qr_code   },
	{ spatial_capability_micro_qr,       XR_SPATIAL_CAPABILITY_MARKER_TRACKING_MICRO_QR_CODE_EXT, XR_TYPE_SPATIAL_CAPABILITY_CONFIGURATION_MICRO_QR_CODE_EXT,  XR_EXT_SPATIAL_MARKER_TRACKING_EXTENSION_NAME, "micro QR code",  marker_type_micro_qr  },
	{ spatial_capability_aruco,          XR_SPATIAL_CAPABILITY_MARKER_TRACKING_ARUCO_MARKER_EXT,  XR_TYPE_SPATIAL_CAPABILITY_CONFIGURATION_ARUCO_MARKER_EXT,   XR_EXT_SPATIAL_MARKER_TRACKING_EXTENSION_NAME, "ArUco marker",   marker_type_aruco     },
	{ spatial_capability_april_tag,      XR_SPATIAL_CAPABILITY_MARKER_TRACKING_APRIL_TAG_EXT,     XR_TYPE_SPATIAL_CAPABILITY_CONFIGURATION_APRIL_TAG_EXT,      XR_EXT_SPATIAL_MARKER_TRACKING_EXTENSION_NAME, "AprilTag",       marker_type_april_tag },
};
static const int32_t cap_mapping_count = sizeof(cap_mappings) / sizeof(cap_mappings[0]);

struct comp_mapping_t {
	spatial_component_         sk_bit;
	XrSpatialComponentTypeEXT  xr_value;
};

static const comp_mapping_t comp_mappings[] = {
	{ spatial_component_bounds2d,        XR_SPATIAL_COMPONENT_TYPE_BOUNDED_2D_EXT          },
	{ spatial_component_bounds3d,        XR_SPATIAL_COMPONENT_TYPE_BOUNDED_3D_EXT          },
	{ spatial_component_parent,          XR_SPATIAL_COMPONENT_TYPE_PARENT_EXT              },
	{ spatial_component_mesh,            XR_SPATIAL_COMPONENT_TYPE_MESH_3D_EXT             },
	{ spatial_component_anchor,          XR_SPATIAL_COMPONENT_TYPE_ANCHOR_EXT              },
	{ spatial_component_persistence,     XR_SPATIAL_COMPONENT_TYPE_PERSISTENCE_EXT         },
	{ spatial_component_plane_alignment, XR_SPATIAL_COMPONENT_TYPE_PLANE_ALIGNMENT_EXT     },
	{ spatial_component_mesh2d,          XR_SPATIAL_COMPONENT_TYPE_MESH_2D_EXT             },
	{ spatial_component_polygon,         XR_SPATIAL_COMPONENT_TYPE_POLYGON_2D_EXT          },
	{ spatial_component_label,           XR_SPATIAL_COMPONENT_TYPE_PLANE_SEMANTIC_LABEL_EXT},
	{ spatial_component_marker,          XR_SPATIAL_COMPONENT_TYPE_MARKER_EXT              },
};
static const int32_t comp_mapping_count = sizeof(comp_mappings) / sizeof(comp_mappings[0]);

///////////////////////////////////////////
// State                                 //
///////////////////////////////////////////

typedef enum slot_state_ {
	slot_state_off,
	slot_state_permission, // Waiting on the capability's permission, or persistence contexts
	slot_state_creating,   // Context creation future in flight
	slot_state_ready,
	slot_state_stopping,   // Wants to stop, discovery future still in flight
} slot_state_;

struct ent_handle_t {
	XrSpatialEntityIdEXT id;
	XrSpatialEntityEXT   handle;
	spatial_entity_t     entity;    // The registry's handle for this entity
	int32_t              batch_idx; // Position in the snapshot being ingested
	// Buffer id stamps: same id means same data, so a re-fetch is only
	// needed when these change.
	XrSpatialBufferIdEXT stamp_mesh_v;
	XrSpatialBufferIdEXT stamp_mesh_i;
	XrSpatialBufferIdEXT stamp_mesh2d_v;
	XrSpatialBufferIdEXT stamp_mesh2d_i;
	XrSpatialBufferIdEXT stamp_polygon;
	XrSpatialBufferIdEXT stamp_marker;
};

struct slot_t {
	slot_state_           state;
	XrSpatialContextEXT   context;
	spatial_component_    comps;         // Components enabled on this context
	uint32_t              config_serial; // Marker config version this context was built with
	int32_t               update_count;  // For pacing full component updates
	bool                  discovery_active;
	bool                  discovery_queued;
	bool                  destroy_after_create;
	bool                  update_warned;
	bool                  perm_warned;
	array_t<ent_handle_t> handles;
	hashmap_t<XrSpatialEntityIdEXT, int32_t> handle_idx; // Entity id to index in handles
};

// Heap data that must survive until the context creation future resolves.
struct slot_create_ctx_t {
	int32_t slot_idx;
	// Large enough for any capability config struct; they all begin with
	// XrSpatialCapabilityConfigurationBaseHeaderEXT.
	XrSpatialCapabilityConfigurationArucoMarkerEXT config;
	XrSpatialCapabilityConfigurationBaseHeaderEXT* config_ptr;
	XrSpatialComponentTypeEXT                      comp_types[comp_mapping_count];
	XrSpatialMarkerSizeEXT                         marker_size;
	XrSpatialMarkerStaticOptimizationEXT           marker_static;
	XrSpatialContextPersistenceConfigEXT           persist_config;
	XrSpatialPersistenceContextEXT                 persist_contexts[2];
};

struct scratch_t {
	void*  data;
	size_t size;
};

struct xr_spatial_state_t {
	bool                available;
	bool                anchor_ext;
	spatial_capability_ supported;
	spatial_capability_ attempted; // Caps tried since their last enable, prevents failure retry spam
	spatial_capability_ feat_fixed_size; // Caps supporting the fixed-size-markers feature
	spatial_capability_ feat_static;     // Caps supporting the static-markers feature
	int32_t             requested_perms; // Bitmask by permission_type_, so requests only fire once
	slot_t              slots[cap_mapping_count];

	bool                           persist_ops_ext;
	bool                           persist_writable; // The app can persist things, not just read them
	XrSpatialPersistenceScopeEXT   usable_scopes[2];
	int32_t                        usable_scope_count;
	bool                           persist_attempted; // Context creation happens once, after permission settles
	int32_t                        persist_pending;   // Persistence context creations in flight
	XrSpatialPersistenceContextEXT persist_contexts[2];
	int32_t                        persist_context_count;
	XrSpatialPersistenceContextEXT persist_write_ctx; // The writable (LOCAL_ANCHORS) context, if any
	float                          find_next_time;    // Pacing for persist id lookups

	// Reused by every snapshot, so frame updates don't allocate
	scratch_t scratch_ids;
	scratch_t scratch_states;
	scratch_t scratch_batch;
	scratch_t scratch_comp_ids;
	scratch_t scratch_comp_states;
	scratch_t scratch_comp_data;
	scratch_t scratch_entities;
};
static xr_spatial_state_t local = {};

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

// The spec faces surfaces and objects along +Z, while StereoKit faces -Z.
// Rotating π around local Y makes Forward the facing direction; 2D vertex
// data in that frame must also negate local X.
static inline pose_t xr_to_pose_faced(const XrPosef& xr_pose) {
	pose_t result = xr_to_pose(xr_pose);
	quat   flip   = { 0,1,0,0 }; // π around Y
	// quat_mul applies its first argument first, so the flip happens in
	// the pose's local frame, before the pose's own rotation.
	result.orientation = quat_mul(flip, result.orientation);
	return result;
}

// Zeroed, and only valid until the next request for the same scratch.
template <typename T>
static T* scratch_get(scratch_t* scratch, int32_t count) {
	size_t size = sizeof(T) * (size_t)count;
	if (scratch->size < size) {
		scratch->data = sk_realloc(scratch->data, size);
		scratch->size = size;
	}
	memset(scratch->data, 0, size);
	return (T*)scratch->data;
}

static_assert(sizeof(sk_uuid_t) == sizeof(XrUuid), "Discovery filters pass sk_uuid_t arrays as XrUuid");
static inline sk_uuid_t xr_to_uuid(const XrUuid& xr_uuid) { sk_uuid_t result; memcpy(result.bytes, xr_uuid.data, sizeof(result.bytes)); return result; }
static inline XrUuid    uuid_to_xr(const sk_uuid_t& uuid ) { XrUuid    result; memcpy(result.data,  uuid.bytes,   sizeof(result.data )); return result; }

static spatial_component_ xr_to_sk_comp(XrSpatialComponentTypeEXT xr_comp) {
	for (int32_t i = 0; i < comp_mapping_count; i++) {
		if (comp_mappings[i].xr_value == xr_comp)
			return comp_mappings[i].sk_bit;
	}
	return spatial_component_none;
}

static spatial_tracking_ xr_to_sk_tracking(XrSpatialEntityTrackingStateEXT xr_state) {
	switch (xr_state) {
	case XR_SPATIAL_ENTITY_TRACKING_STATE_TRACKING_EXT: return spatial_tracking_tracking;
	case XR_SPATIAL_ENTITY_TRACKING_STATE_PAUSED_EXT:   return spatial_tracking_paused;
	default:                                            return spatial_tracking_stopped;
	}
}

static plane_align_ xr_to_plane_align(XrSpatialPlaneAlignmentEXT xr_align) {
	switch (xr_align) {
	case XR_SPATIAL_PLANE_ALIGNMENT_HORIZONTAL_UPWARD_EXT:   return plane_align_horizontal_up;
	case XR_SPATIAL_PLANE_ALIGNMENT_HORIZONTAL_DOWNWARD_EXT: return plane_align_horizontal_down;
	case XR_SPATIAL_PLANE_ALIGNMENT_VERTICAL_EXT:            return plane_align_vertical;
	default:                                                 return plane_align_arbitrary;
	}
}

static spatial_label_ xr_to_plane_label(XrSpatialPlaneSemanticLabelEXT xr_label) {
	switch (xr_label) {
	case XR_SPATIAL_PLANE_SEMANTIC_LABEL_FLOOR_EXT:   return spatial_label_floor;
	case XR_SPATIAL_PLANE_SEMANTIC_LABEL_WALL_EXT:    return spatial_label_wall;
	case XR_SPATIAL_PLANE_SEMANTIC_LABEL_CEILING_EXT: return spatial_label_ceiling;
	case XR_SPATIAL_PLANE_SEMANTIC_LABEL_TABLE_EXT:   return spatial_label_table;
	default:                                          return spatial_label_uncategorized;
	}
}

static int32_t slot_from_context(XrSpatialContextEXT context) {
	for (int32_t i = 0; i < cap_mapping_count; i++)
		if (local.slots[i].context == context && context != XR_NULL_HANDLE) return i;
	return -1;
}

static slot_t* slot_from_cap(spatial_capability_ cap) {
	for (int32_t i = 0; i < cap_mapping_count; i++)
		if (cap_mappings[i].sk_bit == cap) return &local.slots[i];
	return nullptr;
}

static ent_handle_t* slot_find_handle(slot_t* slot, XrSpatialEntityIdEXT id) {
	int32_t* idx = slot->handle_idx.get(id);
	return idx == nullptr ? nullptr : &slot->handles[*idx];
}

static void slot_add_handle(slot_t* slot, const ent_handle_t& handle) {
	slot->handle_idx.set(handle.id, slot->handles.count);
	slot->handles.add(handle);
}

// Swaps the last handle into the gap, so only that one needs re-indexing.
static void slot_remove_handle(slot_t* slot, XrSpatialEntityIdEXT id) {
	int32_t* found = slot->handle_idx.get(id);
	if (found == nullptr) return;
	int32_t idx  = *found;
	int32_t last = slot->handles.count - 1;
	xrDestroySpatialEntityEXT(slot->handles[idx].handle);
	slot->handle_idx.remove(id);
	if (idx != last) {
		slot->handles[idx] = slot->handles[last];
		slot->handle_idx.set(slot->handles[idx].id, idx);
	}
	slot->handles.remove(last);
}

static void slot_clear_handles(slot_t* slot) {
	for (int32_t i = 0; i < slot->handles.count; i++)
		xrDestroySpatialEntityEXT(slot->handles[i].handle);
	slot->handles   .clear();
	slot->handle_idx.free();
}

///////////////////////////////////////////
// Forward declarations                  //
///////////////////////////////////////////

static xr_system_ xr_spatial_initialize(void*);
static void       xr_spatial_shutdown  (void*);
static void       xr_spatial_step_begin(void*);
static void       xr_spatial_event_poll(void* context, XrEventDataBuffer* event_data);

static void       slot_begin_start     (int32_t slot_idx);
static void       slot_create          (int32_t slot_idx);
static void       slot_stop            (int32_t slot_idx);
static void       slot_discover        (int32_t slot_idx, const sk_uuid_t* persist_ids, int32_t persist_id_count);
static void       slot_update          (int32_t slot_idx);
static void       slot_ingest_snapshot (int32_t slot_idx, XrSpatialSnapshotEXT xr_snapshot, bool create_handles, spatial_component_ comps, int32_t max_entities);

static spatial_entity_id_t xr_spatial_create_anchor(pose_t pose, spatial_entity_id_t parent, spatial_entity_t entity);
static void                xr_spatial_destroy      (spatial_entity_id_t id);
static void                xr_spatial_persist      (spatial_entity_id_t id, spatial_entity_t entity);
static void                xr_spatial_unpersist    (spatial_entity_t entity, sk_uuid_t persist_id);
static void                persistence_init        ();
static void                persistence_create_contexts();

///////////////////////////////////////////
// Registration                          //
///////////////////////////////////////////

void xr_ext_spatial_entity_register() {
	xr_system_t sys = {};
	sys.request_exts    [sys.request_ext_count    ++] = XR_EXT_SPATIAL_ENTITY_EXTENSION_NAME;
	sys.request_opt_exts[sys.request_opt_ext_count++] = XR_EXT_SPATIAL_ANCHOR_EXTENSION_NAME;
	sys.request_opt_exts[sys.request_opt_ext_count++] = XR_EXT_SPATIAL_PLANE_TRACKING_EXTENSION_NAME;
	sys.request_opt_exts[sys.request_opt_ext_count++] = XR_EXT_SPATIAL_MARKER_TRACKING_EXTENSION_NAME;
	sys.request_opt_exts[sys.request_opt_ext_count++] = XR_EXT_SPATIAL_PERSISTENCE_EXTENSION_NAME;
	sys.request_opt_exts[sys.request_opt_ext_count++] = XR_EXT_SPATIAL_PERSISTENCE_OPERATIONS_EXTENSION_NAME;
	sys.evt_initialize = { xr_spatial_initialize };
	sys.evt_shutdown   = { xr_spatial_shutdown   };
	sys.evt_step_begin = { xr_spatial_step_begin };
	sys.evt_poll       = { (void(*)(void*, void*))xr_spatial_event_poll };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////
// Initialization                        //
///////////////////////////////////////////

static xr_system_ xr_spatial_initialize(void*) {
	if (!backend_openxr_ext_enabled(XR_EXT_SPATIAL_ENTITY_EXTENSION_NAME))
		return xr_system_fail;

	OPENXR_LOAD_FN_RETURN(XR_EXT_FUNCTIONS, xr_system_fail);

	local.anchor_ext = backend_openxr_ext_enabled(XR_EXT_SPATIAL_ANCHOR_EXTENSION_NAME);
	if (local.anchor_ext) {
		OPENXR_LOAD_FN_RETURN(XR_ANCHOR_FUNCTIONS, xr_system_fail);
	}
	persistence_init();

	// Enumerate supported capabilities
	uint32_t cap_count = 0;
	XrResult result = xrEnumerateSpatialCapabilitiesEXT(xr_instance, xr_system_id, 0, &cap_count, nullptr);
	if (XR_FAILED(result)) { log_warnf("%s [%s]", "xrEnumerateSpatialCapabilitiesEXT", openxr_string(result)); return xr_system_fail; }

	XrSpatialCapabilityEXT* xr_caps = sk_malloc_t(XrSpatialCapabilityEXT, cap_count);
	result = xrEnumerateSpatialCapabilitiesEXT(xr_instance, xr_system_id, cap_count, &cap_count, xr_caps);
	if (XR_FAILED(result)) { log_warnf("%s [%s]", "xrEnumerateSpatialCapabilitiesEXT", openxr_string(result)); sk_free(xr_caps); return xr_system_fail; }

	for (uint32_t i = 0; i < cap_count; i++) {
		int32_t map_idx = -1;
		for (int32_t m = 0; m < cap_mapping_count; m++)
			if (cap_mappings[m].xr_value == xr_caps[i]) { map_idx = m; break; }
		if (map_idx == -1) continue;
		if (!backend_openxr_ext_enabled(cap_mappings[map_idx].ext_name)) continue;

		// Enumerate components for this capability
		XrSpatialCapabilityComponentTypesEXT comp_types = { XR_TYPE_SPATIAL_CAPABILITY_COMPONENT_TYPES_EXT };
		xrEnumerateSpatialCapabilityComponentTypesEXT(xr_instance, xr_system_id, xr_caps[i], &comp_types);
		comp_types.componentTypeCapacityInput = comp_types.componentTypeCountOutput;
		comp_types.componentTypes             = sk_malloc_t(XrSpatialComponentTypeEXT, comp_types.componentTypeCountOutput);
		result = xrEnumerateSpatialCapabilityComponentTypesEXT(xr_instance, xr_system_id, xr_caps[i], &comp_types);
		if (XR_FAILED(result)) { sk_free(comp_types.componentTypes); continue; }

		spatial_component_ comps = spatial_component_none;
		for (uint32_t c = 0; c < comp_types.componentTypeCountOutput; c++)
			comps |= xr_to_sk_comp(comp_types.componentTypes[c]);
		sk_free(comp_types.componentTypes);

		// Persistence means the app can persist things, so it's only listed
		// when there's somewhere to write them.
		if (!local.persist_writable) comps &= ~spatial_component_persistence;

		local.supported |= cap_mappings[map_idx].sk_bit;
		spatial_backend_set_cap_comps(cap_mappings[map_idx].sk_bit, comps);

		// Optional capability features, used during context configuration
		uint32_t feat_count = 0;
		xrEnumerateSpatialCapabilityFeaturesEXT(xr_instance, xr_system_id, xr_caps[i], 0, &feat_count, nullptr);
		if (feat_count > 0) {
			XrSpatialCapabilityFeatureEXT* feats = sk_malloc_t(XrSpatialCapabilityFeatureEXT, feat_count);
			result = xrEnumerateSpatialCapabilityFeaturesEXT(xr_instance, xr_system_id, xr_caps[i], feat_count, &feat_count, feats);
			if (XR_SUCCEEDED(result)) {
				for (uint32_t f = 0; f < feat_count; f++) {
					if      (feats[f] == XR_SPATIAL_CAPABILITY_FEATURE_MARKER_TRACKING_FIXED_SIZE_MARKERS_EXT) local.feat_fixed_size |= cap_mappings[map_idx].sk_bit;
					else if (feats[f] == XR_SPATIAL_CAPABILITY_FEATURE_MARKER_TRACKING_STATIC_MARKERS_EXT    ) local.feat_static     |= cap_mappings[map_idx].sk_bit;
				}
			}
			sk_free(feats);
		}

	}
	sk_free(xr_caps);

	spatial_backend_set_support(local.supported);
	spatial_backend_set_destroy(xr_spatial_destroy);
	if (local.anchor_ext)
		spatial_backend_set_create_anchor(xr_spatial_create_anchor);

	local.available = true;
	return xr_system_succeed;
}

///////////////////////////////////////////

// Checks what persistence support the system has, context creation
// happens later, once permissions are settled.
static void persistence_init() {
	if (!backend_openxr_ext_enabled(XR_EXT_SPATIAL_PERSISTENCE_EXTENSION_NAME))
		return;
	OPENXR_LOAD_FN_RETURN(XR_PERSIST_FUNCTIONS, );

	local.persist_ops_ext = backend_openxr_ext_enabled(XR_EXT_SPATIAL_PERSISTENCE_OPERATIONS_EXTENSION_NAME);
	if (local.persist_ops_ext) {
		OPENXR_LOAD_FN_RETURN(XR_PERSIST_OPS_FUNCTIONS, );
	}

	uint32_t scope_count = 0;
	xrEnumerateSpatialPersistenceScopesEXT(xr_instance, xr_system_id, 0, &scope_count, nullptr);
	if (scope_count == 0)
		return;
	XrSpatialPersistenceScopeEXT* scopes = sk_malloc_t(XrSpatialPersistenceScopeEXT, scope_count);
	XrResult result = xrEnumerateSpatialPersistenceScopesEXT(xr_instance, xr_system_id, scope_count, &scope_count, scopes);
	if (XR_FAILED(result)) {
		log_warnf("%s [%s]", "xrEnumerateSpatialPersistenceScopesEXT", openxr_string(result));
		sk_free(scopes);
		return;
	}

	for (uint32_t i = 0; i < scope_count; i++) {
		XrSpatialPersistenceScopeEXT scope = scopes[i];
		bool writable = scope == XR_SPATIAL_PERSISTENCE_SCOPE_LOCAL_ANCHORS_EXT && local.persist_ops_ext;
		bool usable   = writable || scope == XR_SPATIAL_PERSISTENCE_SCOPE_SYSTEM_MANAGED_EXT;
		if (usable && local.usable_scope_count < 2)
			local.usable_scopes[local.usable_scope_count++] = scope;
		local.persist_writable |= writable;
	}
	sk_free(scopes);
}

// Without writable storage, Persistence comes off every capability, and
// the registry fails anything still waiting on it.
static void persistence_lost() {
	local.persist_writable = false;
	for (int32_t i = 0; i < cap_mapping_count; i++) {
		spatial_capability_ cap = cap_mappings[i].sk_bit;
		spatial_backend_set_cap_comps(cap, spatial_capability_components(cap) & ~spatial_component_persistence);
	}
}

///////////////////////////////////////////

// Persistence storage can be permission gated, so this must not happen
// until permissions have settled.
static void persistence_create_contexts() {
	for (int32_t i = 0; i < local.usable_scope_count; i++) {
		XrSpatialPersistenceScopeEXT scope = local.usable_scopes[i];

		XrSpatialPersistenceContextCreateInfoEXT create_info = { XR_TYPE_SPATIAL_PERSISTENCE_CONTEXT_CREATE_INFO_EXT };
		create_info.scope = scope;
		XrFutureEXT future = XR_NULL_FUTURE_EXT;
		XrResult    result = xrCreateSpatialPersistenceContextAsyncEXT(xr_session, &create_info, &future);
		if (XR_FAILED(result)) {
			log_warnf("%s [%s]", "xrCreateSpatialPersistenceContextAsyncEXT", openxr_string(result));
			if (scope == XR_SPATIAL_PERSISTENCE_SCOPE_LOCAL_ANCHORS_EXT) persistence_lost();
			continue;
		}

		local.persist_pending++;
		xr_ext_future_on_finish(future, [](void* context, XrFutureEXT future) {
			XrSpatialPersistenceScopeEXT scope = (XrSpatialPersistenceScopeEXT)(intptr_t)context;
			local.persist_pending--;

			XrCreateSpatialPersistenceContextCompletionEXT completion = { XR_TYPE_CREATE_SPATIAL_PERSISTENCE_CONTEXT_COMPLETION_EXT };
			XrResult result = xrCreateSpatialPersistenceContextCompleteEXT(xr_session, future, &completion);
			if (XR_FAILED(result) || XR_FAILED(completion.futureResult) || completion.createResult != XR_SPATIAL_PERSISTENCE_CONTEXT_RESULT_SUCCESS_EXT) {
				log_warnf("%s [%s]", "xrCreateSpatialPersistenceContextAsyncEXT", openxr_string(XR_FAILED(result) ? result : completion.futureResult));
				if (scope == XR_SPATIAL_PERSISTENCE_SCOPE_LOCAL_ANCHORS_EXT) persistence_lost();
				return;
			}

			local.persist_contexts[local.persist_context_count++] = completion.persistenceContext;
			if (scope == XR_SPATIAL_PERSISTENCE_SCOPE_LOCAL_ANCHORS_EXT) {
				local.persist_write_ctx = completion.persistenceContext;
				spatial_backend_set_persist_ops(xr_spatial_persist, xr_spatial_unpersist);
			}
		}, (void*)(intptr_t)scope);
	}
}

///////////////////////////////////////////
// Shutdown                              //
///////////////////////////////////////////

static void xr_spatial_shutdown(void*) {
	// The session is going away along with any in-flight futures, so
	// tear everything down directly.
	for (int32_t i = 0; i < cap_mapping_count; i++) {
		slot_t* slot = &local.slots[i];
		slot_clear_handles(slot);
		slot->handles.free();
		if (slot->context != XR_NULL_HANDLE)
			xrDestroySpatialContextEXT(slot->context);
	}
	for (int32_t i = 0; i < local.persist_context_count; i++)
		xrDestroySpatialPersistenceContextEXT(local.persist_contexts[i]);

	spatial_backend_set_support      (spatial_capability_none);
	spatial_backend_set_create_anchor(nullptr);
	spatial_backend_set_destroy      (nullptr);
	spatial_backend_set_persist_ops  (nullptr, nullptr);

	scratch_t* scratches[] = { &local.scratch_ids, &local.scratch_states, &local.scratch_batch, &local.scratch_comp_ids, &local.scratch_comp_states, &local.scratch_comp_data, &local.scratch_entities };
	for (int32_t i = 0; i < (int32_t)(sizeof(scratches) / sizeof(scratches[0])); i++)
		sk_free(scratches[i]->data);
	local = {};

	OPENXR_CLEAR_FN(XR_EXT_FUNCTIONS);
	OPENXR_CLEAR_FN(XR_ANCHOR_FUNCTIONS);
	OPENXR_CLEAR_FN(XR_PERSIST_FUNCTIONS);
	OPENXR_CLEAR_FN(XR_PERSIST_OPS_FUNCTIONS);
}

///////////////////////////////////////////
// Frame stepping                        //
///////////////////////////////////////////

static void xr_spatial_step_begin(void*) {
	// Reconcile requested capabilities with slot states
	spatial_capability_ want_all = spatial_get_enabled() & local.supported;
	for (int32_t i = 0; i < cap_mapping_count; i++) {
		slot_t*             slot = &local.slots[i];
		spatial_capability_ cap  = cap_mappings[i].sk_bit;
		bool want = (want_all & cap) != 0
		         && backend_openxr_ext_enabled(cap_mappings[i].ext_name);

		// A settings change needs a context rebuild, so tear the slot down
		// and let the restart below pick up the new settings.
		if (want && slot->state == slot_state_ready && slot->config_serial != spatial_backend_get_config_serial(cap)) {
			slot_stop(i);
			local.attempted &= ~cap;
		}

		if (want && slot->state == slot_state_off && (local.attempted & cap) == 0) {
			local.attempted |= cap;
			slot_begin_start(i);
		} else if (!want) {
			// Re-arm the attempt flag so a future enable retries
			local.attempted &= ~cap;
			if      (slot->state == slot_state_ready     ) slot_stop(i);
			else if (slot->state == slot_state_creating  ) slot->destroy_after_create = true;
			else if (slot->state == slot_state_permission) slot->state = slot_state_off;
		}
	}

	// Pump slots waiting on permissions or persistence contexts
	for (int32_t i = 0; i < cap_mapping_count; i++) {
		if (local.slots[i].state == slot_state_permission)
			slot_begin_start(i);
	}

	// Look up persist ids apps are waiting on. Storage leaves out ids it
	// can't answer for yet, so this retries until each is resolved.
	float now = time_totalf_unscaled();
	if (now >= local.find_next_time) {
		for (int32_t i = 0; i < cap_mapping_count; i++) {
			slot_t* slot = &local.slots[i];
			if (slot->state != slot_state_ready || slot->discovery_active || (slot->comps & spatial_component_persistence) == 0) continue;

			int32_t count = spatial_backend_get_find_ids(nullptr, 0);
			if (count > 0) {
				sk_uuid_t* ids = sk_malloc_t(sk_uuid_t, count);
				spatial_backend_get_find_ids(ids, count);
				slot_discover(i, ids, count);
				sk_free(ids);
				local.find_next_time = now + 1.0f;
			}
			break;
		}
	}

	// Refresh tracked entities with a synchronous update snapshot
	for (int32_t i = 0; i < cap_mapping_count; i++) {
		if (local.slots[i].state == slot_state_ready && local.slots[i].handles.count > 0)
			slot_update(i);
	}
}

///////////////////////////////////////////
// Event handling                        //
///////////////////////////////////////////

static void xr_spatial_event_poll(void*, XrEventDataBuffer* event_data) {
	if (event_data->type != XR_TYPE_EVENT_DATA_SPATIAL_DISCOVERY_RECOMMENDED_EXT)
		return;

	XrEventDataSpatialDiscoveryRecommendedEXT* recommendation = (XrEventDataSpatialDiscoveryRecommendedEXT*)event_data;
	int32_t slot_idx = slot_from_context(recommendation->spatialContext);
	if (slot_idx < 0 || local.slots[slot_idx].state != slot_state_ready)
		return;

	if (local.slots[slot_idx].discovery_active) local.slots[slot_idx].discovery_queued = true;
	else                                        slot_discover(slot_idx, nullptr, 0);
}

///////////////////////////////////////////
// Slot lifecycle                        //
///////////////////////////////////////////

static permission_type_ cap_permission(spatial_capability_ cap) {
	switch (cap) {
	case spatial_capability_anchor:         return permission_type_anchors;
	case spatial_capability_plane_tracking: return permission_type_plane_tracking;
	default:                                return permission_type_marker_tracking;
	}
}

// Moves a slot toward creation, parking it in slot_state_permission
// while permissions or persistence contexts settle. Safe to call every frame.
static void slot_begin_start(int32_t slot_idx) {
	slot_t*             slot = &local.slots[slot_idx];
	spatial_capability_ cap  = cap_mappings[slot_idx].sk_bit;

	permission_type_  perm_type = cap_permission(cap);
	permission_state_ perm      = permission_state(perm_type);
	if (perm == permission_state_unavailable) {
		log_warnf("Spatial %s tracking's permission is missing from the manifest", cap_mappings[slot_idx].name);
		slot->state = slot_state_off;
		return;
	}
	if (perm == permission_state_capable && (local.requested_perms & (1 << perm_type)) == 0) {
		local.requested_perms |= 1 << perm_type;
		permission_request(&perm_type, 1);
	}
	// A denial may still flip to granted if the app asks again, or the
	// user changes it in settings, so keep waiting rather than giving up.
	if (perm != permission_state_granted && perm != permission_state_unknown) {
		if ((perm == permission_state_denied || perm == permission_state_blocked) && !slot->perm_warned) {
			slot->perm_warned = true;
			log_warnf("Spatial %s tracking is waiting on a denied permission", cap_mappings[slot_idx].name);
		}
		slot->state = slot_state_permission;
		return;
	}
	slot->perm_warned = false;

	// Persistence contexts must be chained at spatial context creation,
	// so persistence capable slots wait for them here.
	if (spatial_capability_components(cap) & spatial_component_persistence) {
		if (!local.persist_attempted) {
			local.persist_attempted = true;
			persistence_create_contexts();
		}
		if (local.persist_pending > 0) {
			slot->state = slot_state_permission;
			return;
		}
	}
	slot_create(slot_idx);
}

///////////////////////////////////////////

static void slot_create(int32_t slot_idx) {
	slot_t* slot = &local.slots[slot_idx];

	// Enable every component the capability supports, though persistence
	// only works when persistence contexts were successfully created.
	slot->comps = spatial_capability_components(cap_mappings[slot_idx].sk_bit);
	if (local.persist_context_count == 0)
		slot->comps &= ~spatial_component_persistence;
	if (slot->comps == spatial_component_none) {
		log_warnf("Spatial %s tracking has no usable components", cap_mappings[slot_idx].name);
		return;
	}

	slot_create_ctx_t* ctx = sk_malloc_zero_t(slot_create_ctx_t, 1);
	ctx->slot_idx = slot_idx;

	int32_t comp_count = 0;
	for (int32_t i = 0; i < comp_mapping_count; i++) {
		if (slot->comps & comp_mappings[i].sk_bit)
			ctx->comp_types[comp_count++] = comp_mappings[i].xr_value;
	}

	XrSpatialCapabilityConfigurationBaseHeaderEXT* cfg = (XrSpatialCapabilityConfigurationBaseHeaderEXT*)&ctx->config;
	cfg->type                  = cap_mappings[slot_idx].config_type;
	cfg->next                  = nullptr;
	cfg->capability            = cap_mappings[slot_idx].xr_value;
	cfg->enabledComponentCount = comp_count;
	cfg->enabledComponents     = ctx->comp_types;
	ctx->config_ptr = cfg;

	// Apply the user's marker configuration. SK enum values match the XR
	// values, with 0 as a "let SK pick" default.
	spatial_capability_ cap         = cap_mappings[slot_idx].sk_bit;
	marker_type_        marker_type = cap_mappings[slot_idx].marker_type;
	if (marker_type != marker_type_none) {
		aruco_dict_     aruco_dict = spatial_get_aruco_dictionary    ();
		april_tag_dict_ april_dict = spatial_get_april_tag_dictionary();
		float           size       = spatial_get_marker_size         (marker_type);

		if (cap == spatial_capability_aruco)
			((XrSpatialCapabilityConfigurationArucoMarkerEXT*)cfg)->arUcoDict = aruco_dict == aruco_dict_default
				? XR_SPATIAL_MARKER_ARUCO_DICT_4X4_50_EXT
				: (XrSpatialMarkerArucoDictEXT)aruco_dict;
		if (cap == spatial_capability_april_tag)
			((XrSpatialCapabilityConfigurationAprilTagEXT*)cfg)->aprilDict = april_dict == april_tag_dict_default
				? XR_SPATIAL_MARKER_APRIL_TAG_DICT_36H11_EXT
				: (XrSpatialMarkerAprilTagDictEXT)april_dict;

		if (size > 0) {
			if (local.feat_fixed_size & cap) {
				ctx->marker_size = { XR_TYPE_SPATIAL_MARKER_SIZE_EXT };
				ctx->marker_size.markerSideLength = size;
				xr_insert_next((XrBaseHeader*)cfg, (XrBaseHeader*)&ctx->marker_size);
			} else log_diagf("Spatial %s tracking doesn't support known marker sizes, ignoring marker size", cap_mappings[slot_idx].name);
		}
		if (spatial_get_marker_stationary(marker_type)) {
			if (local.feat_static & cap) {
				ctx->marker_static = { XR_TYPE_SPATIAL_MARKER_STATIC_OPTIMIZATION_EXT };
				ctx->marker_static.optimizeForStaticMarker = XR_TRUE;
				xr_insert_next((XrBaseHeader*)cfg, (XrBaseHeader*)&ctx->marker_static);
			} else log_diagf("Spatial %s tracking doesn't support stationary marker optimization, ignoring it", cap_mappings[slot_idx].name);
		}
	}
	slot->config_serial = spatial_backend_get_config_serial(cap);

	XrSpatialContextCreateInfoEXT create_info = { XR_TYPE_SPATIAL_CONTEXT_CREATE_INFO_EXT };
	create_info.capabilityConfigCount = 1;
	create_info.capabilityConfigs     = (const XrSpatialCapabilityConfigurationBaseHeaderEXT* const*)&ctx->config_ptr;

	if (slot->comps & spatial_component_persistence) {
		ctx->persist_config = { XR_TYPE_SPATIAL_CONTEXT_PERSISTENCE_CONFIG_EXT };
		memcpy(ctx->persist_contexts, local.persist_contexts, sizeof(local.persist_contexts));
		ctx->persist_config.persistenceContextCount = local.persist_context_count;
		ctx->persist_config.persistenceContexts     = ctx->persist_contexts;
		xr_insert_next((XrBaseHeader*)&create_info, (XrBaseHeader*)&ctx->persist_config);
	}

	XrFutureEXT future = XR_NULL_FUTURE_EXT;
	XrResult    result = xrCreateSpatialContextAsyncEXT(xr_session, &create_info, &future);
	if (XR_FAILED(result)) {
		log_warnf("%s [%s]", "xrCreateSpatialContextAsyncEXT", openxr_string(result));
		sk_free(ctx);
		return;
	}
	slot->state = slot_state_creating;

	xr_ext_future_on_finish(future, [](void* context, XrFutureEXT future) {
		slot_create_ctx_t* ctx  = (slot_create_ctx_t*)context;
		slot_t*            slot = &local.slots[ctx->slot_idx];

		XrCreateSpatialContextCompletionEXT completion = { XR_TYPE_CREATE_SPATIAL_CONTEXT_COMPLETION_EXT };
		XrResult result = xrCreateSpatialContextCompleteEXT(xr_session, future, &completion);
		int32_t  slot_idx = ctx->slot_idx;
		sk_free(ctx);

		if (XR_FAILED(result) || XR_FAILED(completion.futureResult)) {
			log_warnf("%s [%s]", "xrCreateSpatialContextAsyncEXT", openxr_string(XR_FAILED(result) ? result : completion.futureResult));
			slot->state = slot_state_off;
			slot->destroy_after_create = false;
			return;
		}

		slot->context = completion.spatialContext;
		if (slot->destroy_after_create) {
			slot->destroy_after_create = false;
			slot->state = slot_state_ready;
			slot_stop(slot_idx);
			return;
		}

		slot->state = slot_state_ready;
		spatial_backend_set_active(cap_mappings[slot_idx].sk_bit, true);
	}, ctx);
}

///////////////////////////////////////////

static void slot_stop(int32_t slot_idx) {
	slot_t* slot = &local.slots[slot_idx];

	spatial_backend_set_active(cap_mappings[slot_idx].sk_bit, false);
	spatial_backend_drop_source(cap_mappings[slot_idx].sk_bit);
	slot_clear_handles(slot);

	// A discovery future may still reference this context; let its
	// completion callback finish the teardown.
	if (slot->discovery_active) {
		slot->state = slot_state_stopping;
		return;
	}

	if (slot->context != XR_NULL_HANDLE) {
		xrDestroySpatialContextEXT(slot->context);
		slot->context = XR_NULL_HANDLE;
	}
	slot->state            = slot_state_off;
	slot->discovery_queued = false;
	slot->update_warned    = false;
}

///////////////////////////////////////////
// Discovery                             //
///////////////////////////////////////////

// With persist ids, storage reports each one as loaded or not found,
// rather than discovery returning everything.
static void slot_discover(int32_t slot_idx, const sk_uuid_t* persist_ids, int32_t persist_id_count) {
	slot_t* slot = &local.slots[slot_idx];

	XrSpatialDiscoverySnapshotCreateInfoEXT     info   = { XR_TYPE_SPATIAL_DISCOVERY_SNAPSHOT_CREATE_INFO_EXT };
	XrSpatialDiscoveryPersistenceUuidFilterEXT filter = { XR_TYPE_SPATIAL_DISCOVERY_PERSISTENCE_UUID_FILTER_EXT };
	if (persist_id_count > 0) {
		filter.persistedUuidCount = (uint32_t)persist_id_count;
		filter.persistedUuids     = (const XrUuid*)persist_ids;
		info.next = &filter;
	}

	XrFutureEXT future = XR_NULL_FUTURE_EXT;
	XrResult    result = xrCreateSpatialDiscoverySnapshotAsyncEXT(slot->context, &info, &future);
	if (XR_FAILED(result)) {
		log_warnf("%s [%s]", "xrCreateSpatialDiscoverySnapshotAsyncEXT", openxr_string(result));
		return;
	}
	slot->discovery_active = true;

	xr_ext_future_on_finish(future, [](void* context, XrFutureEXT future) {
		int32_t slot_idx = (int32_t)(intptr_t)context;
		slot_t* slot     = &local.slots[slot_idx];
		slot->discovery_active = false;

		XrCreateSpatialDiscoverySnapshotCompletionEXT     completion      = { XR_TYPE_CREATE_SPATIAL_DISCOVERY_SNAPSHOT_COMPLETION_EXT      };
		XrCreateSpatialDiscoverySnapshotCompletionInfoEXT completion_info = { XR_TYPE_CREATE_SPATIAL_DISCOVERY_SNAPSHOT_COMPLETION_INFO_EXT };
		completion_info.baseSpace = xr_app_space;
		completion_info.time      = xr_time;
		completion_info.future    = future;

		XrResult result = xrCreateSpatialDiscoverySnapshotCompleteEXT(slot->context, &completion_info, &completion);

		// Slot was asked to stop while this future was in flight
		if (slot->state == slot_state_stopping) {
			if (XR_SUCCEEDED(result) && XR_SUCCEEDED(completion.futureResult))
				xrDestroySpatialSnapshotEXT(completion.snapshot);
			slot->state = slot_state_ready;
			slot_stop(slot_idx);
			return;
		}

		if (XR_FAILED(result) || XR_FAILED(completion.futureResult)) {
			log_warnf("%s [%s]", "xrCreateSpatialDiscoverySnapshotAsyncEXT", openxr_string(XR_FAILED(result) ? result : completion.futureResult));
			return;
		}

		slot_ingest_snapshot(slot_idx, completion.snapshot, true, slot->comps, -1);
		xrDestroySpatialSnapshotEXT(completion.snapshot);

		if (slot->discovery_queued) {
			slot->discovery_queued = false;
			slot_discover(slot_idx, nullptr, 0);
		}
	}, (void*)(intptr_t)slot_idx);
}

///////////////////////////////////////////
// Per-frame update                      //
///////////////////////////////////////////

static void slot_update(int32_t slot_idx) {
	slot_t* slot = &local.slots[slot_idx];

	XrSpatialEntityEXT* entities = scratch_get<XrSpatialEntityEXT>(&local.scratch_entities, slot->handles.count);
	for (int32_t i = 0; i < slot->handles.count; i++)
		entities[i] = slot->handles[i].handle;

	// Poses change every frame, while other components change rarely, so
	// most updates only ask for the pose carrying components.
	const spatial_component_ pose_comps = spatial_component_anchor | spatial_component_bounds2d | spatial_component_bounds3d;
	spatial_component_       comps      = slot->comps & pose_comps;
	slot->update_count++;
	if (comps == spatial_component_none || slot->update_count % 30 == 0)
		comps = slot->comps;

	XrSpatialComponentTypeEXT comp_types[comp_mapping_count];
	int32_t                   comp_count = 0;
	for (int32_t i = 0; i < comp_mapping_count; i++) {
		if (comps & comp_mappings[i].sk_bit)
			comp_types[comp_count++] = comp_mappings[i].xr_value;
	}

	XrSpatialUpdateSnapshotCreateInfoEXT create_info = { XR_TYPE_SPATIAL_UPDATE_SNAPSHOT_CREATE_INFO_EXT };
	create_info.entityCount        = slot->handles.count;
	create_info.entities           = entities;
	create_info.componentTypeCount = comp_count;
	create_info.componentTypes     = comp_types;
	create_info.baseSpace          = xr_app_space;
	create_info.time               = xr_time;

	XrSpatialSnapshotEXT xr_snapshot = XR_NULL_HANDLE;
	XrResult             result      = xrCreateSpatialUpdateSnapshotEXT(slot->context, &create_info, &xr_snapshot);

	if (XR_FAILED(result)) {
		if (!slot->update_warned)
			log_warnf("%s [%s]", "xrCreateSpatialUpdateSnapshotEXT", openxr_string(result));
		slot->update_warned = true;
		return;
	}

	slot_ingest_snapshot(slot_idx, xr_snapshot, false, comps, slot->handles.count);
	xrDestroySpatialSnapshotEXT(xr_snapshot);
}

///////////////////////////////////////////
// Snapshot ingestion                    //
///////////////////////////////////////////

// Fetch a vec3 buffer from the snapshot, returns null when empty.
static vec3* fetch_buffer_v3(XrSpatialSnapshotEXT snapshot, XrSpatialBufferIdEXT id, int32_t* out_count) {
	XrSpatialBufferGetInfoEXT info = { XR_TYPE_SPATIAL_BUFFER_GET_INFO_EXT };
	info.bufferId = id;
	uint32_t count = 0;
	xrGetSpatialBufferVector3fEXT(snapshot, &info, 0, &count, nullptr);
	*out_count = (int32_t)count;
	if (count == 0) return nullptr;
	vec3* result = sk_malloc_t(vec3, count);
	xrGetSpatialBufferVector3fEXT(snapshot, &info, count, &count, (XrVector3f*)result);
	return result;
}

static vec2* fetch_buffer_v2(XrSpatialSnapshotEXT snapshot, XrSpatialBufferIdEXT id, int32_t* out_count) {
	XrSpatialBufferGetInfoEXT info = { XR_TYPE_SPATIAL_BUFFER_GET_INFO_EXT };
	info.bufferId = id;
	uint32_t count = 0;
	xrGetSpatialBufferVector2fEXT(snapshot, &info, 0, &count, nullptr);
	*out_count = (int32_t)count;
	if (count == 0) return nullptr;
	vec2* result = sk_malloc_t(vec2, count);
	xrGetSpatialBufferVector2fEXT(snapshot, &info, count, &count, (XrVector2f*)result);
	return result;
}

static uint32_t* fetch_buffer_u32(XrSpatialSnapshotEXT snapshot, XrSpatialBufferIdEXT id, int32_t* out_count) {
	XrSpatialBufferGetInfoEXT info = { XR_TYPE_SPATIAL_BUFFER_GET_INFO_EXT };
	info.bufferId = id;
	uint32_t count = 0;
	xrGetSpatialBufferUint32EXT(snapshot, &info, 0, &count, nullptr);
	*out_count = (int32_t)count;
	if (count == 0) return nullptr;
	uint32_t* result = sk_malloc_t(uint32_t, count);
	xrGetSpatialBufferUint32EXT(snapshot, &info, count, &count, result);
	return result;
}

// uint16 index buffers are widened to uint32 for the registry.
static uint32_t* fetch_buffer_u16_widen(XrSpatialSnapshotEXT snapshot, XrSpatialBufferIdEXT id, int32_t* out_count) {
	XrSpatialBufferGetInfoEXT info = { XR_TYPE_SPATIAL_BUFFER_GET_INFO_EXT };
	info.bufferId = id;
	uint32_t count = 0;
	xrGetSpatialBufferUint16EXT(snapshot, &info, 0, &count, nullptr);
	*out_count = (int32_t)count;
	if (count == 0) return nullptr;
	uint16_t* raw = sk_malloc_t(uint16_t, count);
	xrGetSpatialBufferUint16EXT(snapshot, &info, count, &count, raw);
	uint32_t* result = sk_malloc_t(uint32_t, count);
	for (uint32_t i = 0; i < count; i++) result[i] = raw[i];
	sk_free(raw);
	return result;
}

///////////////////////////////////////////

// Pushes a snapshot's entities into the registry. create_handles is for
// discovery snapshots, where unseen ids need handles for frame updates.
static void slot_ingest_snapshot(int32_t slot_idx, XrSpatialSnapshotEXT xr_snapshot, bool create_handles, spatial_component_ comps, int32_t max_entities) {
	slot_t*             slot = &local.slots[slot_idx];
	spatial_capability_ cap  = cap_mappings[slot_idx].sk_bit;

	// Phase 1: all entity ids and tracking states. Callers that know an
	// upper bound for the entity count skip the sizing call.
	XrSpatialComponentDataQueryConditionEXT condition   = { XR_TYPE_SPATIAL_COMPONENT_DATA_QUERY_CONDITION_EXT };
	XrSpatialComponentDataQueryResultEXT    result_info = { XR_TYPE_SPATIAL_COMPONENT_DATA_QUERY_RESULT_EXT };
	XrResult result;
	if (max_entities < 0) {
		result = xrQuerySpatialComponentDataEXT(xr_snapshot, &condition, &result_info);
		if (XR_FAILED(result)) { log_warnf("%s [%s]", "xrQuerySpatialComponentDataEXT", openxr_string(result)); return; }
		max_entities = (int32_t)result_info.entityIdCountOutput;
	}
	if (max_entities == 0) return;

	XrSpatialEntityIdEXT*            ids    = scratch_get<XrSpatialEntityIdEXT>           (&local.scratch_ids,    max_entities);
	XrSpatialEntityTrackingStateEXT* states = scratch_get<XrSpatialEntityTrackingStateEXT>(&local.scratch_states, max_entities);
	result_info.entityIdCapacityInput    = max_entities;
	result_info.entityIds                = ids;
	result_info.entityStateCapacityInput = max_entities;
	result_info.entityStates             = states;

	result = xrQuerySpatialComponentDataEXT(xr_snapshot, &condition, &result_info);
	if (XR_FAILED(result)) {
		log_warnf("%s [%s]", "xrQuerySpatialComponentDataEXT", openxr_string(result));
		return;
	}
	int32_t count = (int32_t)result_info.entityIdCountOutput;
	if (count == 0) return;

	// Only entities with a handle are ingested, since frame updates can't
	// reach the others, and an unknown stopped entity needs no record.
	spatial_ingest_t* batch       = scratch_get<spatial_ingest_t>(&local.scratch_batch, count);
	int32_t           batch_count = 0;
	for (int32_t i = 0; i < count; i++) {
		spatial_tracking_ tracking = xr_to_sk_tracking(states[i]);
		ent_handle_t*     handle   = slot_find_handle(slot, ids[i]);
		if (handle == nullptr && create_handles && tracking != spatial_tracking_stopped) {
			XrSpatialEntityFromIdCreateInfoEXT create_info = { XR_TYPE_SPATIAL_ENTITY_FROM_ID_CREATE_INFO_EXT };
			create_info.entityId = ids[i];
			ent_handle_t created = {};
			created.id = ids[i];
			if (XR_SUCCEEDED(xrCreateSpatialEntityFromIdEXT(slot->context, &create_info, &created.handle))) {
				slot_add_handle(slot, created);
				handle = slot_find_handle(slot, ids[i]);
			}
		}
		if (handle == nullptr) continue;

		handle->batch_idx = batch_count;
		batch[batch_count].entity   = handle->entity;
		batch[batch_count].id       = (spatial_entity_id_t)ids[i];
		batch[batch_count].tracking = tracking;
		batch_count++;
	}

	// Phase 2: per-component queries. Only entities that are actively
	// tracking get data ingested; the spec leaves buffers untouched for
	// paused/stopped entities.
	for (int32_t c = 0; c < comp_mapping_count; c++) {
		spatial_component_ comp_bit = comp_mappings[c].sk_bit;
		if ((comps & comp_bit) == 0) continue;

		XrSpatialComponentTypeEXT xr_comp = comp_mappings[c].xr_value;
		condition.componentTypeCount = 1;
		condition.componentTypes     = &xr_comp;

		// The phase 1 entity count is an upper bound for every component,
		// so a sizing call isn't needed here.
		int32_t comp_count = count;
		XrSpatialEntityIdEXT*            comp_ids    = scratch_get<XrSpatialEntityIdEXT>           (&local.scratch_comp_ids,    comp_count);
		XrSpatialEntityTrackingStateEXT* comp_states = scratch_get<XrSpatialEntityTrackingStateEXT>(&local.scratch_comp_states, comp_count);
		scratch_t*                       comp_data   = &local.scratch_comp_data;
		XrSpatialComponentDataQueryResultEXT comp_result = { XR_TYPE_SPATIAL_COMPONENT_DATA_QUERY_RESULT_EXT };
		comp_result.entityIdCapacityInput    = comp_count;
		comp_result.entityIds                = comp_ids;
		comp_result.entityStateCapacityInput = comp_count;
		comp_result.entityStates             = comp_states;

		// Component-specific list structs, chained by component type
		XrSpatialComponentBounded2DListEXT          list_b2d    = { XR_TYPE_SPATIAL_COMPONENT_BOUNDED_2D_LIST_EXT };
		XrSpatialComponentBounded3DListEXT          list_b3d    = { XR_TYPE_SPATIAL_COMPONENT_BOUNDED_3D_LIST_EXT };
		XrSpatialComponentParentListEXT             list_parent = { XR_TYPE_SPATIAL_COMPONENT_PARENT_LIST_EXT };
		XrSpatialComponentMesh3DListEXT             list_mesh   = { XR_TYPE_SPATIAL_COMPONENT_MESH_3D_LIST_EXT };
		XrSpatialComponentAnchorListEXT             list_anchor = { XR_TYPE_SPATIAL_COMPONENT_ANCHOR_LIST_EXT };
		XrSpatialComponentPlaneAlignmentListEXT     list_align  = { XR_TYPE_SPATIAL_COMPONENT_PLANE_ALIGNMENT_LIST_EXT };
		XrSpatialComponentMesh2DListEXT             list_mesh2d = { XR_TYPE_SPATIAL_COMPONENT_MESH_2D_LIST_EXT };
		XrSpatialComponentPolygon2DListEXT          list_poly   = { XR_TYPE_SPATIAL_COMPONENT_POLYGON_2D_LIST_EXT };
		XrSpatialComponentPlaneSemanticLabelListEXT list_label  = { XR_TYPE_SPATIAL_COMPONENT_PLANE_SEMANTIC_LABEL_LIST_EXT };
		XrSpatialComponentMarkerListEXT             list_marker = { XR_TYPE_SPATIAL_COMPONENT_MARKER_LIST_EXT };
		XrSpatialComponentPersistenceListEXT        list_persist= { XR_TYPE_SPATIAL_COMPONENT_PERSISTENCE_LIST_EXT };

		XrSpatialBounded2DDataEXT*      d_b2d    = nullptr;
		XrBoxf*                         d_b3d    = nullptr;
		XrSpatialEntityIdEXT*           d_parent = nullptr;
		XrSpatialMeshDataEXT*           d_mesh   = nullptr;
		XrPosef*                        d_anchor = nullptr;
		XrSpatialPlaneAlignmentEXT*     d_align  = nullptr;
		XrSpatialMeshDataEXT*           d_mesh2d = nullptr;
		XrSpatialPolygon2DDataEXT*      d_poly   = nullptr;
		XrSpatialPlaneSemanticLabelEXT* d_label  = nullptr;
		XrSpatialMarkerDataEXT*         d_marker = nullptr;
		XrSpatialPersistenceDataEXT*    d_persist= nullptr;

		switch (comp_bit) {
		case spatial_component_bounds2d:
			d_b2d = scratch_get<XrSpatialBounded2DDataEXT>(comp_data, comp_count);
			list_b2d.boundCount = comp_count; list_b2d.bounds = d_b2d;
			xr_insert_next((XrBaseHeader*)&comp_result, (XrBaseHeader*)&list_b2d);
			break;
		case spatial_component_bounds3d:
			d_b3d = scratch_get<XrBoxf>(comp_data, comp_count);
			list_b3d.boundCount = comp_count; list_b3d.bounds = d_b3d;
			xr_insert_next((XrBaseHeader*)&comp_result, (XrBaseHeader*)&list_b3d);
			break;
		case spatial_component_parent:
			d_parent = scratch_get<XrSpatialEntityIdEXT>(comp_data, comp_count);
			list_parent.parentCount = comp_count; list_parent.parents = d_parent;
			xr_insert_next((XrBaseHeader*)&comp_result, (XrBaseHeader*)&list_parent);
			break;
		case spatial_component_mesh:
			d_mesh = scratch_get<XrSpatialMeshDataEXT>(comp_data, comp_count);
			list_mesh.meshCount = comp_count; list_mesh.meshes = d_mesh;
			xr_insert_next((XrBaseHeader*)&comp_result, (XrBaseHeader*)&list_mesh);
			break;
		case spatial_component_anchor:
			d_anchor = scratch_get<XrPosef>(comp_data, comp_count);
			list_anchor.locationCount = comp_count; list_anchor.locations = d_anchor;
			xr_insert_next((XrBaseHeader*)&comp_result, (XrBaseHeader*)&list_anchor);
			break;
		case spatial_component_plane_alignment:
			d_align = scratch_get<XrSpatialPlaneAlignmentEXT>(comp_data, comp_count);
			list_align.planeAlignmentCount = comp_count; list_align.planeAlignments = d_align;
			xr_insert_next((XrBaseHeader*)&comp_result, (XrBaseHeader*)&list_align);
			break;
		case spatial_component_mesh2d:
			d_mesh2d = scratch_get<XrSpatialMeshDataEXT>(comp_data, comp_count);
			list_mesh2d.meshCount = comp_count; list_mesh2d.meshes = d_mesh2d;
			xr_insert_next((XrBaseHeader*)&comp_result, (XrBaseHeader*)&list_mesh2d);
			break;
		case spatial_component_polygon:
			d_poly = scratch_get<XrSpatialPolygon2DDataEXT>(comp_data, comp_count);
			list_poly.polygonCount = comp_count; list_poly.polygons = d_poly;
			xr_insert_next((XrBaseHeader*)&comp_result, (XrBaseHeader*)&list_poly);
			break;
		case spatial_component_label:
			d_label = scratch_get<XrSpatialPlaneSemanticLabelEXT>(comp_data, comp_count);
			list_label.semanticLabelCount = comp_count; list_label.semanticLabels = d_label;
			xr_insert_next((XrBaseHeader*)&comp_result, (XrBaseHeader*)&list_label);
			break;
		case spatial_component_marker:
			d_marker = scratch_get<XrSpatialMarkerDataEXT>(comp_data, comp_count);
			list_marker.markerCount = comp_count; list_marker.markers = d_marker;
			xr_insert_next((XrBaseHeader*)&comp_result, (XrBaseHeader*)&list_marker);
			break;
		case spatial_component_persistence:
			d_persist = scratch_get<XrSpatialPersistenceDataEXT>(comp_data, comp_count);
			list_persist.persistDataCount = comp_count; list_persist.persistData = d_persist;
			xr_insert_next((XrBaseHeader*)&comp_result, (XrBaseHeader*)&list_persist);
			break;
		default: break;
		}

		result     = xrQuerySpatialComponentDataEXT(xr_snapshot, &condition, &comp_result);
		comp_count = (int32_t)comp_result.entityIdCountOutput;
		if (XR_SUCCEEDED(result)) {
			for (int32_t i = 0; i < comp_count; i++) {
				// Not-found ids come without an entity, so they bypass the batch
				if (comp_bit == spatial_component_persistence && d_persist[i].persistState == XR_SPATIAL_PERSISTENCE_STATE_NOT_FOUND_EXT) {
					spatial_backend_persist_not_found(xr_to_uuid(d_persist[i].persistUuid));
					continue;
				}
				ent_handle_t* handle = slot_find_handle(slot, comp_ids[i]);
				if (handle == nullptr || handle->batch_idx >= batch_count || batch[handle->batch_idx].id != (spatial_entity_id_t)comp_ids[i]) continue;
				spatial_ingest_t* in = &batch[handle->batch_idx];
				if (in->tracking != spatial_tracking_tracking && comp_bit != spatial_component_persistence) continue;
				in->present |= comp_bit;

				switch (comp_bit) {
				case spatial_component_bounds2d:
					in->bounds2d_center = xr_to_pose_faced(d_b2d[i].center);
					in->bounds2d_size   = { d_b2d[i].extents.width, d_b2d[i].extents.height };
					break;
				case spatial_component_bounds3d:
					in->bounds3d_center = xr_to_pose_faced(d_b3d[i].center);
					in->bounds3d_size   = { d_b3d[i].extents.width, d_b3d[i].extents.height, d_b3d[i].extents.depth };
					break;
				case spatial_component_parent:
					in->parent = (spatial_entity_id_t)d_parent[i];
					break;
				case spatial_component_anchor:
					in->anchor_pose = xr_to_pose(d_anchor[i]);
					break;
				case spatial_component_plane_alignment:
					in->plane_alignment = xr_to_plane_align(d_align[i]);
					break;
				case spatial_component_label:
					in->label = xr_to_plane_label(d_label[i]);
					break;
				case spatial_component_mesh:
					in->mesh.origin = xr_to_pose(d_mesh[i].origin);
					if (handle->stamp_mesh_v != d_mesh[i].vertexBuffer.bufferId || handle->stamp_mesh_i != d_mesh[i].indexBuffer.bufferId) {
						handle->stamp_mesh_v = d_mesh[i].vertexBuffer.bufferId;
						handle->stamp_mesh_i = d_mesh[i].indexBuffer.bufferId;
						in->mesh.verts = fetch_buffer_v3 (xr_snapshot, d_mesh[i].vertexBuffer.bufferId, &in->mesh.vert_count);
						in->mesh.inds  = fetch_buffer_u32(xr_snapshot, d_mesh[i].indexBuffer .bufferId, &in->mesh.ind_count);
						in->buffers_changed |= spatial_component_mesh;
					}
					break;
				case spatial_component_mesh2d:
					in->mesh2d.origin = xr_to_pose_faced(d_mesh2d[i].origin);
					if (handle->stamp_mesh2d_v != d_mesh2d[i].vertexBuffer.bufferId || handle->stamp_mesh2d_i != d_mesh2d[i].indexBuffer.bufferId) {
						handle->stamp_mesh2d_v = d_mesh2d[i].vertexBuffer.bufferId;
						handle->stamp_mesh2d_i = d_mesh2d[i].indexBuffer.bufferId;
						// 2D mesh vertices are vec2, widen to vec3 with z=0
						int32_t v2_count = 0;
						vec2*   v2       = fetch_buffer_v2(xr_snapshot, d_mesh2d[i].vertexBuffer.bufferId, &v2_count);
						if (v2) {
							in->mesh2d.verts      = sk_malloc_t(vec3, v2_count);
							in->mesh2d.vert_count = v2_count;
							for (int32_t v = 0; v < v2_count; v++)
								in->mesh2d.verts[v] = { -v2[v].x, v2[v].y, 0 };
							sk_free(v2);
						}
						in->mesh2d.inds = fetch_buffer_u16_widen(xr_snapshot, d_mesh2d[i].indexBuffer.bufferId, &in->mesh2d.ind_count);
						in->buffers_changed |= spatial_component_mesh2d;
					}
					break;
				case spatial_component_polygon:
					in->polygon_origin = xr_to_pose_faced(d_poly[i].origin);
					if (handle->stamp_polygon != d_poly[i].vertexBuffer.bufferId) {
						handle->stamp_polygon = d_poly[i].vertexBuffer.bufferId;
						in->polygon_verts = fetch_buffer_v2(xr_snapshot, d_poly[i].vertexBuffer.bufferId, &in->polygon_count);
						for (int32_t v = 0; v < in->polygon_count; v++)
							in->polygon_verts[v].x = -in->polygon_verts[v].x;
						in->buffers_changed |= spatial_component_polygon;
					}
					break;
				case spatial_component_marker:
					in->marker_type = cap_mappings[slot_idx].marker_type;
					in->marker_id   = d_marker[i].markerId;
					if (handle->stamp_marker != d_marker[i].data.bufferId) {
						handle->stamp_marker = d_marker[i].data.bufferId;
						if (d_marker[i].data.bufferId != XR_NULL_SPATIAL_BUFFER_ID_EXT) {
							XrSpatialBufferGetInfoEXT buf_info = { XR_TYPE_SPATIAL_BUFFER_GET_INFO_EXT };
							buf_info.bufferId = d_marker[i].data.bufferId;
							if (d_marker[i].data.bufferType == XR_SPATIAL_BUFFER_TYPE_STRING_EXT) {
								uint32_t len = 0;
								xrGetSpatialBufferStringEXT(xr_snapshot, &buf_info, 0, &len, nullptr);
								if (len > 0) {
									in->marker_text = sk_malloc_t(char, len);
									xrGetSpatialBufferStringEXT(xr_snapshot, &buf_info, len, &len, in->marker_text);
								}
							} else if (d_marker[i].data.bufferType == XR_SPATIAL_BUFFER_TYPE_UINT8_EXT) {
								uint32_t len = 0;
								xrGetSpatialBufferUint8EXT(xr_snapshot, &buf_info, 0, &len, nullptr);
								if (len > 0) {
									in->marker_data      = sk_malloc_t(uint8_t, len);
									in->marker_data_size = (int32_t)len;
									xrGetSpatialBufferUint8EXT(xr_snapshot, &buf_info, len, &len, in->marker_data);
								}
							}
						}
						in->buffers_changed |= spatial_component_marker;
					}
					break;
				case spatial_component_persistence:
					in->persist_id = xr_to_uuid(d_persist[i].persistUuid);
					break;
				default: break;
				}
			}
		}

	}

	spatial_backend_ingest(cap, batch, batch_count);

	// Keep the registry's handles for next time, and drop stopped entities,
	// since they never come back.
	for (int32_t i = 0; i < batch_count; i++) {
		if (batch[i].tracking == spatial_tracking_stopped) slot_remove_handle(slot, (XrSpatialEntityIdEXT)batch[i].id);
		else                                               slot_find_handle  (slot, (XrSpatialEntityIdEXT)batch[i].id)->entity = batch[i].entity;
	}

}

///////////////////////////////////////////
// Anchor creation                       //
///////////////////////////////////////////

static spatial_entity_id_t xr_spatial_create_anchor(pose_t pose, spatial_entity_id_t parent, spatial_entity_t entity) {
	if (parent != 0) {
		log_warn("spatial_entity_create_anchor: this runtime can't attach anchors to a parent");
		return 0;
	}

	slot_t* slot = slot_from_cap(spatial_capability_anchor);
	if (slot->state != slot_state_ready) return 0;

	XrSpatialAnchorCreateInfoEXT create_info = { XR_TYPE_SPATIAL_ANCHOR_CREATE_INFO_EXT };
	create_info.baseSpace = xr_app_space;
	create_info.time      = xr_time;
	create_info.pose      = pose_to_xr(pose);

	XrSpatialEntityIdEXT anchor_id     = XR_NULL_SPATIAL_ENTITY_ID_EXT;
	XrSpatialEntityEXT   anchor_entity = XR_NULL_HANDLE;
	XrResult result = xrCreateSpatialAnchorEXT(slot->context, &create_info, &anchor_id, &anchor_entity);
	if (XR_FAILED(result)) {
		log_warnf("%s [%s]", "xrCreateSpatialAnchorEXT", openxr_string(result));
		return 0;
	}

	ent_handle_t handle = {};
	handle.id     = anchor_id;
	handle.handle = anchor_entity;
	handle.entity = entity;
	slot_add_handle(slot, handle);
	return (spatial_entity_id_t)anchor_id;
}

///////////////////////////////////////////

// The ready slot that has a handle for this entity, if any.
static slot_t* slot_from_entity(spatial_entity_id_t id) {
	for (int32_t i = 0; i < cap_mapping_count; i++) {
		slot_t* slot = &local.slots[i];
		if (slot->state == slot_state_ready && slot_find_handle(slot, (XrSpatialEntityIdEXT)id) != nullptr) return slot;
	}
	return nullptr;
}

// Releases the runtime's handle for an app-created entity. For
// non-persisted anchors, this lets the runtime stop tracking them.
static void xr_spatial_destroy(spatial_entity_id_t id) {
	slot_t* slot = slot_from_entity(id);
	if (slot != nullptr) slot_remove_handle(slot, (XrSpatialEntityIdEXT)id);
}

///////////////////////////////////////////
// Persistence operations                //
///////////////////////////////////////////

static void xr_spatial_persist(spatial_entity_id_t id, spatial_entity_t entity) {
	if (local.persist_write_ctx == XR_NULL_HANDLE) { spatial_backend_persist_failed(entity); return; }

	slot_t* slot = slot_from_entity(id);
	if (slot == nullptr) {
		log_warn("spatial_entity_persist: entity not found");
		spatial_backend_persist_failed(entity);
		return;
	}
	XrSpatialContextEXT context = slot->context;

	XrSpatialEntityPersistInfoEXT info = { XR_TYPE_SPATIAL_ENTITY_PERSIST_INFO_EXT };
	info.spatialContext  = context;
	info.spatialEntityId = (XrSpatialEntityIdEXT)id;

	XrFutureEXT future = XR_NULL_FUTURE_EXT;
	XrResult    result = xrPersistSpatialEntityAsyncEXT(local.persist_write_ctx, &info, &future);
	if (XR_FAILED(result)) {
		log_warnf("%s [%s]", "xrPersistSpatialEntityAsyncEXT", openxr_string(result));
		spatial_backend_persist_failed(entity);
		return;
	}

	xr_ext_future_on_finish(future, [](void* context, XrFutureEXT future) {
		spatial_entity_t entity = (spatial_entity_t)(uintptr_t)context;

		XrPersistSpatialEntityCompletionEXT completion = { XR_TYPE_PERSIST_SPATIAL_ENTITY_COMPLETION_EXT };
		XrResult result = xrPersistSpatialEntityCompleteEXT(local.persist_write_ctx, future, &completion);
		if (XR_FAILED(result) || XR_FAILED(completion.futureResult) || completion.persistResult != XR_SPATIAL_PERSISTENCE_CONTEXT_RESULT_SUCCESS_EXT) {
			log_warnf("Persisting a spatial entity failed [%s, result %d]", openxr_string(XR_FAILED(result) ? result : completion.futureResult), (int)completion.persistResult);
			spatial_backend_persist_failed(entity);
			return;
		}
		spatial_backend_set_persist(entity, xr_to_uuid(completion.persistUuid));
	}, (void*)(uintptr_t)entity);
}

///////////////////////////////////////////

static void xr_spatial_unpersist(spatial_entity_t entity, sk_uuid_t persist_id) {
	if (local.persist_write_ctx == XR_NULL_HANDLE) { spatial_backend_unpersist_failed(entity); return; }

	XrSpatialEntityUnpersistInfoEXT info = { XR_TYPE_SPATIAL_ENTITY_UNPERSIST_INFO_EXT };
	info.persistUuid = uuid_to_xr(persist_id);

	XrFutureEXT future = XR_NULL_FUTURE_EXT;
	XrResult    result = xrUnpersistSpatialEntityAsyncEXT(local.persist_write_ctx, &info, &future);
	if (XR_FAILED(result)) {
		log_warnf("%s [%s]", "xrUnpersistSpatialEntityAsyncEXT", openxr_string(result));
		spatial_backend_unpersist_failed(entity);
		return;
	}

	xr_ext_future_on_finish(future, [](void* context, XrFutureEXT future) {
		spatial_entity_t entity = (spatial_entity_t)(uintptr_t)context;

		XrUnpersistSpatialEntityCompletionEXT completion = { XR_TYPE_UNPERSIST_SPATIAL_ENTITY_COMPLETION_EXT };
		XrResult result = xrUnpersistSpatialEntityCompleteEXT(local.persist_write_ctx, future, &completion);
		if (XR_FAILED(result) || XR_FAILED(completion.futureResult) || completion.unpersistResult != XR_SPATIAL_PERSISTENCE_CONTEXT_RESULT_SUCCESS_EXT) {
			log_warnf("Unpersisting a spatial entity failed [%s, result %d]", openxr_string(XR_FAILED(result) ? result : completion.futureResult), (int)completion.unpersistResult);
			spatial_backend_unpersist_failed(entity);
			return;
		}
		spatial_backend_clear_persist(entity);
	}, (void*)(uintptr_t)entity);
}

}
