// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith

#include "spatial_tests.h"

#include <stereokit.h>

// Internal header, reached through StereoKitC's public include root. The
// fake backend below stands in for OpenXR through the provider interface.
#include <systems/spatial_entity.h>
#include <systems/spatial_names.h>

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

// What the backend's next update snapshot says about an app-created anchor
static void spt_report(spatial_entity_t entity, spatial_tracking_ tracking) {
	spatial_ingest_t in = {};
	in.entity   = entity;
	in.tracking = tracking;
	spatial_backend_ingest(spatial_capability_anchor, &in, 1);
}

// Unnamed, so these tests cover persistence without touching the name store
static spatial_entity_t spt_create_persisted() {
	spatial_entity_t e = spatial_entity_create_anchor(pose_identity, nullptr, 0);
	spt_report(e, spatial_tracking_tracking);
	spatial_entity_persist(e);
	return e;
}

///////////////////////////////////////////

static void spt_test_create_persisted() {
	spt_reset();
	spatial_entity_t e = spatial_entity_create_anchor(pose_identity, nullptr, 0);
	spatial_entity_persist(e);
	SPT_CHECK(e != 0 && spatial_entity_get_status(e) == spatial_status_pending, "persisting a new anchor starts Pending");
	SPT_CHECK(spt.persist_count == 0,                                           "a new anchor waits for the runtime to report it before persisting");
	SPT_CHECK(spatial_entity_get_tracked(e) == button_state_inactive,           "a new anchor is untracked until the runtime reports it");
	SPT_CHECK(!spt_in_new(e),                                                   "app-created entities skip New the frame they're made");

	spt_report(e, spatial_tracking_paused);
	SPT_CHECK(spt.persist_count == 0,                         "a paused anchor keeps waiting to persist");
	spt_report(e, spatial_tracking_tracking);
	SPT_CHECK(spatial_entity_get_tracked(e) == (button_state_active | button_state_just_active), "the runtime's first tracking report starts tracking");
	SPT_CHECK(spt.persist_count == 1 && spt.persists[0] == e, "an anchor persists once the runtime reports it tracking");

	sk_step(nullptr);
	SPT_CHECK(spt_in_new(e), "app-created entities show up in New the following frame");

	spatial_backend_set_persist(e, spt_uuid(1));
	SPT_CHECK(spatial_entity_get_status(e) == spatial_status_ready && spt_uuid_eq(spt_persist_id(e), spt_uuid(1)), "a landed persist is Ready with its id");

	sk_step(nullptr);
	SPT_CHECK(!spt_in_new(e), "entities are in New for exactly one frame");
}

static void spt_test_undo() {
	spt_reset();
	spatial_entity_t e = spt_create_persisted();
	spatial_entity_unpersist(e);
	SPT_CHECK(spatial_entity_get_status(e) == spatial_status_pending, "unpersisting mid-persist stays Pending");

	spatial_backend_set_persist(e, spt_uuid(2));
	SPT_CHECK(spt.unpersist_count == 1 && spt_uuid_eq(spt.unpersists[0].id, spt_uuid(2)), "a persist the app no longer wants is undone once it lands");

	spatial_backend_clear_persist(e);
	SPT_CHECK(spatial_entity_get_status(e) == spatial_status_ready && (spatial_entity_get_components(e) & spatial_component_persistence) == 0, "the undo ends unpersisted and Ready");
}

static void spt_test_redo() {
	spt_reset();
	spatial_entity_t e = spt_create_persisted();
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
	spatial_entity_t e = spt_create_persisted();
	spatial_backend_persist_failed(e);
	SPT_CHECK(spatial_entity_get_status(e) == spatial_status_partial, "a failed persist reports Partial");

	spatial_entity_persist(e);
	SPT_CHECK(spatial_entity_get_status(e) == spatial_status_pending && spt.persist_count == 2, "a new request clears Partial and retries");

	spatial_backend_set_persist(e, spt_uuid(5));
	SPT_CHECK(spatial_entity_get_status(e) == spatial_status_ready, "the retry ends Ready");
}

static void spt_test_destroy_mid_persist() {
	spt_reset();
	spatial_entity_t e = spt_create_persisted();
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
	spatial_entity_t e = spatial_entity_find_anchor_uuid(spt_uuid(7));
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
	spatial_entity_t e = spatial_entity_find_anchor_uuid(spt_uuid(8));
	SPT_CHECK(spatial_entity_destroy(e) && spt.unpersist_count == 1 && spt_in_removed(e), "destroying a lookup removes it from storage and the list");

	sk_step(nullptr);
	spatial_backend_clear_persist(e);
	SPT_CHECK(!spatial_entity_is_valid(e), "a destroyed lookup stays gone once its unpersist lands");
}

