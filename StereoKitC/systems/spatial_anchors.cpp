/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 * Copyright (c) 2026 Qualcomm Technologies, Inc.
 */

// Implements the anchor_t asset API on top of the spatial entity
// registry. Every entity with an anchor component gets a matching
// anchor_t, regardless of which API created or discovered it.

#include "spatial_entity.h"
#include "../asset_types/anchor.h"
#include "../sk_memory.h"
#include "../libraries/array.h"
#include "../libraries/stref.h"

#include <stdio.h>

namespace sk {

///////////////////////////////////////////

struct spatial_anchors_state_t {
	array_t<anchor_t> anchors; // Holds one ref each; anchor->data is the spatial_entity_t
};
static spatial_anchors_state_t local = {};

///////////////////////////////////////////

static spatial_entity_t anchor_entity(anchor_t anchor) {
	return (spatial_entity_t)(uintptr_t)anchor->data;
}

static anchor_t find_by_entity(spatial_entity_t entity) {
	for (int32_t i = 0; i < local.anchors.count; i++) {
		if (anchor_entity(local.anchors[i]) == entity) return local.anchors[i];
	}
	return nullptr;
}

static void uuid_to_string(const uint8_t* uuid, char* out_str, int32_t size) {
	snprintf(out_str, size,
		"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		uuid[0], uuid[1], uuid[ 2], uuid[ 3], uuid[ 4], uuid[ 5], uuid[ 6], uuid[ 7],
		uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
}

// The resulting anchor_t reference belongs to local.anchors.
static anchor_t wrap_entity(spatial_entity_t entity, const char* name) {
	pose_t pose = pose_identity;
	spatial_entity_get_anchor(entity, &pose);

	anchor_t anchor = anchor_create_manual(0, pose, name, (void*)(uintptr_t)entity);
	uint8_t  uuid[16];
	anchor->tracked   = spatial_entity_get_tracked(entity);
	anchor->persisted = spatial_entity_get_persist_id(entity, uuid);
	local.anchors.add(anchor);
	return anchor;
}

///////////////////////////////////////////

bool32_t spatial_anchors_available() {
	return (spatial_capabilities() & spatial_capability_anchor) != 0;
}

///////////////////////////////////////////

bool32_t spatial_anchors_init() {
	// A system-level request, so app calls to spatial_disable can't turn
	// off the capability this system depends on.
	spatial_enable_system(spatial_capability_anchor);
	return true;
}

///////////////////////////////////////////

void spatial_anchors_shutdown() {
	spatial_disable_system(spatial_capability_anchor);
	for (int32_t i = local.anchors.count - 1; i >= 0; i--)
		anchor_release(local.anchors[i]);
	local.anchors.free();
	local = {};
}

///////////////////////////////////////////

void spatial_anchors_step() {
	// Wrap anchor entities we haven't seen before: persisted anchors the
	// system loaded from storage, or ones made via the spatial entity API.
	int32_t count = spatial_entity_get_count(spatial_component_anchor);
	for (int32_t i = 0; i < count; i++) {
		spatial_entity_t entity = spatial_entity_get_index(spatial_component_anchor, i);
		if (find_by_entity(entity) != nullptr)
			continue;

		// Persisted anchors get their uuid as a stable name
		char    name[64];
		uint8_t uuid[16];
		if (spatial_entity_get_persist_id(entity, uuid)) {
			uuid_to_string(uuid, name, sizeof(name));

			// A capability cycle re-discovers persisted anchors as fresh
			// entities, re-bind those to their existing anchor_t.
			anchor_t existing = nullptr;
			for (int32_t a = 0; a < local.anchors.count; a++)
				if (string_eq(local.anchors[a]->name, name)) { existing = local.anchors[a]; break; }
			if (existing != nullptr) {
				existing->data = (void*)(uintptr_t)entity;
				continue;
			}
		} else {
			snprintf(name, sizeof(name), "anchor/%x", entity);
		}
		wrap_entity(entity, name);
	}

	// Sync entity state into the anchor assets
	for (int32_t i = 0; i < local.anchors.count; i++) {
		anchor_t         anchor = local.anchors[i];
		spatial_entity_t entity = anchor_entity(anchor);

		pose_t pose;
		if (spatial_entity_get_anchor(entity, &pose))
			anchor_update_manual(anchor, pose);

		uint8_t  uuid[16];
		bool32_t persisted = spatial_entity_get_persist_id(entity, uuid);
		if (persisted && !anchor->persisted) {
			// Rename to the uuid, so the name matches what this anchor will
			// be called when it's reloaded in a future session
			char name[64];
			uuid_to_string(uuid, name, sizeof(name));
			sk_free(anchor->name);
			anchor->name = string_copy(name);
		}
		anchor->tracked   = spatial_entity_get_tracked(entity);
		anchor->persisted = persisted;
	}
}

///////////////////////////////////////////

anchor_t spatial_anchors_create(pose_t pose, const char* name_utf8) {
	spatial_entity_t entity = spatial_entity_create_anchor(pose);
	if (entity == 0) return nullptr;

	anchor_t anchor = wrap_entity(entity, name_utf8);
	anchor_addref(anchor);
	return anchor;
}

///////////////////////////////////////////

void spatial_anchors_destroy(anchor_t anchor) {
	int32_t idx = local.anchors.index_of(anchor);
	if (idx >= 0) local.anchors.remove(idx);
	anchor->data = nullptr;
}

///////////////////////////////////////////

void spatial_anchors_delete(anchor_t anchor) {
	// Destroys the underlying entity (which also unpersists it), and
	// releases this system's reference so the anchor won't come back.
	spatial_entity_destroy(anchor_entity(anchor));

	int32_t idx = local.anchors.index_of(anchor);
	if (idx >= 0) {
		local.anchors.remove(idx);
		anchor_release(anchor);
	}
}

///////////////////////////////////////////

bool32_t spatial_anchors_persist(anchor_t anchor, bool32_t persistent) {
	if ((spatial_anchors_capabilities() & anchor_caps_storable) == 0)
		return anchor->persisted == persistent;

	// This is asynchronous: anchor->persisted reflects the change once
	// the operation completes.
	if (persistent) spatial_entity_persist  (anchor_entity(anchor));
	else            spatial_entity_unpersist(anchor_entity(anchor));
	return true;
}

///////////////////////////////////////////

void spatial_anchors_clear_stored() {
	for (int32_t i = 0; i < local.anchors.count; i++) {
		if (local.anchors[i]->persisted)
			spatial_entity_unpersist(anchor_entity(local.anchors[i]));
	}
}

///////////////////////////////////////////

anchor_caps_ spatial_anchors_capabilities() {
	anchor_caps_ result = {};
	if (spatial_anchors_available())         result |= anchor_caps_stability;
	if (spatial_persistence_available() &&
	    (spatial_capability_components(spatial_capability_anchor) & spatial_component_persistence))
		result |= anchor_caps_storable;
	return result;
}

}
