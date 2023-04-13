#include "anchor.h"
#include "../libraries/array.h"
#include "../libraries/stref.h"
#include "../utils/random.h"

namespace sk {

typedef struct anchor_listener_t {
	void (*on_anchor_discovered)(void* context, anchor_t anchor);
	void* context;
} anchor_listener_t;

///////////////////////////////////////////

array_t<anchor_system_t>   anch_systems      = {};
array_t<anchor_listener_t> anch_listeners    = {};
array_t<anchor_t>          anch_list         = {};
array_t<anchor_t>          anch_changed      = {};

///////////////////////////////////////////

char to_hex(uint32_t val, uint32_t nibble) {
	uint32_t byte = (val >> (nibble * 4)) & 0xF;
	return byte < 10
		? '0' + byte
		: 'A' + byte - 10;
}

///////////////////////////////////////////

anchor_t anchor_create(pose_t pose, const char* name_utf8, anchor_props_ properties) {
	int32_t system = -1;
	for (int32_t i = 0; i < anch_systems.count; i++) {
		if ((anch_systems[i].properties & properties) == properties) {
			system = i;
			break;
		}
	}
	char gen_name[20];
	if (name_utf8 == nullptr) {
		uint32_t a = rand_x();
		uint32_t b = rand_x();
		char name[20]{
			to_hex(a,0), to_hex(a,1), to_hex(a,2), to_hex(a,3), '-',
			to_hex(a,4), to_hex(a,5), to_hex(a,6), to_hex(a,7), '-',
			to_hex(b,0), to_hex(b,1), to_hex(b,2), to_hex(b,3), '-',
			to_hex(b,4), to_hex(b,5), to_hex(b,6), to_hex(b,7), '\0' };
		// Some juggling since C/C++ doesn't allow us to assign an array to an
		// existing array.
		strncpy(gen_name, name, sizeof(gen_name));
		name_utf8 = gen_name;
	}
	return system == -1
		? nullptr
		: anch_systems[system].on_create(anch_systems[system].context, pose, name_utf8);
}

///////////////////////////////////////////

anchor_t anchor_create_manual(anchor_type_id system_id, pose_t pose, const char *name_utf8, void* data) {
	anchor_t result = (anchor_t)assets_allocate(asset_type_anchor);
	result->source_system = system_id;
	result->pose          = pose;
	result->data          = data;
	result->name          = string_copy(name_utf8);
	anch_list.add(result);

	anchor_mark_dirty(result);

	return result;
}

///////////////////////////////////////////

void anchor_notify_discovery(anchor_t anchor) {
	for (int32_t i = 0; i < anch_listeners.count; i++) {
		anch_listeners[i].on_anchor_discovered(anch_listeners[i].context, anchor);
	}
}

///////////////////////////////////////////

void anchor_destroy(anchor_t anchor) {
	int32_t idx = anch_list.index_of(anchor);
	if (idx != -1) anch_list.remove(idx);

	anchor_system_t* sys = &anch_systems[anchor->source_system];
	if (sys->on_destroy_anchor) {
		sys->on_destroy_anchor(sys->context, anchor, anchor->data);
		if (anchor->header.refs > 0) {
			log_err("Added a reference to an anchor during destruction!");
		}
	}
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

bool32_t anchor_set_persistent(anchor_t anchor, bool32_t persistent) {
	return anch_systems[anchor->source_system].on_persist(anch_systems[anchor->source_system].context, anchor, persistent);
}

///////////////////////////////////////////

bool32_t anchor_get_persistent(const anchor_t anchor) {
	return anchor->persisted;
}

///////////////////////////////////////////

anchor_props_ anchor_get_properties(const anchor_t anchor) {
	return anch_systems[anchor->source_system].properties;
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

void anchor_mark_dirty(anchor_t anchor) {
	if (anchor->changed == false) {
		anchor->changed = true;
		anchor_addref(anchor);
		anch_changed.add(anchor);
	}
}

///////////////////////////////////////////

void anchor_clear_all_dirty() {
	for (int32_t i = 0; i < anch_changed.count; i++) {
		anch_changed[i]->changed = false;
		anchor_release(anch_changed[i]);
	}
	anch_changed.clear();
}

///////////////////////////////////////////

anchor_type_id anchors_register_type(anchor_system_t system) {
	anch_systems.add(system);
	return anch_systems.count - 1;
}

///////////////////////////////////////////

void anchors_clear_stored() {
	for (int32_t i = 0; i < anch_systems.count; i++) {
		if (anch_systems[i].on_clear_stored)
			anch_systems[i].on_clear_stored(anch_systems[i].context);
	}
}

///////////////////////////////////////////

void anchors_subscribe(void (*on_anchor_discovered)(void* context, anchor_t anchor), void* context, bool32_t only_new) {
	if (on_anchor_discovered == nullptr) {
		log_err("anchors_subscribe received a null callback.");
		return;
	}

	anch_listeners.add({ on_anchor_discovered, context });

	if (only_new == (bool32_t)true) return;

	for (int32_t i = 0; i < anch_list.count; i++) {
		on_anchor_discovered(context, anch_list[i]);
	}
}

///////////////////////////////////////////

void anchors_unsubscribe(void (*on_anchor_discovered)(void* context, anchor_t anchor), void* context) {
	for (int32_t i = 0; i < anch_listeners.count; i++) {
		if (anch_listeners[i].on_anchor_discovered == on_anchor_discovered && anch_listeners[i].context == context) {
			anch_listeners.remove(i);
			return;
		}
	}
}

}