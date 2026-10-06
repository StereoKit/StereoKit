// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith

#include "spatial_tests.h"

#include <stereokit.h>

// Internal header, reached through StereoKitC's public include root. The
// fake backend below stands in for OpenXR through the provider interface.
#include <systems/spatial_entity.h>

#include <string.h>

using namespace sk;

///////////////////////////////////////////

static int spt_failures = 0;

#define SPT_CHECK(condition, description) do { \
	if (condition) { log_infof("[spatial_test] pass: %s", description); } \
	else           { log_errf ("[spatial_test] FAIL: %s", description); spt_failures += 1; } \
	} while (0)

///////////////////////////////////////////

// Async operations are only recorded, the tests complete them by hand.
struct spt_unpersist_t {
	spatial_entity_t entity;
	sk_uuid_t        id;
};
struct spt_backend_t {
	spatial_entity_id_t next_id;
	int32_t             destroyed;
	spatial_entity_t    persists  [16];
	int32_t             persist_count;
	spt_unpersist_t     unpersists[16];
	int32_t             unpersist_count;
};
static spt_backend_t spt = {};

static spatial_entity_id_t spt_create(pose_t, spatial_entity_id_t parent, spatial_entity_t) { return parent != 0 ? 0 : ++spt.next_id; }
static void spt_destroy  (spatial_entity_id_t) { spt.destroyed += 1; }
static void spt_persist  (spatial_entity_id_t, spatial_entity_t entity) { if (spt.persist_count   < 16) spt.persists  [spt.persist_count  ++] = entity; }
static void spt_unpersist(spatial_entity_t entity, sk_uuid_t id)        { if (spt.unpersist_count < 16) spt.unpersists[spt.unpersist_count++] = { entity, id }; }

// Keeps ids climbing, so backend ids never repeat between tests.
static void spt_reset() {
	spatial_entity_id_t next_id = spt.next_id;
	spt         = {};
	spt.next_id = next_id;
}

static sk_uuid_t spt_uuid(uint8_t n) {
	sk_uuid_t result = {};
	result.bytes[0]  = n;
	result.bytes[15] = n;
	return result;
}
static bool spt_uuid_eq(sk_uuid_t a, sk_uuid_t b) { return memcmp(a.bytes, b.bytes, sizeof(a.bytes)) == 0; }

static sk_uuid_t spt_persist_id(spatial_entity_t entity) {
	sk_uuid_t id;
	spatial_entity_get_uuid(entity, &id);
	return id;
}

static bool spt_in_new(spatial_entity_t entity) {
	for (int32_t i = 0; i < spatial_entity_get_new_count(spatial_component_none); i++)
		if (spatial_entity_get_new_index(spatial_component_none, i) == entity) return true;
	return false;
}

static bool spt_in_removed(spatial_entity_t entity) {
	for (int32_t i = 0; i < spatial_entity_get_removed_count(spatial_component_none); i++)
		if (spatial_entity_get_removed_index(spatial_component_none, i) == entity) return true;
	return false;
}

static bool spt_finding(sk_uuid_t id) {
	sk_uuid_t ids[16];
	int32_t   count = spatial_backend_get_find_ids(ids, 16);
	for (int32_t i = 0; i < count && i < 16; i++)
		if (spt_uuid_eq(ids[i], id)) return true;
	return false;
}

///////////////////////////////////////////

static void spt_test_create_persisted() {
	spt_reset();
	spatial_entity_t e = spatial_entity_create_anchor(pose_identity, true, 0);
	SPT_CHECK(e != 0 && spatial_entity_get_status(e) == spatial_status_pending, "persisting a new anchor starts Pending");
	SPT_CHECK(spt.persist_count == 1 && spt.persists[0] == e,                  "a tracked anchor persists right away");
	SPT_CHECK(!spt_in_new(e),                                                   "app-created entities skip New the frame they're made");

	sk_step(nullptr);
	SPT_CHECK(spt_in_new(e), "app-created entities show up in New the following frame");

	spatial_backend_set_persist(e, spt_uuid(1));
	SPT_CHECK(spatial_entity_get_status(e) == spatial_status_ready && spt_uuid_eq(spt_persist_id(e), spt_uuid(1)), "a landed persist is Ready with its id");

	sk_step(nullptr);
	SPT_CHECK(!spt_in_new(e), "entities are in New for exactly one frame");
}

