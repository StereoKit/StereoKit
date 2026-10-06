/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 * Copyright (c) 2026 Qualcomm Technologies, Inc.
 */

#include "spatial_entity.h"
#include "spatial_names.h"
#include "../sk_memory.h"
#include "../libraries/array.h"
#include "../libraries/stref.h"
#include "../asset_types/mesh_.h"
#include "vert_format.h"

#include <string.h>

namespace sk {

///////////////////////////////////////////
// Types                                 //
///////////////////////////////////////////

typedef enum record_life_ {
	record_life_live,           // In the list, backed by the backend
	record_life_pending_create, // In the list, waiting on the backend to create it
	record_life_pending_find,   // In the list, waiting on storage to load it
	record_life_lost,           // Left the list, the system stopped tracking it
	record_life_destroyed,      // Left the list at the app's request
	record_life_failed,         // Left the list, couldn't be created or found
	record_life_detached,       // Freed for the app, kept until its persist op lands
} record_life_;

// The app's last persistence request, cleared once it's met.
typedef enum persist_want_ {
	persist_want_none,
	persist_want_persisted,
	persist_want_unpersisted,
} persist_want_;

typedef enum persist_flight_ {
	persist_flight_none,
	persist_flight_persist,
	persist_flight_unpersist,
} persist_flight_;

// Handles are a 32 bit slot index plus a 32 bit generation that bumps on
// free, so stale handles fail instead of aliasing. 0 is never valid.
struct spatial_record_t {
	bool32_t            alive;
	uint32_t            generation;
	spatial_entity_t    handle;
	spatial_entity_id_t backend_id; // The backend's own id for this entity, 0 while pending
	spatial_capability_ source;
	record_life_        life;
	persist_want_       persist_want;
	persist_flight_     persist_flight;
	bool32_t            request_failed; // An app request failed, cleared by the next request
	button_state_       tracked;
	spatial_component_  components; // Components with valid last-known data
	spatial_component_  changed;    // Cleared each frame in spatial_step
	spatial_entity_t    create_parent;

	pose_t              bounds2d_center;
	vec2                bounds2d_size;
	pose_t              bounds3d_center;
	vec3                bounds3d_size;
	spatial_entity_id_t parent;
	spatial_entity_t    parent_cache; // Last resolved parent, checked against `parent` on read
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
	sk_uuid_t           persist_id;
	char*               name; // App chosen anchor name, saved to spatial_names once persisted
};

struct unpersist_req_t {
	spatial_entity_t entity; // 0 when only the id is known
	sk_uuid_t        persist_id;
};

// Lets sequential index loops resume from the last match instead of rescanning
struct list_cursor_t {
	spatial_component_ filter;
	int32_t            index;
	int32_t            pos;
	uint32_t           version;
};

// App choices, which can be made before SK init and survive it.
struct spatial_settings_t {
	spatial_capability_ requested;
	spatial_capability_ user_disabled; // Explicit spatial_disable calls, these override system requests
	float               marker_size      [32]; // Indexed by capability bit position
	bool32_t            marker_stationary[32];
	aruco_dict_         aruco_dict;
	april_tag_dict_     april_tag_dict;
	uint32_t            config_serials[32]; // Indexed by capability bit position
};

struct spatial_state_t {
	spatial_settings_t  settings;
	spatial_capability_ supported;
	spatial_component_  cap_comps[32]; // Indexed by capability bit position
	spatial_capability_ requested_system; // Internal systems' requests
	spatial_capability_ running;
	spatial_entity_id_t (*create_anchor)(pose_t pose, spatial_entity_id_t parent, spatial_entity_t entity);
	void                (*destroy)      (spatial_entity_id_t id);
	void                (*persist)      (spatial_entity_id_t id, spatial_entity_t entity);
	void                (*unpersist)    (spatial_entity_t entity, sk_uuid_t persist_id);

	array_t<spatial_record_t> slots;
	array_t<int32_t>          slot_free;
	array_t<spatial_entity_t> live;    // Handles in enumeration order
	array_t<spatial_entity_t> arrived;      // Appeared this frame
	array_t<spatial_entity_t> arrived_next; // App requests, which appear next frame
	array_t<spatial_entity_t> removed;      // Left this frame, freed at spatial_step
	array_t<spatial_entity_t> detached;
	array_t<unpersist_req_t>  unpersist_reqs; // Waiting on persistence to start up

