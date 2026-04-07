/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#pragma once

#include "../../stereokit.h"
#include "../../libraries/array.h"

namespace sk {

///////////////////////////////////////////
// Types                                 //
///////////////////////////////////////////

// Opaque ID for a spatial entity within a spatial context. Valid only for the
// lifetime of that context. Reconfiguring capabilities invalidates all
// previous IDs.
typedef uint64_t spatial_entity_id;

// Tracking state mirrors XrSpatialEntityTrackingStateEXT, decoupled from
// the OpenXR header so non-XR code can use it.
typedef enum spatial_tracking_ {
	spatial_tracking_stopped  = 1, // Lost permanently
	spatial_tracking_paused   = 2, // Temporarily not tracked
	spatial_tracking_tracking = 3, // Actively tracked, data is valid
} spatial_tracking_;

// Bitmask of spatial capabilities. Each bit maps to one
// XrSpatialCapabilityEXT value.
typedef enum spatial_capability_ {
	spatial_capability_none           = 0,
	spatial_capability_anchor         = 1 << 0,
	spatial_capability_plane_tracking = 1 << 1,
	spatial_capability_qr_code        = 1 << 2,
	spatial_capability_micro_qr       = 1 << 3,
	spatial_capability_aruco          = 1 << 4,
	spatial_capability_april_tag      = 1 << 5,
} spatial_capability_;

// Bitmask of component types. Each bit maps to one
// XrSpatialComponentTypeEXT value.
typedef enum spatial_component_ {
	spatial_component_none            = 0,
	spatial_component_bounded_2d      = 1 << 0,
	spatial_component_bounded_3d      = 1 << 1,
	spatial_component_parent          = 1 << 2,
	spatial_component_mesh_3d         = 1 << 3,
	spatial_component_anchor          = 1 << 4,
	spatial_component_persistence     = 1 << 5,
	spatial_component_plane_alignment = 1 << 6,
	spatial_component_mesh_2d         = 1 << 7,
	spatial_component_polygon_2d      = 1 << 8,
	spatial_component_plane_label     = 1 << 9,
	spatial_component_marker          = 1 << 10,
} spatial_component_;

typedef enum spatial_plane_align_ {
	spatial_plane_align_horizontal_up   = 0,
	spatial_plane_align_horizontal_down = 1,
	spatial_plane_align_vertical        = 2,
	spatial_plane_align_arbitrary       = 3,
} spatial_plane_align_;

typedef enum spatial_plane_label_ {
	spatial_plane_label_uncategorized = 1,
	spatial_plane_label_floor         = 2,
	spatial_plane_label_wall          = 3,
	spatial_plane_label_ceiling       = 4,
	spatial_plane_label_table         = 5,
} spatial_plane_label_;

// Which marker capability detected this marker.
typedef enum spatial_marker_type_ {
	spatial_marker_type_qr_code  = 0,
	spatial_marker_type_micro_qr = 1,
	spatial_marker_type_aruco    = 2,
	spatial_marker_type_april_tag= 3,
} spatial_marker_type_;

///////////////////////////////////////////
// Component data (POD, no constructors) //
///////////////////////////////////////////

struct spatial_bounds_2d_t {
	pose_t center;
	vec2   extents;
};

struct spatial_bounds_3d_t {
	pose_t center;
	vec3   extents;
};

struct spatial_mesh_t {
	pose_t    origin;
	vec3*     vertices;
	int32_t   vertex_count;
	uint32_t* indices;
	int32_t   index_count;
};

struct spatial_polygon_2d_t {
	pose_t  origin;
	vec2*   vertices;
	int32_t vertex_count;
};

struct spatial_marker_t {
	spatial_marker_type_ type;
	uint32_t             marker_id; // ArUco/AprilTag ID, 0 for QR codes
	const char*          data;      // Decoded string data (QR/MicroQR), null otherwise
	const uint8_t*       data_raw;  // Raw decoded bytes (QR/MicroQR), null otherwise
	int32_t              data_size; // Size of data_raw in bytes
};

///////////////////////////////////////////
// Entity + snapshot results             //
///////////////////////////////////////////

// Per-entity record. Check `components` bitmask before reading a
// component slot at this entity's index.
struct spatial_entity_t {
	spatial_entity_id  id;
	spatial_tracking_  tracking;
	spatial_component_ components; // Bitmask of which components are valid
};

// Snapshot result. All arrays are allocated by the spatial entity system.
// Call spatial_snapshot_release to free.
//
// Component arrays are non-null if ANY entity in the snapshot has that
// component. Arrays are always entity_count length. Check
// entities[i].components before reading component data at index i —
// slots where the entity lacks that component are zero-initialized.
struct spatial_snapshot_t {
	spatial_entity_t* entities;
	int32_t           entity_count;

