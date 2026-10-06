/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 * Copyright (c) 2026 Qualcomm Technologies, Inc.
 */

// Gives every anchor entity a matching anchor_t, whichever API made or found it

#include "spatial_entity.h"
#include "spatial_names.h"
#include "../asset_types/anchor.h"
#include "../libraries/array.h"

#include <stdio.h>

namespace sk {

///////////////////////////////////////////

struct spatial_anchors_state_t {
	array_t<anchor_t>         anchors;  // Holds one ref each
	array_t<spatial_entity_t> entities; // Parallel to anchors
	bool32_t                  woken;    // The app has used the Anchor API
};
static spatial_anchors_state_t local = {};

///////////////////////////////////////////

static spatial_entity_t anchor_entity(anchor_t anchor) {
	int32_t idx = local.anchors.index_of(anchor);
	return idx < 0 ? 0 : local.entities[idx];
}

static void unlink(int32_t idx) {
	local.anchors .remove(idx);
	local.entities.remove(idx);
}

// The resulting anchor_t reference belongs to local.anchors.
static anchor_t wrap_entity(spatial_entity_t entity, const char* name) {
	anchor_t  anchor = anchor_create_manual(0, spatial_entity_get_pose(entity), name, nullptr);
	sk_uuid_t id;
	anchor->tracked   = spatial_entity_get_tracked(entity);
	anchor->persisted = spatial_entity_get_uuid(entity, &id);
	local.anchors .add(anchor);
	local.entities.add(entity);
	return anchor;
}

///////////////////////////////////////////

bool32_t spatial_anchors_available() {
	return (spatial_capabilities() & spatial_capability_anchor) != 0
		&& !spatial_is_user_disabled(spatial_capability_anchor);
}

///////////////////////////////////////////

bool32_t spatial_anchors_init() {
	return true;
}

///////////////////////////////////////////

// Starting anchors can prompt for a permission, so this waits for real use
void spatial_anchors_wake() {
	if (local.woken) return;
	local.woken = true;

	// A system-level request still lets an explicit spatial_disable win
	spatial_request_system(spatial_capability_anchor);

	// Named anchors are listed right away, and fill in as storage loads them
	for (int32_t i = 0; i < spatial_names_count(); i++) {
		spatial_entity_t entity = spatial_entity_find_anchor_uuid(spatial_names_get_index(i));
		if (entity != 0) wrap_entity(entity, spatial_entity_get_name(entity));
	}
}

///////////////////////////////////////////

void spatial_anchors_shutdown() {
	spatial_release_system(spatial_capability_anchor);
	for (int32_t i = local.anchors.count - 1; i >= 0; i--)
		anchor_release(local.anchors[i]);
	local.anchors .free();
	local.entities.free();
	local = {};
}

///////////////////////////////////////////

void spatial_anchors_step() {
	// Like unnamed persisted anchors, or ones made through SpatialEntity
	int32_t count = spatial_entity_get_count(spatial_component_anchor);
	for (int32_t i = 0; i < count; i++) {
		spatial_entity_t entity = spatial_entity_get_index(spatial_component_anchor, i);
		if (local.entities.index_of(entity) >= 0) continue;

		char        fallback[32];
		const char* name = spatial_entity_get_name(entity);
		if (name == nullptr) {
			snprintf(fallback, sizeof(fallback), "anchor/%llx", (unsigned long long)entity);
			name = fallback;
		}
		wrap_entity(entity, name);
	}

	for (int32_t i = 0; i < local.anchors.count; i++) {
		anchor_t         anchor = local.anchors[i];
		spatial_entity_t entity = local.entities[i];

		if (spatial_entity_get_components(entity) & spatial_component_anchor)
			anchor_update_manual(anchor, spatial_entity_get_pose(entity));

		sk_uuid_t id;
		anchor->tracked   = spatial_entity_get_tracked(entity);
		anchor->persisted = spatial_entity_get_uuid(entity, &id);
	}
}

///////////////////////////////////////////

// Lost persisted entities wait on storage instead, so these are gone for good
void spatial_anchors_on_removed() {
	int32_t count = spatial_entity_get_removed_count(spatial_component_none);
	for (int32_t i = 0; i < count; i++) {
		int32_t idx = local.entities.index_of(spatial_entity_get_removed_index(spatial_component_none, i));
		if (idx >= 0) anchor_delete(local.anchors[idx]);
	}
}

///////////////////////////////////////////

anchor_t spatial_anchors_create(pose_t pose, const char* name_utf8) {
	spatial_entity_t entity = spatial_entity_create_anchor(pose, nullptr, 0);
	if (entity == 0) return nullptr;

	// Saved with the uuid if the anchor is persisted later
	spatial_entity_set_name(entity, name_utf8);
	anchor_t anchor = wrap_entity(entity, name_utf8);
	anchor_addref(anchor);
	return anchor;
}

///////////////////////////////////////////

void spatial_anchors_destroy(anchor_t anchor) {
	int32_t idx = local.anchors.index_of(anchor);
	if (idx >= 0) unlink(idx);
}

///////////////////////////////////////////

void spatial_anchors_delete(anchor_t anchor) {
	spatial_entity_destroy(anchor_entity(anchor));

	int32_t idx = local.anchors.index_of(anchor);
	if (idx >= 0) {
		unlink(idx);
		anchor_release(anchor);
	}
}

///////////////////////////////////////////

bool32_t spatial_anchors_persist(anchor_t anchor, bool32_t persistent) {
	if ((spatial_anchors_capabilities() & anchor_caps_storable) == 0)
		return anchor->persisted == persistent;

	// Asynchronous, anchor->persisted catches up once the operation lands
	spatial_entity_t entity = anchor_entity(anchor);
	return persistent
		? spatial_entity_persist  (entity)
		: spatial_entity_unpersist(entity);
}

///////////////////////////////////////////

void spatial_anchors_clear_stored() {
	for (int32_t i = 0; i < local.anchors.count; i++) {
		if (local.anchors[i]->persisted)
			spatial_entity_unpersist(local.entities[i]);
	}
}

///////////////////////////////////////////

anchor_caps_ spatial_anchors_capabilities() {
	anchor_caps_ result = {};
	if (spatial_anchors_available()) result |= anchor_caps_stability;
	if (spatial_capability_components(spatial_capability_anchor) & spatial_component_persistence)
		result |= anchor_caps_storable;
	return result;
}

}
