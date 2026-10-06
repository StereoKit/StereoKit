/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 * Copyright (c) 2026 Qualcomm Technologies, Inc.
 */

#pragma once

#include "../stereokit.h"

namespace sk {

// The public spatial entity API lives in stereokit.h. This is the
// internal side: system lifecycle, plus the provider interface spatial
// backends use to push data into the registry.

typedef uint64_t spatial_entity_id_t;

///////////////////////////////////////////
// System lifecycle                      //
///////////////////////////////////////////

bool spatial_init    ();
void spatial_step    (); // Runs after app code: clears new/changed marks, removes stopped entities
void spatial_shutdown();

///////////////////////////////////////////
// Backend provider interface            //
///////////////////////////////////////////
// All calls must come from the main thread.

typedef enum spatial_tracking_ {
	spatial_tracking_stopped  = 1, // Lost permanently, entity will be removed
	spatial_tracking_paused   = 2, // Temporarily not tracked, data is stale
	spatial_tracking_tracking = 3, // Actively tracked, data is live
} spatial_tracking_;

struct spatial_mesh_data_t {
	pose_t    origin; // The vertices are relative to this
	vec3*     verts;
	int32_t   vert_count;
	uint32_t* inds;
	int32_t   ind_count;
};

// One entity's worth of data for ingestion. Fields are read when their
// bit is in `present`, buffer-backed fields also need their bit in
// `buffers_changed`, and ownership of their allocations transfers here.
struct spatial_ingest_t {
	spatial_entity_t    entity; // 0 for an entity the registry hasn't seen, ingest fills it in
	spatial_entity_id_t id;
	spatial_tracking_   tracking;
	spatial_component_  present;
	spatial_component_  buffers_changed;

	pose_t              bounds2d_center;
	vec2                bounds2d_size;
	pose_t              bounds3d_center;
	vec3                bounds3d_size;
	spatial_entity_id_t parent;
	pose_t              anchor_pose;
	plane_align_        plane_alignment;
	spatial_label_      label;
	marker_type_        marker_type;
	uint32_t            marker_id;

	spatial_mesh_data_t mesh;
	spatial_mesh_data_t mesh2d;
	pose_t              polygon_origin;
	vec2*               polygon_verts;
	int32_t             polygon_count;
	char*               marker_text;
	uint8_t*            marker_data;
	int32_t             marker_data_size;
	// Unlike other components, persistence data is valid regardless of
	// tracking state.
	sk_uuid_t           persist_id;
};

// Capability requests from internal systems. These stay on when the app
// never mentions a capability, but an explicit spatial_disable wins.
void     spatial_enable_system   (spatial_capability_ capabilities);
void     spatial_disable_system  (spatial_capability_ capabilities);
bool32_t spatial_is_user_disabled(spatial_capability_ capability);

// Report what the backend can do. Called once at backend init.
void spatial_backend_set_support  (spatial_capability_ caps);
void spatial_backend_set_cap_comps(spatial_capability_ cap, spatial_component_ comps);
// Anchor creation hook: returns the new entity's id, or 0 on failure.
// The registry fills in the new entity's initial data itself.
void spatial_backend_set_create_anchor(spatial_entity_id_t (*create)(pose_t pose, spatial_entity_id_t parent, spatial_entity_t entity));
// Entity destruction hook: releases the backend's tracking of an
// app-created entity.
void spatial_backend_set_destroy      (void (*destroy)(spatial_entity_id_t id));

// Mark a capability's context as warmed up / torn down.
void spatial_backend_set_active   (spatial_capability_ cap, bool32_t active);
// Push entity data into the registry. Fills in `entity` for new ones,
// which the backend should send back on later ingests.
void spatial_backend_ingest       (spatial_capability_ source, spatial_ingest_t* entities, int32_t count);
// Removes all of a capability's entities, as if tracking stopped
void spatial_backend_drop_source  (spatial_capability_ source);

// Increments when a capability's settings change, so the backend can
// tell when an active context needs a rebuild.
uint32_t            spatial_backend_get_config_serial(spatial_capability_ cap);
spatial_capability_ spatial_marker_capability(marker_type_ type);

// Async persistence hooks, null when there's no persistence. Results
// come back with the entity they were given, which is 0 for an unpersist
// when only the uuid is known.
void spatial_backend_set_persist_ops(void (*persist)(spatial_entity_id_t id, spatial_entity_t entity), void (*unpersist)(spatial_entity_t entity, sk_uuid_t persist_id));
void spatial_backend_set_persist      (spatial_entity_t entity, sk_uuid_t persist_id);
void spatial_backend_persist_failed   (spatial_entity_t entity);
void spatial_backend_clear_persist    (spatial_entity_t entity);
void spatial_backend_unpersist_failed (spatial_entity_t entity);
// For persist id lookups, the backend fetches the ids apps are waiting
// on, and reports the ones storage definitively doesn't have.
int32_t spatial_backend_get_find_ids     (sk_uuid_t* out_ids, int32_t capacity);
void    spatial_backend_persist_not_found(sk_uuid_t persist_id);

///////////////////////////////////////////
// Anchor asset backend                  //
///////////////////////////////////////////
// Implements the anchor_t asset API on top of the spatial entity
// registry, see anchor_system_spatial in asset_types/anchor.

bool32_t     spatial_anchors_available   ();
bool32_t     spatial_anchors_init        ();
void         spatial_anchors_shutdown    ();
void         spatial_anchors_step        ();
void         spatial_anchors_on_removed  (); // Called by spatial_step, before removed entities are freed
anchor_t     spatial_anchors_create      (pose_t pose, const char* name_utf8);
void         spatial_anchors_destroy     (anchor_t anchor);
void         spatial_anchors_delete      (anchor_t anchor);
bool32_t     spatial_anchors_persist     (anchor_t anchor, bool32_t persistent);
void         spatial_anchors_clear_stored();
anchor_caps_ spatial_anchors_capabilities();

}
