/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 * Copyright (c) 2026 Qualcomm Technologies, Inc.
 */

#include "spatial_entity.h"
#include "../sk_memory.h"
#include "../libraries/array.h"
#include "../asset_types/mesh_.h"

#include <string.h>

namespace sk {

///////////////////////////////////////////
// Types                                 //
///////////////////////////////////////////

// Entity records live in a slot map: a spatial_entity_t handle is a
// 16 bit slot index plus a 16 bit generation, and freeing a slot bumps
// its generation, so stale handles fail to resolve instead of aliasing
// a newer entity. Generations start at 1, so 0 is never a valid handle.
struct spatial_record_t {
	bool32_t            alive;
	uint16_t            generation;
	spatial_entity_t    handle;
	spatial_entity_id_t backend_id; // The backend's own id for this entity
	spatial_capability_ source;
	button_state_       tracked;
	spatial_component_  components; // Components with valid last-known data
	spatial_component_  changed;    // Cleared each frame in spatial_step
	bool32_t            in_list;    // Still present in the live entity list
	bool32_t            persist_requested; // Persist deferred until the entity is tracking

	pose_t              bounds2d_center;
	vec2                bounds2d_size;
	pose_t              bounds3d_center;
	vec3                bounds3d_size;
	spatial_entity_id_t parent;
	pose_t              anchor_pose;
	plane_align_        plane_alignment;
	plane_label_        plane_label;
	marker_type_        marker_type;
	uint32_t            marker_id;

	pose_t              mesh_origin;
	vec3*               mesh_verts;
	int32_t             mesh_vert_count;
	uint32_t*           mesh_inds;
	int32_t             mesh_ind_count;
	pose_t              mesh2d_origin;
	vec3*               mesh2d_verts;
	int32_t             mesh2d_vert_count;
	uint32_t*           mesh2d_inds;
	int32_t             mesh2d_ind_count;
	pose_t              polygon_origin;
	vec2*               polygon_verts;
	int32_t             polygon_count;
	char*               marker_text;
	uint8_t*            marker_data;
	int32_t             marker_data_size;
	uint8_t             persist_uuid[16];
};

struct spatial_state_t {
	spatial_capability_ supported;
	spatial_component_  cap_comps[32]; // Indexed by capability bit position
	spatial_capability_ requested;
	spatial_capability_ requested_system; // Internal systems' requests, immune to spatial_disable
	spatial_capability_ active;
	spatial_entity_id_t (*create_anchor)(pose_t pose);
	void                (*destroy)      (spatial_entity_id_t id);
	void                (*persist)      (spatial_entity_id_t id);
	void                (*unpersist)    (spatial_entity_id_t id, const uint8_t* uuid_16);

	spatial_marker_config_t marker_configs[32]; // Indexed by capability bit position
	uint32_t                config_serials[32];

