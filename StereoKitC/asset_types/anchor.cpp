#include "anchor.h"
#include "../libraries/array.h"

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
anchor_type_id             anch_default_type = -1;

///////////////////////////////////////////

anchor_t anchor_create(pose_t pose) {
	return anchor_create_manual(anch_default_type, pose, nullptr);
}

///////////////////////////////////////////

anchor_t anchor_create_type(anchor_type_id type, pose_t pose) {
	return anchor_create_manual(type, pose, nullptr);
}

///////////////////////////////////////////

anchor_t anchor_create_manual(anchor_type_id system_id, pose_t pose, void* data) {
	anchor_t result = (anchor_t)assets_allocate(asset_type_anchor);
	result->source_system = system_id;
	result->pose          = pose;
	result->data          = data;
	anch_list.add(result);

	anchor_mark_dirty(result);

	for (int32_t i = 0; i < anch_listeners.count; i++) {
		anch_listeners[i].on_anchor_discovered(anch_listeners[i].context, result);
	}

	return result;
}

///////////////////////////////////////////

void anchor_destroy(anchor_t anchor) {
	anchor_system_t* sys = &anch_systems[anchor->source_system];
	if (sys->on_destroy_anchor) {
		sys->on_destroy_anchor(sys->context, anchor, anchor->data);
		if (anchor->header.refs > 0) {
			log_err("Added a reference to an anchor during destruction!");
		}
	}

	int32_t idx = anch_list.index_of(anchor);
	if (idx != -1) anch_list.remove(idx);

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

anchor_type_ anchor_get_type(const anchor_t anchor) {
	return anch_systems[anchor->source_system].info.type;
}

///////////////////////////////////////////

anchor_type_id anchor_get_type_id(const anchor_t anchor) {
	return anchor->source_system;
}

///////////////////////////////////////////

anchor_props_ anchor_get_properties(const anchor_t anchor) {
	return anch_systems[anchor->source_system].info.properties;
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

bounds_t anchor_get_bounds(const anchor_t anchor) {
	return anchor->bounds;
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

int32_t anchors_type_count() {
	return anch_systems.count;
}

///////////////////////////////////////////

anchor_type_t anchors_type_get(anchor_type_id index) {
	return (index >= 0 && index < anch_systems.count)
		? anch_systems[index].info
		: anchor_type_t{};
}

///////////////////////////////////////////

bool32_t anchors_type_enable(anchor_type_id index) {
	return (index >= 0 && index < anch_systems.count && anch_systems[index].on_enable != nullptr)
		? anch_systems[index].on_enable(anch_systems[index].context)
		: false;
}

///////////////////////////////////////////

void anchors_set_default(anchor_type_id id) {
	if (anch_systems[id].info.properties & anchor_props_creatable) {
		anch_default_type = id;
	} else {
		log_errf("Default anchor type must be creatable!");
	}
}

///////////////////////////////////////////

anchor_type_id anchors_get_default(anchor_type_id id) {
	return anch_default_type;
}

///////////////////////////////////////////

void anchors_clear_stored(anchor_type_id id) {
	if (anch_systems[id].info.properties & anchor_props_storable) {
		if (anch_systems[id].on_clear_stored)
			anch_systems[id].on_clear_stored(anch_systems[id].context);
	} else {
		log_errf("Anchor type must be storable!");
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