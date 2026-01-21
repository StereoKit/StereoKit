#include "anchor.h"
#include "../libraries/array.h"
#include "../libraries/stref.h"
#include "../utils/random.h"
#include "../platforms/platform.h"

#include "../xr_backends/openxr.h"
#include "../xr_backends/extensions/ext_management.h"
#include "../xr_backends/extensions/msft_anchors.h"
#include "../xr_backends/anchor_stage.h"

namespace sk {

///////////////////////////////////////////

array_t<anchor_t> anch_list         = {};
array_t<anchor_t> anch_changed      = {};
anchor_system_    anch_sys          = anchor_system_none;
bool32_t          anch_initialized  = false;

///////////////////////////////////////////

void anchors_register() {
	xr_system_t system = {};
	system.evt_initialize = { [](void*) { return anchors_init() ? xr_system_succeed : xr_system_fail; } };
	system.evt_shutdown   = { anchors_shutdown   };
	system.evt_step_begin = { anchors_step_begin };
	system.evt_step_end   = { anchors_step_end   };
	ext_management_sys_register(system);
}

///////////////////////////////////////////

bool anchors_init() {
	if (anch_initialized) {
		log_err("Anchor system already initialized!");
		return false;
	}

	if (xr_ext_msft_spatial_anchors_available())
		anch_sys = anchor_system_openxr_msft;
	else if (backend_xr_get_type() == backend_xr_type_simulator)
		anch_sys = anchor_system_stage;
	else
		anch_sys = anchor_system_none;

	bool32_t result = false;
	switch (anch_sys) {
	case anchor_system_stage:       result = anchor_stage_init(); break;
	case anchor_system_openxr_msft: result = true;                break;
	default: break;
	}

	if (!result) anch_sys = anchor_system_none;
	anch_initialized = true;

	switch (anch_sys) {
	case anchor_system_openxr_msft: log_diagf("Using MSFT spatial anchors."); break;
	case anchor_system_stage:       log_diagf("Using fallback stage spatial anchors."); break;
	default:                        log_diagf("NOT using spatial anchors."); break;
	}

	return true;
}

///////////////////////////////////////////

void anchors_shutdown(void*) {
	if (!anch_initialized) return;

	for (int32_t i = anch_list   .count-1; i>=0; i--) anchor_release(anch_list   [i]);
	for (int32_t i = anch_changed.count-1; i>=0; i--) anchor_release(anch_changed[i]);

	switch (anch_sys) {
	case anchor_system_stage: anchor_stage_shutdown(); break;
	default: break;
	}

	anch_list   .free();
	anch_changed.free();

	anch_initialized = false;
}

///////////////////////////////////////////

void anchors_step_begin(void*) {
	switch (anch_sys) {
	case anchor_system_stage: break;
	default: break;
	}
}

///////////////////////////////////////////

void anchors_step_end(void*) {
	for (int32_t i = 0; i < anch_changed.count; i++) {
		//anch_changed[i]->changed = false;
		anchor_release(anch_changed[i]);
	}
	anch_changed.clear();
}

///////////////////////////////////////////

char to_hex(uint32_t val, uint32_t byte_idx) {
	uint32_t byte = (val >> (byte_idx * 4)) & 0xF;
	return byte < 10
		? (char)('0' + byte)
		: (char)('A' + byte - 10);
}

///////////////////////////////////////////

anchor_t anchor_create(pose_t pose) {
	uint32_t a = rand_x();
	uint32_t b = rand_x();
	char name[20]{
		to_hex(a,0), to_hex(a,1), to_hex(a,2), to_hex(a,3), '-',
		to_hex(a,4), to_hex(a,5), to_hex(a,6), to_hex(a,7), '-',
		to_hex(b,0), to_hex(b,1), to_hex(b,2), to_hex(b,3), '-',
		to_hex(b,4), to_hex(b,5), to_hex(b,6), to_hex(b,7), '\0' };

	switch (anch_sys) {
	case anchor_system_openxr_msft: return xr_ext_msft_spatial_anchors_create(pose, name);
	case anchor_system_stage:       return anchor_stage_create               (pose, name);
	default: return nullptr;
	}
}

///////////////////////////////////////////

anchor_t anchor_create_manual(anchor_type_id system_id, pose_t pose, const char *name_utf8, void* data) {
	anchor_t result = (anchor_t)assets_allocate(asset_type_anchor);
	result->source_system = system_id;
	result->pose          = pose;
	result->data          = data;
	result->name          = string_copy(name_utf8);
	anch_list.add(result);
	anchor_addref(result);

	anchor_mark_dirty(result);

	return result;
}

///////////////////////////////////////////

void anchor_destroy(anchor_t anchor) {
	switch (anch_sys) {
	case anchor_system_openxr_msft: xr_ext_msft_spatial_anchors_destroy(anchor); break;
	//case anchor_system_stage:       anchor_stage_destroy               (anchor); break;
	default: break;
	}

	int32_t idx = anch_list.index_of(anchor);
	if (idx >= 0) anch_list.remove(idx);
	idx = anch_changed.index_of(anchor);
	if (idx >= 0) anch_changed.remove(idx);

	sk_free(anchor->name);
	*anchor = {};
}

///////////////////////////////////////////

anchor_t anchor_find(const char* id) {
	anchor_t result = (anchor_t)assets_find(id, asset_type_anchor);
	if (result != nullptr) {
		anchor_addref(result);
		return result;
	}
	return result;
}

///////////////////////////////////////////

void anchor_set_id(anchor_t anchor, const char* id) {
	assets_set_id(&anchor->header, id);
}

///////////////////////////////////////////

const char* anchor_get_id(const anchor_t anchor) {
	return anchor->header.id_text;
}

///////////////////////////////////////////

void anchor_addref(anchor_t anchor) {
	assets_addref(&anchor->header);
}

///////////////////////////////////////////

void anchor_release(anchor_t anchor) {
	if (anchor == nullptr)
		return;
	assets_releaseref(&anchor->header);
}

///////////////////////////////////////////

void anchor_update_manual(anchor_t anchor, pose_t pose) {
	if (memcmp(&anchor->pose, &pose, sizeof(pose_t)) != 0) {
		anchor->pose    = pose;
		anchor->changed = true;
	}
}

///////////////////////////////////////////

bool32_t anchor_try_set_persistent(anchor_t anchor, bool32_t persistent) {
	switch (anch_sys) {
	case anchor_system_openxr_msft: return xr_ext_msft_spatial_anchors_persist(anchor, persistent);
	case anchor_system_stage:       return anchor_stage_persist               (anchor, persistent);
	default: return false;
	}
}

///////////////////////////////////////////

bool32_t anchor_get_persistent(const anchor_t anchor) {
	return anchor->persisted;
}

///////////////////////////////////////////

pose_t anchor_get_pose(const anchor_t anchor) {
	return anchor->pose;
}

///////////////////////////////////////////

bool32_t anchor_get_changed(const anchor_t anchor) {
	return anchor->changed;
}

///////////////////////////////////////////

const char *anchor_get_name(const anchor_t anchor) {
	return anchor->name;
}

///////////////////////////////////////////

button_state_ anchor_get_tracked(const anchor_t anchor) {
	return anchor->tracked;
}

///////////////////////////////////////////

bool32_t anchor_get_perception_anchor(const anchor_t anchor, void** perception_spatial_anchor) {
	return false;
}

///////////////////////////////////////////

void anchor_mark_dirty(anchor_t anchor) {
	if (anchor->changed == false) {
		anchor->changed = true;
		anchor_addref(anchor);
		anch_changed.add(anchor);
	}
}

///////////////////////////////////////////

void anchor_clear_stored() {
	switch (anch_sys) {
	case anchor_system_openxr_msft: xr_ext_msft_spatial_anchors_clear_stored(); break;
	case anchor_system_stage:       anchor_stage_clear_stored(); break;
	default: break;
	}
}

///////////////////////////////////////////

anchor_caps_ anchor_get_capabilities() {
	switch (anch_sys) {
	case anchor_system_openxr_msft: return xr_ext_msft_spatial_anchors_capabilities();
	case anchor_system_stage:       return anchor_caps_storable;
	default: return (anchor_caps_)0;
	}
}

///////////////////////////////////////////

int32_t anchor_get_count() {
	return anch_list.count;
}

///////////////////////////////////////////

anchor_t anchor_get_index(int32_t index) {
	if (index < 0 || index >= anch_list.count)
		return nullptr;

	anchor_addref(anch_list[index]);
	return anch_list[index];
}

///////////////////////////////////////////

int32_t anchor_get_new_count() {
	return anch_changed.count;
}

///////////////////////////////////////////

anchor_t anchor_get_new_index(int32_t index) {
	if (index < 0 || index >= anch_changed.count)
		return nullptr;

	anchor_addref(anch_changed[index]);
	return anch_changed[index];
}

}