static void spt_test_lookup_binds() {
	spt_reset();
	spatial_entity_t e = spatial_entity_find_anchor_uuid(spt_uuid(9));

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

static bool spt_pose_eq(pose_t a, pose_t b) { return memcmp(&a, &b, sizeof(pose_t)) == 0; }

static void spt_test_pose() {
	spt_reset();
	SPT_CHECK(spt_pose_eq(spatial_entity_get_pose(spatial_entity_find_anchor_uuid(spt_uuid(20))), pose_identity), "an unloaded lookup has an identity pose");

	pose_t           at     = { {1, 2, 3}, quat_identity };
	spatial_entity_t anchor = spatial_entity_create_anchor(at, nullptr, 0);
	SPT_CHECK(spt_pose_eq(spatial_entity_get_pose(anchor), at), "an anchor's pose is its anchor");

	spatial_ingest_t plane = {};
	plane.id              = 4000;
	plane.tracking        = spatial_tracking_tracking;
	plane.present         = spatial_component_bounds2d;
	plane.bounds2d_center = { {4, 5, 6}, quat_identity };
	spatial_backend_ingest(spatial_capability_plane_tracking, &plane, 1);
	SPT_CHECK(spt_pose_eq(spatial_entity_get_pose(plane.entity), plane.bounds2d_center), "a plane's pose is its bounds center");

	spatial_ingest_t both = plane;
	both.entity      = 0;
	both.id          = 4001;
	both.present     = spatial_component_bounds2d | spatial_component_anchor;
	both.anchor_pose = { {7, 8, 9}, quat_identity };
	spatial_backend_ingest(spatial_capability_plane_tracking, &both, 1);
	SPT_CHECK(spt_pose_eq(spatial_entity_get_pose(both.entity), both.anchor_pose), "an anchor wins over bounds");

	spatial_ingest_t paused = plane;
	paused.entity   = 0;
	paused.id       = 4002;
	paused.tracking = spatial_tracking_paused;
	spatial_backend_ingest(spatial_capability_plane_tracking, &paused, 1);
	SPT_CHECK(spt_pose_eq(spatial_entity_get_pose(paused.entity), pose_identity), "something first seen while paused has no pose yet");
}

static bool spt_named(const char* name, sk_uuid_t uuid) {
	sk_uuid_t stored;
	return spatial_names_find(name, &stored) && spt_uuid_eq(stored, uuid);
}

static void spt_test_names() {
	spt_reset();
	spatial_names_clear();

	spatial_entity_t a = spatial_entity_create_anchor(pose_identity, "spt_table", 0);
	spt_report(a, spatial_tracking_tracking);
	SPT_CHECK(spt.persist_count == 1,                          "a named anchor persists");
	SPT_CHECK(spatial_entity_find_anchor("spt_table") == a,    "a name finds its anchor before the persist lands");
	spatial_backend_set_persist(a, spt_uuid(30));
	SPT_CHECK(spt_named("spt_table", spt_uuid(30)),            "the name is saved once the persist lands");

	spatial_entity_t b = spatial_entity_create_anchor(pose_identity, "spt_table", 0);
	SPT_CHECK(spt_in_removed(a) && spt.unpersist_count == 1 && spt_uuid_eq(spt.unpersists[0].id, spt_uuid(30)), "a new anchor takes the name, and the old one is destroyed");
	SPT_CHECK(spatial_entity_find_anchor("spt_table") == b,    "the name finds the new anchor");
	spt_report(b, spatial_tracking_tracking);
	spatial_backend_set_persist(b, spt_uuid(31));
	SPT_CHECK(spt_named("spt_table", spt_uuid(31)),            "the name moves to the new anchor's uuid");

	spatial_names_set(spt_uuid(32), "spt_saved");
	spatial_names_shutdown();
	SPT_CHECK(spt_named("spt_saved", spt_uuid(32)) && spt_named("spt_table", spt_uuid(31)), "names survive a reload from disk");

	spatial_entity_t saved = spatial_entity_find_anchor("spt_saved");
	SPT_CHECK(saved != 0 && spatial_entity_get_status(saved) == spatial_status_pending && spt_finding(spt_uuid(32)), "a stored name looks its anchor up by uuid");

	spatial_names_set(spt_uuid(33), "spt_gone");
	spatial_entity_t gone = spatial_entity_find_anchor("spt_gone");
	spatial_backend_persist_not_found(spt_uuid(33));
	SPT_CHECK(spatial_entity_get_status(gone) == spatial_status_failed && !spt_named("spt_gone", spt_uuid(33)), "storage lacking the uuid drops the name");

	spatial_names_set(spt_uuid(34), "spt_found");
	spatial_ingest_t found = {};
	found.id         = 5000;
	found.tracking   = spatial_tracking_tracking;
	found.present    = spatial_component_anchor | spatial_component_persistence;
	found.persist_id = spt_uuid(34);
	spatial_backend_ingest(spatial_capability_anchor, &found, 1);
	const char* found_name = spatial_entity_get_name(found.entity);
	SPT_CHECK(found_name != nullptr && strcmp(found_name, "spt_found") == 0, "a stored anchor gets its name back when discovered");
	const char* b_name = spatial_entity_get_name(b);
	SPT_CHECK(b_name != nullptr && strcmp(b_name, "spt_table") == 0,         "a named anchor reports its name");

	spatial_entity_unpersist(b);
	SPT_CHECK(!spt_named("spt_table", spt_uuid(31)),           "unpersisting releases the name");
	SPT_CHECK(spatial_entity_find_anchor("spt_nope") == 0,     "an unknown name finds nothing");

	spatial_names_clear();
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
	spatial_backend_set_running      (spatial_capability_anchor, true);
	spatial_request                  (spatial_capability_anchor);

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
	spt_test_pose               ();
	spt_test_names              ();

	sk_shutdown();

	if (spt_failures == 0) log_info ("[spatial_test] all tests passed!");
	else                   log_errf ("[spatial_test] %d failure(s)", spt_failures);
	return spt_failures;
}