	// Component data arrays, parallel with entities[].
	spatial_bounds_2d_t*   bounds_2d;       // spatial_component_bounded_2d
	spatial_bounds_3d_t*   bounds_3d;       // spatial_component_bounded_3d
	spatial_entity_id*     parents;         // spatial_component_parent
	spatial_mesh_t*        meshes_3d;       // spatial_component_mesh_3d
	pose_t*                anchor_poses;    // spatial_component_anchor
	spatial_plane_align_*  plane_alignments;// spatial_component_plane_alignment
	spatial_mesh_t*        meshes_2d;       // spatial_component_mesh_2d
	spatial_polygon_2d_t*  polygons_2d;     // spatial_component_polygon_2d
	spatial_plane_label_*  plane_labels;    // spatial_component_plane_label
	spatial_marker_t*      markers;         // spatial_component_marker
};

///////////////////////////////////////////
// Discovery callback                    //
///////////////////////////////////////////

// Fired when a discovery snapshot completes. Called on the main thread
// during step_begin. The snapshot is owned by the callee — it must call
// spatial_snapshot_release when done with it.
typedef void (*spatial_on_discover_t)(const spatial_snapshot_t* snapshot, void* context);

///////////////////////////////////////////
// Lifecycle                             //
///////////////////////////////////////////

// Is the XR_EXT_spatial_entity extension available on this runtime?
// True after initialization even before start() is called.
bool spatial_entity_available();

// What capabilities does the runtime support? Usable any time after
// spatial_entity_available() returns true, before calling start().
spatial_capability_ spatial_entity_supported_capabilities();

// What components does the runtime support for a given capability?
// Usable any time after spatial_entity_available() returns true.
spatial_component_ spatial_entity_supported_components(spatial_capability_ capability);

// Start tracking with the given capabilities and components. Creates an
// XrSpatialContextEXT asynchronously. Discovery results will arrive via
// on_discover once the context is ready and the runtime recommends
// discovery.
//
// Calling start again with different parameters destroys the old context
// and creates a new one — all previous entity IDs become invalid.
// The on_discover callback will begin firing for the new configuration
// once the new context is ready.
//
// Components are automatically assigned to whichever capabilities support
// them. Components unsupported by any requested capability are ignored.
void spatial_entity_start(spatial_capability_ capabilities, spatial_component_ components, spatial_on_discover_t on_discover, void* context);

// Stop tracking and destroy the spatial context. All entity IDs become
// invalid. Safe to call if not started.
void spatial_entity_stop();

// Is the context created and ready for update queries / anchor creation?
bool spatial_entity_ready();

///////////////////////////////////////////
// Queries (only valid while ready)      //
///////////////////////////////////////////

// Create a synchronous update snapshot for specific entities. The caller
// provides entity IDs it previously learned about from discovery. Returns
// true on success. Caller must call spatial_snapshot_release on the result.
bool spatial_entity_request_update(const spatial_entity_id* entity_ids, int32_t entity_count, spatial_component_ components, spatial_snapshot_t* out_snapshot);

// Create a spatial anchor at the given pose. Returns the entity ID for
// the new anchor, or 0 on failure. Requires spatial_capability_anchor.
spatial_entity_id spatial_entity_create_anchor(pose_t pose);

// Release all memory owned by a snapshot.
void spatial_snapshot_release(spatial_snapshot_t* snapshot);

///////////////////////////////////////////
// Extension registration (internal)     //
///////////////////////////////////////////

void xr_ext_spatial_entity_register();

}