static void spt_test_undo() {
	spt_reset();
	spatial_entity_t e = spatial_entity_create_anchor(pose_identity, true, 0);
	spatial_entity_unpersist(e);
	SPT_CHECK(spatial_entity_get_status(e) == spatial_status_pending, "unpersisting mid-persist stays Pending");

	spatial_backend_set_persist(e, spt_uuid(2));
	SPT_CHECK(spt.unpersist_count == 1 && spt_uuid_eq(spt.unpersists[0].id, spt_uuid(2)), "a persist the app no longer wants is undone once it lands");

	spatial_backend_clear_persist(e);
	SPT_CHECK(spatial_entity_get_status(e) == spatial_status_ready && (spatial_entity_get_components(e) & spatial_component_persistence) == 0, "the undo ends unpersisted and Ready");
}

static void spt_test_redo() {
	spt_reset();
	spatial_entity_t e = spatial_entity_create_anchor(pose_identity, true, 0);
	spatial_backend_set_persist(e, spt_uuid(3));

	spatial_entity_unpersist(e);
	spatial_entity_persist  (e);
	SPT_CHECK(spt.unpersist_count == 1 && spt.persist_count == 1, "persisting mid-unpersist waits for the unpersist");

	spatial_backend_clear_persist(e);
	SPT_CHECK(spt.persist_count == 2, "the entity persists again once the unpersist lands");

	spatial_backend_set_persist(e, spt_uuid(4));
	SPT_CHECK(spatial_entity_get_status(e) == spatial_status_ready && spt_uuid_eq(spt_persist_id(e), spt_uuid(4)), "the redo ends Ready with the new id");
}

static void spt_test_failure() {
	spt_reset();
	spatial_entity_t e = spatial_entity_create_anchor(pose_identity, true, 0);
	spatial_backend_persist_failed(e);
	SPT_CHECK(spatial_entity_get_status(e) == spatial_status_partial, "a failed persist reports Partial");

	spatial_entity_persist(e);
	SPT_CHECK(spatial_entity_get_status(e) == spatial_status_pending && spt.persist_count == 2, "a new request clears Partial and retries");

	spatial_backend_set_persist(e, spt_uuid(5));
	SPT_CHECK(spatial_entity_get_status(e) == spatial_status_ready, "the retry ends Ready");
}

static void spt_test_destroy_mid_persist() {
	spt_reset();
	spatial_entity_t e = spatial_entity_create_anchor(pose_identity, true, 0);
	SPT_CHECK(spatial_entity_destroy(e) && spt.destroyed == 1 && spt_in_removed(e), "destroy releases the anchor and removes it");

	sk_step(nullptr);
	SPT_CHECK(!spatial_entity_is_valid(e), "a destroyed entity stops resolving after its final frame");

	spatial_backend_set_persist(e, spt_uuid(6));
	SPT_CHECK(spt.unpersist_count == 1 && spt_uuid_eq(spt.unpersists[0].id, spt_uuid(6)), "a persist landing after destroy is undone");

	spatial_backend_clear_persist(e);
	SPT_CHECK(!spatial_entity_is_valid(e) && spatial_entity_get_status(e) == spatial_status_none, "the undo doesn't bring the entity back");
}

static void spt_test_lookup_unpersist() {
	spt_reset();
	spatial_entity_t e = spatial_entity_find_uuid(spt_uuid(7));
	SPT_CHECK(spatial_entity_get_status(e) == spatial_status_pending && spt_finding(spt_uuid(7)), "a lookup is Pending and asks the backend for its id");

	spatial_entity_unpersist(e);
	SPT_CHECK(spatial_entity_is_valid(e) && !spt_in_removed(e) && spt.unpersist_count == 1, "unpersisting a lookup doesn't destroy it");
	SPT_CHECK(!spt_finding(spt_uuid(7)), "a lookup being unpersisted stops loading");

	spatial_backend_clear_persist(e);
	SPT_CHECK(spt_in_removed(e), "a lookup leaves once storage no longer has its id");

	sk_step(nullptr);
	SPT_CHECK(!spatial_entity_is_valid(e), "the finished lookup stops resolving");
}

static void spt_test_destroy_lookup() {
	spt_reset();
	spatial_entity_t e = spatial_entity_find_uuid(spt_uuid(8));
	SPT_CHECK(spatial_entity_destroy(e) && spt.unpersist_count == 1 && spt_in_removed(e), "destroying a lookup removes it from storage and the list");

	sk_step(nullptr);
	spatial_backend_clear_persist(e);
	SPT_CHECK(!spatial_entity_is_valid(e), "a destroyed lookup stays gone once its unpersist lands");
}