	uint32_t      list_version; // Bumps whenever list contents or components change
	list_cursor_t cursor_live;
	list_cursor_t cursor_arrived;
	list_cursor_t cursor_removed;
};
static spatial_state_t local = {};

///////////////////////////////////////////
// Helpers                               //
///////////////////////////////////////////

static int32_t cap_bit_index(spatial_capability_ cap) {
	for (int32_t i = 0; i < 32; i++)
		if ((int)cap & (1 << i)) return i;
	return -1;
}

static bool uuid_eq    (const sk_uuid_t& a, const sk_uuid_t& b) { return memcmp(a.bytes, b.bytes, sizeof(a.bytes)) == 0; }
static bool uuid_is_nil(const sk_uuid_t& uuid) { return uuid_eq(uuid, {}); }

static bool record_listed(const spatial_record_t* rec) {
	return rec->life <= record_life_pending_find;
}

static bool cap_persistable(spatial_capability_ cap) {
	int32_t idx = cap_bit_index(cap);
	return idx >= 0 && (local.cap_comps[idx] & spatial_component_persistence) != 0;
}

// Includes detached records, which only backend completions should see.
static spatial_record_t* record_find_any(spatial_entity_t handle) {
	uint32_t idx = (uint32_t)(handle & 0xFFFFFFFF);
	uint32_t gen = (uint32_t)(handle >> 32);
	if (gen == 0 || idx >= (uint32_t)local.slots.count) return nullptr;

	spatial_record_t* rec = &local.slots[idx];
	return rec->alive && rec->generation == gen ? rec : nullptr;
}

static spatial_record_t* record_find(spatial_entity_t handle) {
	spatial_record_t* rec = record_find_any(handle);
	return rec != nullptr && rec->life != record_life_detached ? rec : nullptr;
}

// Null unless the entity is valid and has every component asked for.
static spatial_record_t* record_with(spatial_entity_t handle, spatial_component_ components) {
	spatial_record_t* rec = record_find(handle);
	return rec != nullptr && (rec->components & components) == components ? rec : nullptr;
}

static spatial_record_t* record_find_persisted(sk_uuid_t persist_id) {
	for (int32_t i = 0; i < local.live.count; i++) {
		spatial_record_t* rec = record_find(local.live[i]);
		if (rec != nullptr && record_listed(rec) && (rec->components & spatial_component_persistence) && uuid_eq(rec->persist_id, persist_id))
			return rec;
	}
	return nullptr;
}

// May grow the slot array, invalidating any held record pointers
static spatial_record_t* record_add(spatial_capability_ source, record_life_ life) {
	int32_t idx;
	if (local.slot_free.count > 0) {
		idx = local.slot_free[local.slot_free.count - 1];
		local.slot_free.remove(local.slot_free.count - 1);
	} else {
		local.slots.add({});
		idx = local.slots.count - 1;
	}

	spatial_record_t* rec = &local.slots[idx];
	uint32_t          gen = rec->generation == 0 ? 1 : rec->generation;
	*rec = {};
	rec->alive      = true;
	rec->generation = gen;
	rec->handle     = (spatial_entity_t)gen << 32 | (uint32_t)idx;
	rec->source     = source;
	rec->life       = life;
	local.live.add(rec->handle);
	// App requests start pending, and wait a frame so New sees them exactly once
	if (life == record_life_live) local.arrived     .add(rec->handle);
	else                          local.arrived_next.add(rec->handle);
	local.list_version++;
	return rec;
}

static void mesh_data_free(spatial_mesh_data_t* mesh) {
	sk_free(mesh->verts);
	sk_free(mesh->inds);
}

static void record_free(spatial_record_t* rec) {
	mesh_data_free(&rec->mesh);
	mesh_data_free(&rec->mesh2d);
	sk_free(rec->polygon_verts);
	sk_free(rec->marker_text);
	sk_free(rec->marker_data);
	sk_free(rec->name);

	int32_t  idx = (int32_t)(rec->handle & 0xFFFFFFFF);
	uint32_t gen = rec->generation + 1;
	if (gen == 0) gen = 1;

	*rec = {};
	rec->generation = gen;
	local.slot_free.add(idx);
}

static void record_untrack(spatial_record_t* rec) {
	rec->tracked = (rec->tracked & button_state_active)
		? (button_state_)(button_state_inactive | button_state_just_inactive)
		: button_state_inactive;
}

// Takes the entity out of the live list at the end of the frame.
static void record_leave(spatial_record_t* rec, record_life_ life) {
	if (!record_listed(rec)) return;
	rec->life = life;
	record_untrack(rec);
	local.removed.add(rec->handle);
	local.list_version++;
}

// A lost persisted entity keeps its handle and waits on a storage lookup
static void record_lose(spatial_record_t* rec) {
	if (rec->life == record_life_pending_create || rec->life == record_life_pending_find) return;
	bool persist_busy = rec->persist_want != persist_want_none || rec->persist_flight != persist_flight_none;
	if ((rec->components & spatial_component_persistence) == 0 || persist_busy) {
		record_leave(rec, record_life_lost);
		return;
	}
	rec->backend_id = 0;
	rec->life       = record_life_pending_find;
	record_untrack(rec);
	local.list_version++;
}

///////////////////////////////////////////
// Capabilities and configuration        //
///////////////////////////////////////////

spatial_capability_ spatial_capabilities() {
	return local.supported;
}

spatial_component_ spatial_capability_components(spatial_capability_ capability) {
	int32_t idx = cap_bit_index(capability);
	return idx < 0 ? spatial_component_none : local.cap_comps[idx];
}

void spatial_request(spatial_capability_ capabilities) {
	local.settings.requested     |=  capabilities;
	local.settings.user_disabled &= ~capabilities;
}

void spatial_disable(spatial_capability_ capabilities) {
	local.settings.requested     &= ~capabilities;
	local.settings.user_disabled |=  capabilities;
}

///////////////////////////////////////////

// Settings are baked into contexts, so a change asks the backend to rebuild
static void config_changed(spatial_capability_ capabilities) {
	for (int32_t i = 0; i < 32; i++)
		if (capabilities & (1 << i)) local.settings.config_serials[i]++;
}

spatial_capability_ spatial_marker_capability(marker_type_ type) {
	switch (type) {
	case marker_type_qr_code:   return spatial_capability_qr_code;
	case marker_type_micro_qr:  return spatial_capability_micro_qr;
	case marker_type_aruco:     return spatial_capability_aruco;
	case marker_type_april_tag: return spatial_capability_april_tag;
	default:                    return spatial_capability_none;
	}
}

void spatial_set_marker_size(marker_type_ type, float size_meters) {
	int32_t idx = cap_bit_index(spatial_marker_capability(type));
	if (idx < 0) { log_warn("spatial_set_marker_size: invalid marker type"); return; }
	if (size_meters < 0) size_meters = 0;
	if (local.settings.marker_size[idx] == size_meters) return;
	local.settings.marker_size[idx] = size_meters;
	config_changed(spatial_marker_capability(type));
}
float spatial_get_marker_size(marker_type_ type) {
	int32_t idx = cap_bit_index(spatial_marker_capability(type));
	return idx < 0 ? 0 : local.settings.marker_size[idx];
}

void spatial_set_marker_stationary(marker_type_ type, bool32_t stationary) {
	int32_t idx = cap_bit_index(spatial_marker_capability(type));
	if (idx < 0) { log_warn("spatial_set_marker_stationary: invalid marker type"); return; }
	stationary = stationary ? 1 : 0;
	if (local.settings.marker_stationary[idx] == stationary) return;
	local.settings.marker_stationary[idx] = stationary;
	config_changed(spatial_marker_capability(type));
}
bool32_t spatial_get_marker_stationary(marker_type_ type) {
	int32_t idx = cap_bit_index(spatial_marker_capability(type));
	return idx < 0 ? false : local.settings.marker_stationary[idx];
}

void spatial_set_aruco_dictionary(aruco_dict_ dictionary) {
	if (local.settings.aruco_dict == dictionary) return;
	local.settings.aruco_dict = dictionary;
	config_changed(spatial_capability_aruco);
}
aruco_dict_ spatial_get_aruco_dictionary() { return local.settings.aruco_dict; }

void spatial_set_april_tag_dictionary(april_tag_dict_ dictionary) {
	if (local.settings.april_tag_dict == dictionary) return;
	local.settings.april_tag_dict = dictionary;
	config_changed(spatial_capability_april_tag);
}
april_tag_dict_ spatial_get_april_tag_dictionary() { return local.settings.april_tag_dict; }

bool32_t spatial_is_user_disabled(spatial_capability_ capability) {
	return (local.settings.user_disabled & capability) != 0;
}

void spatial_request_system(spatial_capability_ capabilities) {
	local.requested_system |= capabilities;
}

void spatial_release_system(spatial_capability_ capabilities) {
	local.requested_system &= ~capabilities;
}

spatial_capability_ spatial_get_requested() {
	return (local.settings.requested | local.requested_system) & ~local.settings.user_disabled;
}

spatial_capability_ spatial_get_running() {
	return local.running;
}

///////////////////////////////////////////
// Entity list                           //
///////////////////////////////////////////

static int32_t list_count(const array_t<spatial_entity_t>* list, spatial_component_ with_components) {
	if (with_components == spatial_component_none)
		return list->count;

	int32_t result = 0;
	for (int32_t i = 0; i < list->count; i++) {
		if (record_with(list->data[i], with_components)) result++;
	}
	return result;
}

static spatial_entity_t list_index(const array_t<spatial_entity_t>* list, list_cursor_t* cursor, spatial_component_ with_components, int32_t index) {
	if (index < 0) return 0;
	if (with_components == spatial_component_none)
		return index < list->count ? list->data[index] : 0;

	int32_t pos  = 0;
	int32_t curr = 0;
	if (cursor->version == local.list_version && cursor->filter == with_components && cursor->index >= 0 && cursor->index <= index) {
		pos  = cursor->pos;
		curr = cursor->index;
	}
	for (; pos < list->count; pos++) {
		if (!record_with(list->data[pos], with_components)) continue;
		if (curr == index) {
			*cursor = { with_components, index, pos, local.list_version };
			return list->data[pos];
		}
		curr++;
	}
	return 0;
}

int32_t spatial_entity_get_count(spatial_component_ with_components) {
	return list_count(&local.live, with_components);
}

spatial_entity_t spatial_entity_get_index(spatial_component_ with_components, int32_t index) {
	return list_index(&local.live, &local.cursor_live, with_components, index);
}

int32_t spatial_entity_get_new_count(spatial_component_ with_components) {
	return list_count(&local.arrived, with_components);
}

spatial_entity_t spatial_entity_get_new_index(spatial_component_ with_components, int32_t index) {
	return list_index(&local.arrived, &local.cursor_arrived, with_components, index);
}

int32_t spatial_entity_get_removed_count(spatial_component_ with_components) {
	return list_count(&local.removed, with_components);
}

spatial_entity_t spatial_entity_get_removed_index(spatial_component_ with_components, int32_t index) {
	return list_index(&local.removed, &local.cursor_removed, with_components, index);
}

// Storage loads async, so this returns a placeholder the backend's lookup binds to
spatial_entity_t spatial_entity_find_anchor_uuid(sk_uuid_t uuid) {
	if (uuid_is_nil(uuid)) return 0;

	spatial_record_t* rec = record_find_persisted(uuid);
	if (rec != nullptr) return rec->handle;

	if (!cap_persistable(spatial_capability_anchor)) {
		log_warn("spatial_entity_find_anchor: persistence isn't supported on this system");
		return 0;
	}
	if ((spatial_get_requested() & spatial_capability_anchor) == 0)
		log_warn("spatial_entity_find_anchor: this won't load until SpatialCapability.Anchor is requested");

	rec = record_add(spatial_capability_anchor, record_life_pending_find);
	rec->components = spatial_component_persistence;
	rec->persist_id = uuid;
	return rec->handle;
}

///////////////////////////////////////////

// Includes anchors whose name hasn't reached storage yet, since their persist is still in flight.
static spatial_record_t* record_find_named(const char* name) {
	for (int32_t i = 0; i < local.live.count; i++) {
		spatial_record_t* rec = record_find(local.live[i]);
		if (rec != nullptr && record_listed(rec) && rec->name != nullptr && string_eq(rec->name, name))
			return rec;
	}
	return nullptr;
}

spatial_entity_t spatial_entity_find_anchor(const char* name_utf8) {
	if (name_utf8 == nullptr || name_utf8[0] == '\0') return 0;

	spatial_record_t* rec = record_find_named(name_utf8);
	if (rec != nullptr) return rec->handle;

	sk_uuid_t uuid;
	if (!spatial_names_find(name_utf8, &uuid)) return 0;

	spatial_entity_t entity = spatial_entity_find_anchor_uuid(uuid);
	rec = record_find(entity);
	if (rec != nullptr && rec->name == nullptr) rec->name = string_copy(name_utf8);
	return entity;
}

///////////////////////////////////////////

const char* spatial_entity_get_name(spatial_entity_t entity) {
	spatial_record_t* rec = record_find(entity);
	if (rec == nullptr) return nullptr;
	if (rec->name != nullptr) return rec->name;
	return (rec->components & spatial_component_persistence) ? spatial_names_get(rec->persist_id) : nullptr;
}

void spatial_entity_set_name(spatial_entity_t entity, const char* name_utf8) {
	spatial_record_t* rec = record_find(entity);
	if (rec == nullptr) return;
	sk_free(rec->name);
	rec->name = name_utf8 == nullptr ? nullptr : string_copy(name_utf8);
	if (rec->name != nullptr && (rec->components & spatial_component_persistence))
		spatial_names_set(rec->persist_id, rec->name);
}

///////////////////////////////////////////
// Entity data                           //
///////////////////////////////////////////

bool32_t spatial_entity_is_valid(spatial_entity_t entity) {
	return record_find(entity) != nullptr;
}

spatial_status_ spatial_entity_get_status(spatial_entity_t entity) {
	spatial_record_t* rec = record_find(entity);
	if (rec == nullptr) return spatial_status_none;
	switch (rec->life) {
	case record_life_failed:         return spatial_status_failed;
	case record_life_pending_create:
	case record_life_pending_find:   return spatial_status_pending;
	default: break;
	}
	if (rec->persist_want != persist_want_none || rec->persist_flight != persist_flight_none) return spatial_status_pending;
	return rec->request_failed ? spatial_status_partial : spatial_status_ready;
}

button_state_ spatial_entity_get_tracked(spatial_entity_t entity) {
	spatial_record_t* rec = record_find(entity);
	return rec == nullptr ? button_state_inactive : rec->tracked;
}

spatial_component_ spatial_entity_get_components(spatial_entity_t entity) {
	spatial_record_t* rec = record_find(entity);
	return rec == nullptr ? spatial_component_none : rec->components;
}

spatial_component_ spatial_entity_get_changed(spatial_entity_t entity) {
	spatial_record_t* rec = record_find(entity);
	return rec == nullptr ? spatial_component_none : rec->changed;
}

// Resolved on read, since a parent can arrive later than its child
spatial_entity_t spatial_entity_get_parent(spatial_entity_t entity) {
	spatial_record_t* rec = record_with(entity, spatial_component_parent);
	if (rec == nullptr || rec->parent == 0) return 0;

	spatial_record_t* cached = record_find(rec->parent_cache);
	if (cached != nullptr && cached->backend_id == rec->parent) return cached->handle;

	rec->parent_cache = 0;
	for (int32_t i = 0; i < local.live.count; i++) {
		spatial_record_t* parent = record_find(local.live[i]);
		if (parent != nullptr && parent->backend_id == rec->parent) {
			rec->parent_cache = parent->handle;
			break;
		}
	}
	return rec->parent_cache;
}

///////////////////////////////////////////

// Anchors and bounds say where the thing is, mesh origins are only vertex frames
pose_t spatial_entity_get_pose(spatial_entity_t entity) {
	spatial_record_t* rec = record_find(entity);
	if (rec == nullptr) return pose_identity;

	spatial_component_ comps = rec->components;
	if (comps & spatial_component_anchor  ) return rec->anchor_pose;
	if (comps & spatial_component_bounds2d) return rec->bounds2d_center;
	if (comps & spatial_component_bounds3d) return rec->bounds3d_center;
	if (comps & spatial_component_mesh2d  ) return rec->mesh2d.origin;
	if (comps & spatial_component_polygon ) return rec->polygon_origin;
	if (comps & spatial_component_mesh    ) return rec->mesh.origin;
	return pose_identity;
}

///////////////////////////////////////////

bool32_t spatial_entity_get_bounds2d(spatial_entity_t entity, pose_t* out_center, vec2* out_size) {
	spatial_record_t* rec = record_with(entity, spatial_component_bounds2d);
	if (rec == nullptr) return false;
	*out_center = rec->bounds2d_center;
	*out_size   = rec->bounds2d_size;
	return true;
}

bool32_t spatial_entity_get_bounds3d(spatial_entity_t entity, pose_t* out_center, vec3* out_size) {
	spatial_record_t* rec = record_with(entity, spatial_component_bounds3d);
	if (rec == nullptr) return false;
	*out_center = rec->bounds3d_center;
	*out_size   = rec->bounds3d_size;
	return true;
}

bool32_t spatial_entity_get_plane_align(spatial_entity_t entity, plane_align_* out_alignment) {
	spatial_record_t* rec = record_with(entity, spatial_component_plane_alignment);
	*out_alignment = rec == nullptr ? plane_align_none : rec->plane_alignment;
	return rec != nullptr;
}

bool32_t spatial_entity_get_label(spatial_entity_t entity, spatial_label_* out_label) {
	spatial_record_t* rec = record_with(entity, spatial_component_label);
	*out_label = rec == nullptr ? spatial_label_none : rec->label;
	return rec != nullptr;
}

// A null ref_mesh only fetches the origin, filling the mesh is the expensive part
static bool32_t fill_mesh(const spatial_mesh_data_t* data, mesh_t ref_mesh, pose_t* out_origin) {
	*out_origin = data->origin;
	if (ref_mesh == nullptr) return true;
	if (data->vert_count <= 0 || data->ind_count <= 0) return false;

	vert_t* verts = sk_malloc_t(vert_t, data->vert_count);
	for (int32_t i = 0; i < data->vert_count; i++)
		verts[i] = { data->verts[i], {0,1,0}, {0,0}, {255,255,255,255} };
	mesh_calculate_normals(VERT_FORMAT_DEFAULT, verts, data->vert_count, (const vind_t*)data->inds, data->ind_count);
	mesh_set_data(ref_mesh, verts, data->vert_count, (const vind_t*)data->inds, data->ind_count);

	sk_free(verts);
	return true;
}

bool32_t spatial_entity_get_mesh(spatial_entity_t entity, mesh_t ref_mesh, pose_t* out_origin) {
	spatial_record_t* rec = record_with(entity, spatial_component_mesh);
	return rec != nullptr && fill_mesh(&rec->mesh, ref_mesh, out_origin);
}

bool32_t spatial_entity_get_mesh2d(spatial_entity_t entity, mesh_t ref_mesh, pose_t* out_origin) {
	spatial_record_t* rec = record_with(entity, spatial_component_mesh2d);
	return rec != nullptr && fill_mesh(&rec->mesh2d, ref_mesh, out_origin);
}

bool32_t spatial_entity_get_polygon(spatial_entity_t entity, pose_t* out_origin, const vec2** out_verts, int32_t* out_count) {
	spatial_record_t* rec = record_with(entity, spatial_component_polygon);
	if (rec == nullptr) return false;
	*out_origin = rec->polygon_origin;
	*out_verts  = rec->polygon_verts;
	*out_count  = rec->polygon_count;
	return true;
}

bool32_t spatial_entity_get_marker(spatial_entity_t entity, marker_type_* out_type, uint32_t* out_marker_id) {
	spatial_record_t* rec = record_with(entity, spatial_component_marker);
	if (rec == nullptr) return false;
	*out_type      = rec->marker_type;
	*out_marker_id = rec->marker_id;
	return true;
}

const char* spatial_entity_get_marker_text(spatial_entity_t entity) {
	spatial_record_t* rec = record_find(entity);
	return rec == nullptr ? nullptr : rec->marker_text;
}

const uint8_t* spatial_entity_get_marker_data(spatial_entity_t entity, int32_t* out_size) {
	spatial_record_t* rec = record_find(entity);
	*out_size = rec == nullptr ? 0 : rec->marker_data_size;
	return rec == nullptr ? nullptr : rec->marker_data;
}

///////////////////////////////////////////
// Requests                              //
///////////////////////////////////////////

// Storage only needs the id, so this can wait for persistence to start.
static void unpersist_send(spatial_entity_t entity, sk_uuid_t persist_id) {
	spatial_names_remove(persist_id);
	if (local.unpersist != nullptr) local.unpersist(entity, persist_id);
	else                            local.unpersist_reqs.add({ entity, persist_id });
}

static void record_detached_free(spatial_record_t* rec) {
	int32_t idx = local.detached.index_of(rec->handle);
	if (idx >= 0) local.detached.remove(idx);
	record_free(rec);
}

// Moves one operation toward the app's last request. The backend may finish
// it before this returns, so don't trust the record's state afterwards.
static void persist_reconcile(spatial_record_t* rec) {
	if (rec->persist_flight != persist_flight_none) return;
	bool persisted = (rec->components & spatial_component_persistence) != 0;

	if (rec->persist_want == persist_want_unpersisted && persisted) {
		rec->persist_flight = persist_flight_unpersist;
		unpersist_send(rec->handle, rec->persist_id);
		return;
	}
	if (rec->persist_want == persist_want_persisted && !persisted && record_listed(rec)) {
		if (!cap_persistable(rec->source)) {
			rec->persist_want   = persist_want_none;
			rec->request_failed = true;
			return;
		}
		// Runtimes can't reliably persist an untracked entity
		if (local.persist == nullptr || rec->life != record_life_live || (rec->tracked & button_state_active) == 0) return;
		rec->persist_flight = persist_flight_persist;
		local.persist(rec->backend_id, rec->handle);
		return;
	}
	rec->persist_want = persist_want_none;

	// A lookup can never load once storage no longer has it
	if      (rec->life == record_life_detached)                   record_detached_free(rec);
	else if (rec->life == record_life_pending_find && !persisted) record_leave(rec, record_life_lost);
}

// Until anchors are running, a pending anchor sits untracked at its requested pose
static void record_try_create(spatial_record_t* rec) {
	if ((local.supported & spatial_capability_anchor) == 0) { record_leave(rec, record_life_failed); return; }
	if (local.create_anchor == nullptr || (local.running & spatial_capability_anchor) == 0) return;

	spatial_entity_id_t parent_id = 0;
	if (rec->create_parent != 0) {
		spatial_record_t* parent = record_find(rec->create_parent);
		if (parent == nullptr || !record_listed(parent)) {
			log_warn("spatial_entity_create_anchor: parent entity went away before the anchor could be created");
			record_leave(rec, record_life_failed);
			return;
		}
		if (parent->life != record_life_live) return;
		parent_id = parent->backend_id;
	}

	spatial_entity_id_t id = local.create_anchor(rec->anchor_pose, parent_id, rec->handle);
	if (id == 0) { record_leave(rec, record_life_failed); return; }

	rec->life       = record_life_live;
	rec->backend_id = id;
	rec->tracked    = (button_state_)(button_state_active | button_state_just_active);
	local.list_version++;
	persist_reconcile(rec);
}

// A new anchor takes over the name, so whatever held it goes away for good.
static void name_take(const char* name) {
	spatial_record_t* held = record_find_named(name);
	if (held == nullptr) {
		sk_uuid_t uuid;
		if (!spatial_names_find(name, &uuid)) return;
		held = record_find_persisted(uuid);
		if (held == nullptr) { spatial_entity_unpersist_uuid(uuid); return; }
	}

	spatial_entity_t handle = held->handle;
	sk_free(held->name);
	held->name = nullptr;
	if (!spatial_entity_destroy(handle)) spatial_entity_unpersist(handle);
}

spatial_entity_t spatial_entity_create_anchor(pose_t pose, const char* opt_name_utf8, spatial_entity_t parent) {
	bool persist = opt_name_utf8 != nullptr;
	if ((local.supported & spatial_capability_anchor) == 0) {
		log_warn("spatial_entity_create_anchor: anchors aren't supported on this system");
		return 0;
	}
	if (persist && opt_name_utf8[0] == '\0') {
		log_warn("spatial_entity_create_anchor: an anchor's name can't be empty");
		return 0;
	}
	if (persist && !cap_persistable(spatial_capability_anchor)) {
		log_warn("spatial_entity_create_anchor: anchors can't be persisted on this system");
		return 0;
	}
	if (parent != 0 && record_find(parent) == nullptr) {
		log_warn("spatial_entity_create_anchor: parent entity is not valid");
		return 0;
	}
	if ((spatial_get_requested() & spatial_capability_anchor) == 0)
		log_warn("spatial_entity_create_anchor: this anchor won't be created until SpatialCapability.Anchor is requested");
	if (persist) name_take(opt_name_utf8);

	spatial_record_t* rec = record_add(spatial_capability_anchor, record_life_pending_create);
	rec->create_parent = parent;
	rec->components    = spatial_component_anchor;
	rec->anchor_pose   = pose;
	if (persist) {
		rec->persist_want = persist_want_persisted;
		rec->name         = string_copy(opt_name_utf8);
	}
	spatial_entity_t handle = rec->handle;
	record_try_create(rec);
	return handle;
}

///////////////////////////////////////////

// Leaves the list at frame end, so app code gets one just_inactive frame
bool32_t spatial_entity_destroy(spatial_entity_t entity) {
	spatial_record_t* rec = record_find(entity);
	if (rec == nullptr || !record_listed(rec)) return false;
	bool backed = rec->life == record_life_live;
	if (backed && (local.destroy == nullptr || (rec->components & spatial_component_anchor) == 0)) return false;

	// Destroyed means gone for good, so it leaves storage too
	spatial_entity_unpersist(entity);
	if (backed) local.destroy(rec->backend_id);
	record_leave(rec, record_life_destroyed);
	return true;
}

///////////////////////////////////////////

bool32_t spatial_entity_get_uuid(spatial_entity_t entity, sk_uuid_t* out_uuid) {
	spatial_record_t* rec = record_with(entity, spatial_component_persistence);
	*out_uuid = rec == nullptr ? sk_uuid_t{} : rec->persist_id;
	return rec != nullptr;
}

bool32_t spatial_entity_persist(spatial_entity_t entity) {
	spatial_record_t* rec = record_find(entity);
	if (rec == nullptr || !record_listed(rec)) return false;
	if (!cap_persistable(rec->source)) {
		log_warn("spatial_entity_persist: this entity can't be persisted on this system");
		return false;
	}

	rec->request_failed = false;
	rec->persist_want   = persist_want_persisted;
	persist_reconcile(rec);
	return true;
}

bool32_t spatial_entity_unpersist(spatial_entity_t entity) {
	spatial_record_t* rec = record_find(entity);
	if (rec == nullptr) return false;

	rec->request_failed = false;
	rec->persist_want   = persist_want_unpersisted;
	persist_reconcile(rec);
	return true;
}

bool32_t spatial_entity_unpersist_uuid(sk_uuid_t persist_id) {
	if (uuid_is_nil(persist_id)) return false;

	spatial_record_t* rec = record_find_persisted(persist_id);
	if (rec != nullptr) return spatial_entity_unpersist(rec->handle);

	if (!cap_persistable(spatial_capability_anchor)) return false;
	unpersist_send(0, persist_id);
	return true;
}

///////////////////////////////////////////
// System lifecycle                      //
///////////////////////////////////////////

bool spatial_init() {
	spatial_settings_t settings = local.settings;
	local          = {};
	local.settings = settings;
	return true;
}

///////////////////////////////////////////

void spatial_step() {
	// Anything added from here on belongs to next frame's New
	local.arrived.clear();

	// Anchor assets need removed entities' data, so they go before the free
	spatial_anchors_on_removed();

	// App code has had its just_inactive frame, so these can go now
	for (int32_t i = 0; i < local.removed.count; i++) {
		spatial_record_t* rec = record_find(local.removed[i]);
		if (rec == nullptr) continue;

		int32_t idx = local.live.index_of(local.removed[i]);
		if (idx >= 0) local.live.remove(idx);

		// Nobody can learn a persist id that lands now, so it gets undone
		if (rec->persist_flight != persist_flight_none) {
			rec->life         = record_life_detached;
			rec->persist_want = persist_want_unpersisted;
			local.detached.add(rec->handle);
		} else {
			record_free(rec);
		}
	}
	local.removed.clear();

	for (int32_t i = 0; i < local.arrived_next.count; i++)
		if (record_find(local.arrived_next[i]) != nullptr) local.arrived.add(local.arrived_next[i]);
	local.arrived_next.clear();
	local.list_version++;

	for (int32_t i = 0; i < local.live.count; i++) {
		spatial_record_t* rec = record_find(local.live[i]);
		if (rec == nullptr) continue;
		rec->changed = spatial_component_none;
		rec->tracked = rec->tracked & ~button_state_changed;

		// Edges set here are for the coming frame, so these go after the clear
		if      (rec->life == record_life_pending_create) record_try_create(rec);
		else if (rec->life == record_life_pending_find && !cap_persistable(rec->source)) record_leave(rec, record_life_failed);
		else                                              persist_reconcile(rec);
	}

	// Persistence can vanish after startup, like from a denied permission
	if (!cap_persistable(spatial_capability_anchor)) {
		for (int32_t i = 0; i < local.unpersist_reqs.count; i++)
			if (local.unpersist_reqs[i].entity != 0) spatial_backend_unpersist_failed(local.unpersist_reqs[i].entity);
		local.unpersist_reqs.clear();
	}
	if (local.unpersist != nullptr && local.unpersist_reqs.count > 0) {
		for (int32_t i = 0; i < local.unpersist_reqs.count; i++)
			local.unpersist(local.unpersist_reqs[i].entity, local.unpersist_reqs[i].persist_id);
		local.unpersist_reqs.clear();
	}
}

///////////////////////////////////////////

void spatial_shutdown() {
	for (int32_t i = 0; i < local.slots.count; i++) {
		if (local.slots[i].alive) record_free(&local.slots[i]);
	}
	local.slots         .free();
	local.slot_free     .free();
	local.live          .free();
	local.arrived       .free();
	local.arrived_next  .free();
	local.removed       .free();
	local.detached      .free();
	local.unpersist_reqs.free();
	local = {};
	spatial_names_shutdown();
}

///////////////////////////////////////////
// Backend provider interface            //
///////////////////////////////////////////

void spatial_backend_set_support(spatial_capability_ caps) {
	local.supported = caps;
}

void spatial_backend_set_cap_comps(spatial_capability_ cap, spatial_component_ comps) {
	int32_t idx = cap_bit_index(cap);
	if (idx >= 0) local.cap_comps[idx] = comps;
}

void spatial_backend_set_create_anchor(spatial_entity_id_t (*create)(pose_t pose, spatial_entity_id_t parent, spatial_entity_t entity)) {
	local.create_anchor = create;
}

void spatial_backend_set_destroy(void (*destroy)(spatial_entity_id_t id)) {
	local.destroy = destroy;
}

void spatial_backend_set_running(spatial_capability_ cap, bool32_t running) {
	if (running) local.running |=  cap;
	else         local.running &= ~cap;
}

///////////////////////////////////////////

// True when the data changed, and takes ownership of replaced buffers
static bool mesh_data_take(spatial_mesh_data_t* dest, const spatial_mesh_data_t* src, bool buffers_changed) {
	dest->origin = src->origin;
	if (!buffers_changed) return false;
	mesh_data_free(dest);
	*dest = *src;
	return true;
}

// Flags a component changed when its value differs from the last one.
template <typename T>
static void take_value(spatial_record_t* rec, spatial_component_ component, T* dest, const T& src) {
	if ((rec->components & component) && memcmp(dest, &src, sizeof(T)) != 0) rec->changed |= component;
	*dest = src;
}

void spatial_backend_ingest(spatial_capability_ source, spatial_ingest_t* entities, int32_t count) {
	local.list_version++;
	for (int32_t i = 0; i < count; i++) {
		spatial_ingest_t* in  = &entities[i];
		spatial_record_t* rec = record_find(in->entity);

		if (rec == nullptr) {
			// Never surface an entity that's already permanently gone
			if (in->tracking == spatial_tracking_stopped) continue;

			// An app lookup may already be waiting on this id
			spatial_record_t* found = (in->present & spatial_component_persistence) ? record_find_persisted(in->persist_id) : nullptr;
			if (found != nullptr && found->life == record_life_pending_find) {
				rec         = found;
				rec->life   = record_life_live;
				rec->source = source;
			} else {
				rec = record_add(source, record_life_live);
			}
			rec->backend_id = in->id;
			in->entity      = rec->handle;
		}

		if (in->tracking == spatial_tracking_stopped) {
			record_lose(rec);
			continue;
		}

		// Keeps just_* edges from earlier ingests this frame, spatial_step clears them
		bool was = (rec->tracked & button_state_active) != 0;
		bool is  = in->tracking == spatial_tracking_tracking;
		button_state_ edges = rec->tracked & button_state_changed;
		rec->tracked = is ? button_state_active : button_state_inactive;
		if      ( is && !was) rec->tracked |= button_state_just_active;
		else if (!is &&  was) rec->tracked |= button_state_just_inactive;
		rec->tracked |= edges;

		// Persistence data is valid regardless of tracking state
		if (in->present & spatial_component_persistence) {
			if ((rec->components & spatial_component_persistence) == 0 || !uuid_eq(rec->persist_id, in->persist_id))
				rec->changed |= spatial_component_persistence;
			rec->persist_id  = in->persist_id;
			rec->components |= spatial_component_persistence;
		}
		persist_reconcile(rec);

		// Paused entities keep their last-known data
		if (!is) continue;

		rec->changed |= in->present & ~rec->components;

		if (in->present & spatial_component_bounds2d) {
			rec->bounds2d_center = in->bounds2d_center;
			rec->bounds2d_size   = in->bounds2d_size;
		}
		if (in->present & spatial_component_bounds3d) {
			rec->bounds3d_center = in->bounds3d_center;
			rec->bounds3d_size   = in->bounds3d_size;
		}
		if (in->present & spatial_component_anchor)          rec->anchor_pose = in->anchor_pose;
		if (in->present & spatial_component_parent)          take_value(rec, spatial_component_parent,          &rec->parent,          in->parent);
		if (in->present & spatial_component_plane_alignment) take_value(rec, spatial_component_plane_alignment, &rec->plane_alignment, in->plane_alignment);
		if (in->present & spatial_component_label)           take_value(rec, spatial_component_label,           &rec->label,           in->label);
		if ((in->present & spatial_component_mesh  ) && mesh_data_take(&rec->mesh,   &in->mesh,   (in->buffers_changed & spatial_component_mesh  ) != 0)) rec->changed |= spatial_component_mesh;
		if ((in->present & spatial_component_mesh2d) && mesh_data_take(&rec->mesh2d, &in->mesh2d, (in->buffers_changed & spatial_component_mesh2d) != 0)) rec->changed |= spatial_component_mesh2d;
		if (in->present & spatial_component_marker) {
			rec->marker_type = in->marker_type;
			rec->marker_id   = in->marker_id;
			if (in->buffers_changed & spatial_component_marker) {
				sk_free(rec->marker_text);
				sk_free(rec->marker_data);
				rec->marker_text      = in->marker_text;
				rec->marker_data      = in->marker_data;
				rec->marker_data_size = in->marker_data_size;
				rec->changed |= spatial_component_marker;
			}
		}
		if (in->present & spatial_component_polygon) {
			rec->polygon_origin = in->polygon_origin;
			if (in->buffers_changed & spatial_component_polygon) {
				sk_free(rec->polygon_verts);
				rec->polygon_verts = in->polygon_verts;
				rec->polygon_count = in->polygon_count;
				rec->changed |= spatial_component_polygon;
			}
		}
		rec->components |= in->present;
	}
}

///////////////////////////////////////////

void spatial_backend_drop_source(spatial_capability_ source) {
	for (int32_t i = 0; i < local.live.count; i++) {
		spatial_record_t* rec = record_find(local.live[i]);
		if (rec != nullptr && rec->source == source && record_listed(rec))
			record_lose(rec);
	}
}

///////////////////////////////////////////

void spatial_backend_set_persist_ops(void (*persist)(spatial_entity_id_t id, spatial_entity_t entity), void (*unpersist)(spatial_entity_t entity, sk_uuid_t persist_id)) {
	local.persist   = persist;
	local.unpersist = unpersist;
}

// A failure only counts against the request that was still wanted.
static void persist_landed(spatial_record_t* rec, bool failed, persist_want_ attempted) {
	rec->persist_flight = persist_flight_none;
	if (failed && rec->persist_want == attempted) {
		rec->persist_want   = persist_want_none;
		rec->request_failed = true;
	}
	persist_reconcile(rec);
}

static void persist_set(spatial_record_t* rec, bool persisted, sk_uuid_t persist_id) {
	rec->persist_id  = persist_id;
	rec->components  = persisted
		? rec->components |  spatial_component_persistence
		: rec->components & ~spatial_component_persistence;
	rec->changed    |= spatial_component_persistence;
	local.list_version++;
}

void spatial_backend_set_persist(spatial_entity_t entity, sk_uuid_t persist_id) {
	spatial_record_t* rec = record_find_any(entity);
	if (rec == nullptr) return;
	persist_set   (rec, true, persist_id);
	if (rec->name != nullptr) spatial_names_set(persist_id, rec->name);
	persist_landed(rec, false, persist_want_persisted);
}

void spatial_backend_persist_failed(spatial_entity_t entity) {
	spatial_record_t* rec = record_find_any(entity);
	if (rec == nullptr) return;
	persist_landed(rec, true, persist_want_persisted);
}

void spatial_backend_clear_persist(spatial_entity_t entity) {
	spatial_record_t* rec = record_find_any(entity);
	if (rec == nullptr) return;
	persist_set   (rec, false, {});
	persist_landed(rec, false, persist_want_unpersisted);
}

void spatial_backend_unpersist_failed(spatial_entity_t entity) {
	spatial_record_t* rec = record_find_any(entity);
	if (rec == nullptr) return;
	persist_landed(rec, true, persist_want_unpersisted);
}

void spatial_backend_persist_not_found(sk_uuid_t persist_id) {
	spatial_names_remove(persist_id);
	spatial_record_t* rec = record_find_persisted(persist_id);
	if (rec != nullptr && rec->life == record_life_pending_find) record_leave(rec, record_life_failed);
}

int32_t spatial_backend_get_find_ids(sk_uuid_t* out_ids, int32_t capacity) {
	int32_t count = 0;
	for (int32_t i = 0; i < local.live.count; i++) {
		spatial_record_t* rec = record_find(local.live[i]);
		// Skips ones being unpersisted, which shouldn't load
		if (rec == nullptr || rec->life != record_life_pending_find || rec->persist_want != persist_want_none || rec->persist_flight != persist_flight_none) continue;
		if (count < capacity) out_ids[count] = rec->persist_id;
		count++;
	}
	return count;
}

///////////////////////////////////////////

uint32_t spatial_backend_get_config_serial(spatial_capability_ cap) {
	int32_t idx = cap_bit_index(cap);
	return idx < 0 ? 0 : local.settings.config_serials[idx];
}

}