	array_t<spatial_record_t> slots;
	array_t<int32_t>          slot_free;
	array_t<spatial_entity_t> live;    // Handles in enumeration order
	array_t<spatial_entity_t> arrived; // Appeared this frame
	array_t<spatial_entity_t> stopped; // Remove from the live list at spatial_step
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

static spatial_record_t* record_find(spatial_entity_t handle) {
	int32_t  idx = (int32_t )(handle & 0xFFFF);
	uint16_t gen = (uint16_t)(handle >> 16);
	if (gen == 0 || idx >= local.slots.count) return nullptr;

	spatial_record_t* rec = &local.slots[idx];
	return rec->alive && rec->generation == gen ? rec : nullptr;
}

static spatial_record_t* record_find_backend(spatial_entity_id_t backend_id) {
	for (int32_t i = 0; i < local.live.count; i++) {
		spatial_record_t* rec = record_find(local.live[i]);
		if (rec != nullptr && rec->backend_id == backend_id) return rec;
	}
	return nullptr;
}

// Note that allocation may grow the slot array, invalidating any
// previously held record pointers.
static spatial_record_t* record_alloc() {
	int32_t idx;
	if (local.slot_free.count > 0) {
		idx = local.slot_free[local.slot_free.count - 1];
		local.slot_free.remove(local.slot_free.count - 1);
	} else {
		local.slots.add({});
		idx = local.slots.count - 1;
	}

	spatial_record_t* rec = &local.slots[idx];
	uint16_t          gen = rec->generation == 0 ? 1 : rec->generation;
	*rec = {};
	rec->alive      = true;
	rec->generation = gen;
	rec->handle     = (spatial_entity_t)((uint32_t)gen << 16 | (uint32_t)idx);
	return rec;
}

static void record_free(spatial_record_t* rec) {
	sk_free(rec->mesh_verts);
	sk_free(rec->mesh_inds);
	sk_free(rec->mesh2d_verts);
	sk_free(rec->mesh2d_inds);
	sk_free(rec->polygon_verts);
	sk_free(rec->marker_text);
	sk_free(rec->marker_data);

	int32_t  idx = (int32_t)(rec->handle & 0xFFFF);
	uint16_t gen = rec->generation + 1;
	if (gen == 0) gen = 1;

	*rec = {};
	rec->generation = gen;
	local.slot_free.add(idx);
}

// An entity is gone for good: mark it lost and queue removal from the
// live list at the end of the frame.
static void record_stop(spatial_record_t* rec) {
	if (!rec->in_list) return;
	rec->in_list = false;
	rec->tracked = (rec->tracked & button_state_active)
		? (button_state_)(button_state_inactive | button_state_just_inactive)
		: button_state_inactive;
	local.stopped.add(rec->handle);
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

void spatial_enable(spatial_capability_ capabilities) {
	local.requested |= capabilities;
}

void spatial_enable_marker(spatial_capability_ capabilities, spatial_marker_config_t config) {
	const spatial_capability_ marker_caps = spatial_capability_qr_code | spatial_capability_micro_qr | spatial_capability_aruco | spatial_capability_april_tag;
	if ((capabilities & ~marker_caps) != spatial_capability_none)
		log_warn("spatial_enable_marker: ignoring non-marker capabilities");

	for (int32_t i = 0; i < 32; i++) {
		spatial_capability_ cap = (spatial_capability_)(1 << i);
		if ((capabilities & marker_caps & cap) == 0) continue;

		if (memcmp(&local.marker_configs[i], &config, sizeof(config)) != 0) {
			local.marker_configs[i] = config;
			local.config_serials[i]++;
		}
	}
	spatial_enable(capabilities & marker_caps);
}

void spatial_disable(spatial_capability_ capabilities) {
	local.requested &= ~capabilities;
}

void spatial_enable_system(spatial_capability_ capabilities) {
	local.requested_system |= capabilities;
}

void spatial_disable_system(spatial_capability_ capabilities) {
	local.requested_system &= ~capabilities;
}

spatial_capability_ spatial_get_enabled() {
	return local.requested | local.requested_system;
}

spatial_capability_ spatial_get_active() {
	return local.active;
}

///////////////////////////////////////////
// Entity list                           //
///////////////////////////////////////////

int32_t spatial_entity_get_count(spatial_component_ with_components) {
	if (with_components == spatial_component_none)
		return local.live.count;

	int32_t result = 0;
	for (int32_t i = 0; i < local.live.count; i++) {
		spatial_record_t* rec = record_find(local.live[i]);
		if (rec != nullptr && (rec->components & with_components) == with_components) result++;
	}
	return result;
}

spatial_entity_t spatial_entity_get_index(spatial_component_ with_components, int32_t index) {
	if (with_components == spatial_component_none)
		return index >= 0 && index < local.live.count ? local.live[index] : 0;

	int32_t curr = 0;
	for (int32_t i = 0; i < local.live.count; i++) {
		spatial_record_t* rec = record_find(local.live[i]);
		if (rec == nullptr || (rec->components & with_components) != with_components) continue;
		if (curr == index) return local.live[i];
		curr++;
	}
	return 0;
}

int32_t spatial_entity_get_new_count() {
	return local.arrived.count;
}

spatial_entity_t spatial_entity_get_new_index(int32_t index) {
	return index >= 0 && index < local.arrived.count ? local.arrived[index] : 0;
}

///////////////////////////////////////////
// Entity data                           //
///////////////////////////////////////////

bool32_t spatial_entity_is_valid(spatial_entity_t entity) {
	return record_find(entity) != nullptr;
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

spatial_entity_t spatial_entity_get_parent(spatial_entity_t entity) {
	spatial_record_t* rec = record_find(entity);
	if (rec == nullptr || (rec->components & spatial_component_parent) == 0) return 0;

	spatial_record_t* parent = record_find_backend(rec->parent);
	return parent == nullptr ? 0 : parent->handle;
}

///////////////////////////////////////////

bool32_t spatial_entity_get_anchor(spatial_entity_t entity, pose_t* out_pose) {
	spatial_record_t* rec = record_find(entity);
	if (rec == nullptr || (rec->components & spatial_component_anchor) == 0) return false;
	*out_pose = rec->anchor_pose;
	return true;
}

bool32_t spatial_entity_get_bounds2d(spatial_entity_t entity, pose_t* out_center, vec2* out_size) {
	spatial_record_t* rec = record_find(entity);
	if (rec == nullptr || (rec->components & spatial_component_bounds2d) == 0) return false;
	*out_center = rec->bounds2d_center;
	*out_size   = rec->bounds2d_size;
	return true;
}

bool32_t spatial_entity_get_bounds3d(spatial_entity_t entity, pose_t* out_center, vec3* out_size) {
	spatial_record_t* rec = record_find(entity);
	if (rec == nullptr || (rec->components & spatial_component_bounds3d) == 0) return false;
	*out_center = rec->bounds3d_center;
	*out_size   = rec->bounds3d_size;
	return true;
}

bool32_t spatial_entity_get_plane(spatial_entity_t entity, plane_align_* out_alignment, plane_label_* out_label) {
	spatial_record_t* rec = record_find(entity);
	if (rec == nullptr || (rec->components & (spatial_component_plane_alignment | spatial_component_plane_label)) == 0) return false;
	*out_alignment = rec->plane_alignment;
	*out_label     = rec->plane_label;
	return true;
}

static bool32_t fill_mesh(mesh_t ref_mesh, const vec3* verts, int32_t vert_count, const uint32_t* inds, int32_t ind_count) {
	if (vert_count <= 0 || ind_count <= 0) return false;

	vert_t* mesh_verts = sk_malloc_t(vert_t, vert_count);
	for (int32_t i = 0; i < vert_count; i++)
		mesh_verts[i] = { verts[i], {0,1,0}, {0,0}, {255,255,255,255} };
	mesh_calculate_normals(mesh_verts, vert_count, (const vind_t*)inds, ind_count);
	mesh_set_data(ref_mesh, mesh_verts, vert_count, (const vind_t*)inds, ind_count);

	sk_free(mesh_verts);
	return true;
}

// ref_mesh may be null to retrieve only the origin pose, filling the
// mesh is the expensive path.
bool32_t spatial_entity_get_mesh(spatial_entity_t entity, mesh_t ref_mesh, pose_t* out_origin) {
	spatial_record_t* rec = record_find(entity);
	if (rec == nullptr || (rec->components & spatial_component_mesh) == 0) return false;
	*out_origin = rec->mesh_origin;
	return ref_mesh == nullptr
		? (bool32_t)true
		: fill_mesh(ref_mesh, rec->mesh_verts, rec->mesh_vert_count, rec->mesh_inds, rec->mesh_ind_count);
}

bool32_t spatial_entity_get_mesh2d(spatial_entity_t entity, mesh_t ref_mesh, pose_t* out_origin) {
	spatial_record_t* rec = record_find(entity);
	if (rec == nullptr || (rec->components & spatial_component_mesh2d) == 0) return false;
	*out_origin = rec->mesh2d_origin;
	return ref_mesh == nullptr
		? (bool32_t)true
		: fill_mesh(ref_mesh, rec->mesh2d_verts, rec->mesh2d_vert_count, rec->mesh2d_inds, rec->mesh2d_ind_count);
}

bool32_t spatial_entity_get_polygon(spatial_entity_t entity, pose_t* out_origin, const vec2** out_verts, int32_t* out_count) {
	spatial_record_t* rec = record_find(entity);
	if (rec == nullptr || (rec->components & spatial_component_polygon) == 0) return false;
	*out_origin = rec->polygon_origin;
	*out_verts  = rec->polygon_verts;
	*out_count  = rec->polygon_count;
	return true;
}

bool32_t spatial_entity_get_marker(spatial_entity_t entity, marker_type_* out_type, uint32_t* out_marker_id) {
	spatial_record_t* rec = record_find(entity);
	if (rec == nullptr || (rec->components & spatial_component_marker) == 0) return false;
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

spatial_entity_t spatial_entity_create_anchor(pose_t pose) {
	if (local.create_anchor == nullptr || (local.active & spatial_capability_anchor) == 0) {
		log_warn("spatial_entity_create_anchor: anchor capability is not active");
		return 0;
	}

	spatial_entity_id_t id = local.create_anchor(pose);
	if (id == 0) return 0;

	spatial_record_t* rec = record_find_backend(id);
	return rec == nullptr ? 0 : rec->handle;
}

///////////////////////////////////////////

// The entity leaves the list at the end of the frame, so app code gets
// one frame to see it as just_inactive.
bool32_t spatial_entity_destroy(spatial_entity_t entity) {
	spatial_record_t* rec = record_find(entity);
	if (rec == nullptr || local.destroy == nullptr || !rec->in_list) return false;
	if ((rec->components & spatial_component_anchor) == 0)           return false;

	if (rec->components & spatial_component_persistence)
		spatial_entity_unpersist(entity);
	local.destroy(rec->backend_id);
	record_stop(rec);
	return true;
}

///////////////////////////////////////////

bool32_t spatial_entity_get_persist_id(spatial_entity_t entity, uint8_t* out_uuid_16) {
	spatial_record_t* rec = record_find(entity);
	if (rec == nullptr || (rec->components & spatial_component_persistence) == 0) {
		memset(out_uuid_16, 0, 16);
		return false;
	}
	memcpy(out_uuid_16, rec->persist_uuid, 16);
	return true;
}

// Can entities be written to persistent storage on this system? This
// may take a moment to become true while the system starts up.
bool32_t spatial_persistence_available() {
	return local.persist != nullptr;
}

// Runtimes can't reliably persist an untracked entity, so requests for
// entities that aren't tracking yet wait until they are.
void spatial_entity_persist(spatial_entity_t entity) {
	if (local.persist == nullptr) {
		log_warn("spatial_entity_persist: persistence isn't available on this system");
		return;
	}
	spatial_record_t* rec = record_find(entity);
	if (rec == nullptr || (rec->components & spatial_component_persistence)) return;

	if ((rec->tracked & button_state_active) == 0) {
		rec->persist_requested = true;
		return;
	}
	local.persist(rec->backend_id);
}

void spatial_entity_unpersist(spatial_entity_t entity) {
	if (local.unpersist == nullptr) return;
	spatial_record_t* rec = record_find(entity);
	if (rec == nullptr || (rec->components & spatial_component_persistence) == 0) return;
	local.unpersist(rec->backend_id, rec->persist_uuid);
}

///////////////////////////////////////////
// System lifecycle                      //
///////////////////////////////////////////

bool spatial_init() {
	local = {};
	return true;
}

///////////////////////////////////////////

void spatial_step() {
	// Remove entities that stopped this frame, now that app code has had
	// a chance to see their just_inactive state.
	for (int32_t i = 0; i < local.stopped.count; i++) {
		spatial_record_t* rec = record_find(local.stopped[i]);
		if (rec == nullptr) continue;

		int32_t idx = local.live.index_of(local.stopped[i]);
		if (idx >= 0) local.live.remove(idx);
		record_free(rec);
	}
	local.stopped.clear();
	local.arrived.clear();

	for (int32_t i = 0; i < local.live.count; i++) {
		spatial_record_t* rec = record_find(local.live[i]);
		if (rec == nullptr) continue;
		rec->changed = spatial_component_none;
		rec->tracked = rec->tracked & ~button_state_changed;
	}
}

///////////////////////////////////////////

void spatial_shutdown() {
	for (int32_t i = 0; i < local.live.count; i++) {
		spatial_record_t* rec = record_find(local.live[i]);
		if (rec != nullptr) record_free(rec);
	}
	local.slots    .free();
	local.slot_free.free();
	local.live     .free();
	local.arrived  .free();
	local.stopped  .free();
	local = {};
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

void spatial_backend_set_create_anchor(spatial_entity_id_t (*create)(pose_t pose)) {
	local.create_anchor = create;
}

void spatial_backend_set_destroy(void (*destroy)(spatial_entity_id_t id)) {
	local.destroy = destroy;
}

void spatial_backend_set_active(spatial_capability_ cap, bool32_t active) {
	if (active) local.active |=  cap;
	else        local.active &= ~cap;
}

///////////////////////////////////////////

void spatial_backend_ingest(spatial_capability_ source, const spatial_ingest_t* entities, int32_t count) {
	for (int32_t i = 0; i < count; i++) {
		const spatial_ingest_t* in  = &entities[i];
		spatial_record_t*       rec = record_find_backend(in->id);

		if (rec == nullptr) {
			// Never surface an entity that's already permanently gone
			if (in->tracking == spatial_tracking_stopped) continue;

			rec = record_alloc();
			rec->backend_id = in->id;
			rec->source     = source;
			rec->in_list    = true;
			local.live   .add(rec->handle);
			local.arrived.add(rec->handle);
		}

		if (in->tracking == spatial_tracking_stopped) {
			record_stop(rec);
			continue;
		}

		// Tracking state edges. Preserve just_* marks from earlier
		// ingests this frame; spatial_step clears them.
		bool was = (rec->tracked & button_state_active) != 0;
		bool is  = in->tracking == spatial_tracking_tracking;
		button_state_ edges = rec->tracked & button_state_changed;
		rec->tracked = is ? button_state_active : button_state_inactive;
		if      ( is && !was) rec->tracked |= button_state_just_active;
		else if (!is &&  was) rec->tracked |= button_state_just_inactive;
		rec->tracked |= edges;

		// Dispatch any persist request that was waiting on tracking
		if (is && rec->persist_requested) {
			rec->persist_requested = false;
			if (local.persist != nullptr && (rec->components & spatial_component_persistence) == 0)
				local.persist(rec->backend_id);
		}

		// Persistence data is valid regardless of tracking state
		if (in->present & spatial_component_persistence) {
			if ((rec->components & spatial_component_persistence) == 0 || memcmp(rec->persist_uuid, in->persist_uuid, 16) != 0)
				rec->changed |= spatial_component_persistence;
			memcpy(rec->persist_uuid, in->persist_uuid, 16);
			rec->components |= spatial_component_persistence;
		}

		// Other component data is only valid while tracking; paused
		// entities keep their last-known data.
		if (!is) continue;

		spatial_component_ arrived_comps = in->present & ~rec->components;
		rec->changed |= arrived_comps;

		if (in->present & spatial_component_bounds2d) {
			rec->bounds2d_center = in->bounds2d_center;
			rec->bounds2d_size   = in->bounds2d_size;
		}
		if (in->present & spatial_component_bounds3d) {
			rec->bounds3d_center = in->bounds3d_center;
			rec->bounds3d_size   = in->bounds3d_size;
		}
		if (in->present & spatial_component_anchor) {
			rec->anchor_pose = in->anchor_pose;
		}
		if (in->present & spatial_component_parent) {
			if ((rec->components & spatial_component_parent) && rec->parent != in->parent) rec->changed |= spatial_component_parent;
			rec->parent = in->parent;
		}
		if (in->present & spatial_component_plane_alignment) {
			if ((rec->components & spatial_component_plane_alignment) && rec->plane_alignment != in->plane_alignment) rec->changed |= spatial_component_plane_alignment;
			rec->plane_alignment = in->plane_alignment;
		}
		if (in->present & spatial_component_plane_label) {
			if ((rec->components & spatial_component_plane_label) && rec->plane_label != in->plane_label) rec->changed |= spatial_component_plane_label;
			rec->plane_label = in->plane_label;
		}
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
		if (in->present & spatial_component_mesh) {
			rec->mesh_origin = in->mesh_origin;
			if (in->buffers_changed & spatial_component_mesh) {
				sk_free(rec->mesh_verts);
				sk_free(rec->mesh_inds);
				rec->mesh_verts      = in->mesh_verts;
				rec->mesh_vert_count = in->mesh_vert_count;
				rec->mesh_inds       = in->mesh_inds;
				rec->mesh_ind_count  = in->mesh_ind_count;
				rec->changed |= spatial_component_mesh;
			}
		}
		if (in->present & spatial_component_mesh2d) {
			rec->mesh2d_origin = in->mesh2d_origin;
			if (in->buffers_changed & spatial_component_mesh2d) {
				sk_free(rec->mesh2d_verts);
				sk_free(rec->mesh2d_inds);
				rec->mesh2d_verts      = in->mesh2d_verts;
				rec->mesh2d_vert_count = in->mesh2d_vert_count;
				rec->mesh2d_inds       = in->mesh2d_inds;
				rec->mesh2d_ind_count  = in->mesh2d_ind_count;
				rec->changed |= spatial_component_mesh2d;
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
		if (rec != nullptr && rec->source == source)
			record_stop(rec);
	}
}

///////////////////////////////////////////

void spatial_backend_set_persist_ops(void (*persist)(spatial_entity_id_t id), void (*unpersist)(spatial_entity_id_t id, const uint8_t* uuid_16)) {
	local.persist   = persist;
	local.unpersist = unpersist;
}

void spatial_backend_set_persist(spatial_entity_id_t id, const uint8_t* uuid_16) {
	spatial_record_t* rec = record_find_backend(id);
	if (rec == nullptr) return;
	memcpy(rec->persist_uuid, uuid_16, 16);
	rec->components |= spatial_component_persistence;
	rec->changed    |= spatial_component_persistence;
}

void spatial_backend_clear_persist(spatial_entity_id_t id) {
	spatial_record_t* rec = record_find_backend(id);
	if (rec == nullptr) return;
	memset(rec->persist_uuid, 0, 16);
	rec->components &= ~spatial_component_persistence;
	rec->changed    |= spatial_component_persistence;
}

///////////////////////////////////////////

spatial_marker_config_t spatial_backend_get_marker_config(spatial_capability_ cap) {
	int32_t idx = cap_bit_index(cap);
	return idx < 0 ? spatial_marker_config_t{} : local.marker_configs[idx];
}

uint32_t spatial_backend_get_config_serial(spatial_capability_ cap) {
	int32_t idx = cap_bit_index(cap);
	return idx < 0 ? 0 : local.config_serials[idx];
}

}
