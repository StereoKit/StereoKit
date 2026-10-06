/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 * Copyright (c) 2026 Qualcomm Technologies, Inc.
 */

// Gives every anchor entity a matching anchor_t, whichever API made or found it

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

// Runtimes only store a uuid, so names live here to survive across sessions
struct anchor_name_t {
	sk_uuid_t persist_id;
	char*     name;
};

struct spatial_anchors_state_t {
	array_t<anchor_t>         anchors;  // Holds one ref each
	array_t<spatial_entity_t> entities;    // Parallel to anchors
	array_t<sk_uuid_t>        persist_ids; // Parallel to anchors, last known, zero when not persisted
	array_t<anchor_name_t>    names;
	bool32_t                  has_names_file; // False when there's no app data folder
	bool32_t                  woken;          // The app has used the Anchor API
	char                      names_path[1024];
};
static spatial_anchors_state_t local = {};

static const char* names_filename = "spatial_anchor_names.txt";

///////////////////////////////////////////

static spatial_entity_t anchor_entity(anchor_t anchor) {
	int32_t idx = local.anchors.index_of(anchor);
	return idx < 0 ? 0 : local.entities[idx];
}

static void unlink(int32_t idx) {
	local.anchors    .remove(idx);
	local.entities   .remove(idx);
	local.persist_ids.remove(idx);
}

static bool uuid_eq(const sk_uuid_t& a, const sk_uuid_t& b) { return memcmp(a.bytes, b.bytes, sizeof(a.bytes)) == 0; }

static void uuid_to_string(sk_uuid_t id, char* out_str, int32_t size) {
	const uint8_t* b = id.bytes;
	snprintf(out_str, size,
		"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		b[0], b[1], b[ 2], b[ 3], b[ 4], b[ 5], b[ 6], b[ 7],
		b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15]);
}

static int32_t hex_val(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

static int32_t names_find(sk_uuid_t id) {
	for (int32_t i = 0; i < local.names.count; i++)
		if (uuid_eq(local.names[i].persist_id, id)) return i;
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
			entry.persist_id.bytes[b] = (uint8_t)(hi << 4 | lo);
		}
		if (!valid || names_find(entry.persist_id) >= 0) continue;
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
			snprintf(&hex[b*2], 3, "%02x", local.names[i].persist_id.bytes[b]);
		file = string_append(file, 4, hex, " ", local.names[i].name, "\n");
	}
	ska_file_write_text(local.names_path, file);
	sk_free(file);
}

static void names_set(sk_uuid_t id, const char* name) {
	// The file is one entry per line, so a line break would corrupt it
	char* line_safe = string_copy(name);
	for (char* c = line_safe; *c; c++)
		if (*c == '\n' || *c == '\r') *c = ' ';

	int32_t idx = names_find(id);
	if (idx >= 0) {
		if (string_eq(local.names[idx].name, line_safe)) { sk_free(line_safe); return; }
		sk_free(local.names[idx].name);
		local.names[idx].name = line_safe;
	} else {
		anchor_name_t entry = {};
		entry.persist_id = id;
		entry.name       = line_safe;
		local.names.add(entry);
	}
	names_save();
}

static void names_remove(sk_uuid_t id) {
	int32_t idx = names_find(id);
	if (idx < 0) return;
	sk_free(local.names[idx].name);
	local.names.remove(idx);
	names_save();
}

// The stored name if there is one, otherwise the uuid as a string.
static void persisted_name(sk_uuid_t id, char* out_name, int32_t size) {
	int32_t idx = names_find(id);
	if (idx >= 0) snprintf(out_name, size, "%s", local.names[idx].name);
	else          uuid_to_string(id, out_name, size);
}

// The resulting anchor_t reference belongs to local.anchors.
static anchor_t wrap_entity(spatial_entity_t entity, const char* name) {
	pose_t pose = pose_identity;
	spatial_entity_get_anchor(entity, &pose);

	anchor_t  anchor = anchor_create_manual(0, pose, name, nullptr);
	sk_uuid_t id;
	anchor->tracked   = spatial_entity_get_tracked(entity);
	anchor->persisted = spatial_entity_get_uuid(entity, &id);
	local.anchors    .add(anchor);
	local.entities   .add(entity);
	local.persist_ids.add(id);
	return anchor;
}

///////////////////////////////////////////

