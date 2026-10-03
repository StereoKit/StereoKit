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
#include "../platforms/platform.h"
#include "../sk_memory.h"
#include "../libraries/array.h"
#include "../libraries/stref.h"

#include <sk_app.h>
#include <stdio.h>
#include <string.h>

namespace sk {

///////////////////////////////////////////

// Runtimes only store a uuid, so the anchor's name is kept here to give
// it back the same Name when it's rediscovered in a later session.
struct anchor_name_t {
	uint8_t uuid[16];
	char*   name;
};

struct spatial_anchors_state_t {
	array_t<anchor_t>         anchors;  // Holds one ref each
	array_t<spatial_entity_t> entities; // Parallel to anchors, 0 while a persisted anchor awaits rediscovery
	array_t<anchor_name_t>    names;
	bool32_t                  has_names_file; // False when there's no app data folder
	char                      names_path[1024];
};
static spatial_anchors_state_t local = {};

static const char* names_filename = "spatial_anchor_names.txt";

///////////////////////////////////////////

static spatial_entity_t anchor_entity(anchor_t anchor) {
	int32_t idx = local.anchors.index_of(anchor);
	return idx < 0 ? 0 : local.entities[idx];
}

static int32_t find_by_entity(spatial_entity_t entity) {
	for (int32_t i = 0; i < local.entities.count; i++) {
		if (local.entities[i] == entity) return i;
	}
	return -1;
}

static void unlink(int32_t idx) {
	local.anchors .remove(idx);
	local.entities.remove(idx);
}

static void uuid_to_string(const uint8_t* uuid, char* out_str, int32_t size) {
	snprintf(out_str, size,
		"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		uuid[0], uuid[1], uuid[ 2], uuid[ 3], uuid[ 4], uuid[ 5], uuid[ 6], uuid[ 7],
		uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
}

static int32_t hex_val(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

static int32_t names_find(const uint8_t* uuid) {
	for (int32_t i = 0; i < local.names.count; i++)
		if (memcmp(local.names[i].uuid, uuid, 16) == 0) return i;
	return -1;
}

// One "<32 hex uuid> <name>" pair per line.
static void names_load() {
	char folder[1024];
	local.has_names_file = ska_path_get(ska_path_data, folder, sizeof(folder));
	if (!local.has_names_file) return;
	snprintf(local.names_path, sizeof(local.names_path), "%s" platform_path_separator "%s", folder, names_filename);
	if (!ska_file_exists(local.names_path)) return;

	char*  file = nullptr;
	size_t size = 0;
	if (!ska_file_read(local.names_path, (void**)&file, &size)) return;

	stref_t data = stref_make(file);
	stref_t line = {};
	while (stref_nextline(data, line)) {
		if (line.length < 34 || line.start[32] != ' ') continue;

		anchor_name_t entry = {};
		bool          valid = true;
		for (int32_t b = 0; b < 16 && valid; b++) {
			int32_t hi = hex_val(line.start[b*2]);
			int32_t lo = hex_val(line.start[b*2+1]);
			valid = hi >= 0 && lo >= 0;
			entry.uuid[b] = (uint8_t)(hi << 4 | lo);
		}
		if (!valid || names_find(entry.uuid) >= 0) continue;
		entry.name = stref_copy(stref_substr(line.start + 33, line.length - 33));
		local.names.add(entry);
	}
	ska_file_free_data(file);
}

static void names_save() {
	if (!local.has_names_file) return;
	if (local.names.count == 0) {
		if (ska_file_exists(local.names_path)) platform_file_delete(local.names_path);
		return;
	}

	char* file = string_copy("");
	char  hex[33];
	for (int32_t i = 0; i < local.names.count; i++) {
		for (int32_t b = 0; b < 16; b++)
			snprintf(&hex[b*2], 3, "%02x", local.names[i].uuid[b]);
		file = string_append(file, 4, hex, " ", local.names[i].name, "\n");
	}
	ska_file_write_text(local.names_path, file);
	sk_free(file);
}

static void names_set(const uint8_t* uuid, const char* name) {
	int32_t idx = names_find(uuid);
	if (idx >= 0) {
		if (string_eq(local.names[idx].name, name)) return;
		sk_free(local.names[idx].name);
		local.names[idx].name = string_copy(name);
	} else {
		anchor_name_t entry = {};
		memcpy(entry.uuid, uuid, 16);
		entry.name = string_copy(name);
		local.names.add(entry);
	}
	names_save();
}

static void names_remove_at(int32_t idx) {
	sk_free(local.names[idx].name);
	local.names.remove(idx);
	names_save();
}

// The stored name if there is one, otherwise the uuid as a string.
static void persisted_name(const uint8_t* uuid, char* out_name, int32_t size) {
	int32_t idx = names_find(uuid);
	if (idx >= 0) snprintf(out_name, size, "%s", local.names[idx].name);
	else          uuid_to_string(uuid, out_name, size);
}

// The resulting anchor_t reference belongs to local.anchors.
static anchor_t wrap_entity(spatial_entity_t entity, const char* name) {
	pose_t pose = pose_identity;
	spatial_entity_get_anchor(entity, &pose);

	anchor_t anchor = anchor_create_manual(0, pose, name, nullptr);
	uint8_t  uuid[16];
	anchor->tracked   = spatial_entity_get_tracked(entity);
	anchor->persisted = spatial_entity_get_persist_id(entity, uuid);
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
	// A system-level request keeps anchors running without the app asking,
	// while still letting an explicit spatial_disable turn them off.
	spatial_enable_system(spatial_capability_anchor);
	names_load();
	return true;
}

///////////////////////////////////////////

void spatial_anchors_shutdown() {
	spatial_disable_system(spatial_capability_anchor);
	for (int32_t i = local.anchors.count - 1; i >= 0; i--)
		anchor_release(local.anchors[i]);
	local.anchors .free();
	local.entities.free();
	for (int32_t i = 0; i < local.names.count; i++)
		sk_free(local.names[i].name);
	local.names.free();
	local = {};
}

///////////////////////////////////////////

void spatial_anchors_step() {
	// Wrap anchor entities we haven't seen before: persisted anchors the
	// system loaded from storage, or ones made via the spatial entity API.
	int32_t count = spatial_entity_get_count(spatial_component_anchor);
	for (int32_t i = 0; i < count; i++) {
		spatial_entity_t entity = spatial_entity_get_index(spatial_component_anchor, i);
		if (find_by_entity(entity) >= 0)
			continue;

		char    name[128];
		uint8_t uuid[16];
		if (spatial_entity_get_persist_id(entity, uuid)) {
			persisted_name(uuid, name, sizeof(name));

			// A capability cycle re-discovers persisted anchors as fresh
			// entities, re-bind those only to an anchor_t that's waiting.
			int32_t existing = -1;
			for (int32_t a = 0; a < local.anchors.count; a++)
				if (string_eq(local.anchors[a]->name, name)) { existing = a; break; }
			if (existing >= 0) {
				if (local.entities[existing] == 0) local.entities[existing] = entity;
				continue;
			}
		} else {
			snprintf(name, sizeof(name), "anchor/%llx", (unsigned long long)entity);
		}
		wrap_entity(entity, name);
	}

	// Sync entity state into the anchor assets
	for (int32_t i = 0; i < local.anchors.count; i++) {
		anchor_t         anchor = local.anchors[i];
		spatial_entity_t entity = local.entities[i];
		if (entity == 0) continue;

		pose_t pose;
		if (spatial_entity_get_anchor(entity, &pose))
			anchor_update_manual(anchor, pose);

		uint8_t  uuid[16];
		bool32_t persisted = spatial_entity_get_persist_id(entity, uuid);
		if (persisted && !anchor->persisted) {
			names_set(uuid, anchor->name);
		} else if (!persisted && anchor->persisted) {
			for (int32_t n = 0; n < local.names.count; n++)
				if (string_eq(local.names[n].name, anchor->name)) { names_remove_at(n); break; }
		}
		anchor->tracked   = spatial_entity_get_tracked(entity);
		anchor->persisted = persisted;
	}
}

///////////////////////////////////////////

void spatial_anchors_on_removed() {
	int32_t count = spatial_entity_get_removed_count(spatial_component_anchor);
	for (int32_t i = 0; i < count; i++) {
		spatial_entity_t entity = spatial_entity_get_removed_index(spatial_component_anchor, i);
		int32_t          idx    = find_by_entity(entity);
		if (idx < 0) continue;

		// Persisted anchors that were lost, rather than destroyed, can come
		// back through discovery, so they wait unbound for a re-bind.
		anchor_t anchor = local.anchors[idx];
		uint8_t  uuid[16];
		if (spatial_entity_get_persist_id(entity, uuid) && !spatial_entity_was_destroyed(entity)) {
			local.entities[idx] = 0;
			anchor->tracked     = button_state_inactive;
			continue;
		}
		anchor_delete(anchor);
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
	if (idx >= 0) unlink(idx);
}

///////////////////////////////////////////

void spatial_anchors_delete(anchor_t anchor) {
	// Destroys the underlying entity (which also unpersists it), and
	// releases this system's reference so the anchor won't come back.
	spatial_entity_t entity = anchor_entity(anchor);
	uint8_t          uuid[16];
	if (spatial_entity_get_persist_id(entity, uuid)) {
		int32_t name_idx = names_find(uuid);
		if (name_idx >= 0) names_remove_at(name_idx);
	}
	spatial_entity_destroy(entity);

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