static void spt_test_lookup_binds() {
	spt_reset();
	spatial_entity_t e = spatial_entity_find_uuid(spt_uuid(9));

	spatial_ingest_t in = {};
	in.id          = 1000;
	in.tracking    = spatial_tracking_tracking;
	in.present     = spatial_component_persistence | spatial_component_anchor;
	in.persist_id  = spt_uuid(9);
	in.anchor_pose = pose_identity;
	spatial_backend_ingest(spatial_capability_anchor, &in, 1);
	SPT_CHECK(in.entity == e,                                                                   "a lookup binds to the entity storage loads");
	SPT_CHECK(spatial_entity_get_status(e) == spatial_status_ready && (spatial_entity_get_tracked(e) & button_state_active), "a bound lookup is Ready and tracked");
}

static void spt_test_not_persistable() {
	spt_reset();
	spatial_backend_set_cap_comps(spatial_capability_plane_tracking, spatial_component_bounds2d);

	spatial_ingest_t in = {};
	in.id       = 2000;
	in.tracking = spatial_tracking_tracking;
	in.present  = spatial_component_bounds2d;
	spatial_backend_ingest(spatial_capability_plane_tracking, &in, 1);
	SPT_CHECK(!spatial_entity_persist(in.entity) && spatial_entity_get_status(in.entity) == spatial_status_ready, "persisting an unpersistable entity fails without marking it Partial");
}

static void spt_test_parent() {
	spt_reset();
	spatial_ingest_t child = {};
	child.id       = 3000;
	child.tracking = spatial_tracking_tracking;
	child.present  = spatial_component_parent;
	child.parent   = 3001;
	spatial_backend_ingest(spatial_capability_plane_tracking, &child, 1);
	SPT_CHECK(spatial_entity_get_parent(child.entity) == 0, "a parent that hasn't arrived yet resolves to nothing");

	spatial_ingest_t parent = {};
	parent.id       = 3001;
	parent.tracking = spatial_tracking_tracking;
	spatial_backend_ingest(spatial_capability_plane_tracking, &parent, 1);
	SPT_CHECK(spatial_entity_get_parent(child.entity) == parent.entity, "a parent arriving after its child still resolves");

	parent.tracking = spatial_tracking_stopped;
	spatial_backend_ingest(spatial_capability_plane_tracking, &parent, 1);
	sk_step(nullptr);
	SPT_CHECK(spatial_entity_get_parent(child.entity) == 0, "a removed parent stops resolving");

	spatial_ingest_t again = {};
	again.id       = 3001;
	again.tracking = spatial_tracking_tracking;
	spatial_backend_ingest(spatial_capability_plane_tracking, &again, 1);
	SPT_CHECK(again.entity != parent.entity && spatial_entity_get_parent(child.entity) == again.entity, "a parent that returns under a new handle resolves to it");
}

///////////////////////////////////////////

int spatial_tests_run() {
	sk_settings_t settings = {};
	settings.app_name      = "StereoKitC Spatial Tests";
	settings.mode          = app_mode_offscreen;
	settings.standby_mode  = standby_mode_none;
	if (!sk_init(settings)) {
		log_err("[spatial_test] sk_init failed");
		return 1;
	}

	spatial_backend_set_support      (spatial_capability_anchor | spatial_capability_plane_tracking);
	spatial_backend_set_cap_comps    (spatial_capability_anchor, spatial_component_anchor | spatial_component_persistence);
	spatial_backend_set_create_anchor(spt_create);
	spatial_backend_set_destroy      (spt_destroy);
	spatial_backend_set_persist_ops  (spt_persist, spt_unpersist);
	spatial_backend_set_active       (spatial_capability_anchor, true);
	spatial_enable                   (spatial_capability_anchor);

	// spatial_step runs as the next sk_step closes this frame, so tests start
	// inside a frame like app code does.
	sk_step(nullptr);

	spt_test_create_persisted   ();
	spt_test_undo               ();
	spt_test_redo               ();
	spt_test_failure            ();
	spt_test_destroy_mid_persist();
	spt_test_lookup_unpersist   ();
	spt_test_destroy_lookup     ();
	spt_test_lookup_binds       ();
	spt_test_not_persistable    ();
	spt_test_parent             ();

	sk_shutdown();

	if (spt_failures == 0) log_info ("[spatial_test] all tests passed!");
	else                   log_errf ("[spatial_test] %d failure(s)", spt_failures);
	return spt_failures;
}