bool32_t spatial_anchors_available() {
	return (spatial_capabilities() & spatial_capability_anchor) != 0
		&& !spatial_is_user_disabled(spatial_capability_anchor);
}

///////////////////////////////////////////

bool32_t spatial_anchors_init() {
	names_load();
	return true;
}

///////////////////////////////////////////

// Starting anchors can prompt for a permission, so this waits for real use
void spatial_anchors_wake() {
	if (local.woken) return;
	local.woken = true;

	// A system-level request still lets an explicit spatial_disable win
	spatial_enable_system(spatial_capability_anchor);

	// Named anchors are listed right away, and fill in as storage loads them
	for (int32_t i = 0; i < local.names.count; i++) {
		spatial_entity_t entity = spatial_entity_find_uuid(local.names[i].persist_id);
		if (entity != 0) wrap_entity(entity, local.names[i].name);
	}
}

///////////////////////////////////////////

void spatial_anchors_shutdown() {
	spatial_disable_system(spatial_capability_anchor);
	for (int32_t i = local.anchors.count - 1; i >= 0; i--)
		anchor_release(local.anchors[i]);
	local.anchors    .free();
	local.entities   .free();
	local.persist_ids.free();
	for (int32_t i = 0; i < local.names.count; i++)
		sk_free(local.names[i].name);
	local.names.free();
	local = {};
}

///////////////////////////////////////////

void spatial_anchors_step() {
	// Like unnamed persisted anchors, or ones made through SpatialEntity
	int32_t count = spatial_entity_get_count(spatial_component_anchor);
	for (int32_t i = 0; i < count; i++) {
		spatial_entity_t entity = spatial_entity_get_index(spatial_component_anchor, i);
		if (local.entities.index_of(entity) >= 0) continue;

		char      name[128];
		sk_uuid_t id;
		if (spatial_entity_get_uuid(entity, &id)) persisted_name(id, name, sizeof(name));
		else snprintf(name, sizeof(name), "anchor/%llx", (unsigned long long)entity);
		wrap_entity(entity, name);
	}

	for (int32_t i = 0; i < local.anchors.count; i++) {
		anchor_t         anchor = local.anchors[i];
		spatial_entity_t entity = local.entities[i];

		pose_t pose;
		if (spatial_entity_get_anchor(entity, &pose))
			anchor_update_manual(anchor, pose);

		// The id is gone by the time unpersisting lands, so names go by the last known one
		sk_uuid_t  id;
		sk_uuid_t* known     = &local.persist_ids[i];
		bool32_t   persisted = spatial_entity_get_uuid(entity, &id);
		bool       moved     = !uuid_eq(id, *known);
		if (anchor->persisted && (!persisted || moved)) names_remove(*known);
		if (persisted && (!anchor->persisted || moved)) names_set   (id, anchor->name);
		*known            = id;
		anchor->tracked   = spatial_entity_get_tracked(entity);
		anchor->persisted = persisted;
	}
}

///////////////////////////////////////////

// Lost persisted entities wait on storage instead, so these are gone for good
void spatial_anchors_on_removed() {
	bool    persistable = (spatial_capability_components(spatial_capability_anchor) & spatial_component_persistence) != 0;
	int32_t count       = spatial_entity_get_removed_count(spatial_component_none);
	for (int32_t i = 0; i < count; i++) {
		spatial_entity_t entity = spatial_entity_get_removed_index(spatial_component_none, i);
		int32_t          idx    = local.entities.index_of(entity);
		if (idx < 0) continue;

		// A failed lookup only means storage lacks the id while persistence works
		sk_uuid_t id;
		if (persistable && spatial_entity_get_status(entity) == spatial_status_failed && spatial_entity_get_uuid(entity, &id))
			names_remove(id);
		anchor_delete(local.anchors[idx]);
	}
}

///////////////////////////////////////////

anchor_t spatial_anchors_create(pose_t pose, const char* name_utf8) {
	spatial_entity_t entity = spatial_entity_create_anchor(pose, false, 0);
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
	// Destroying also unpersists, but an entity that already left keeps its
	// name, since a lookup can fail just because persistence is unavailable.
	spatial_entity_t entity = anchor_entity(anchor);
	sk_uuid_t        id;
	bool32_t         has_id = spatial_entity_get_uuid(entity, &id);
	if (spatial_entity_destroy(entity) && has_id)
		names_remove(id);

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
