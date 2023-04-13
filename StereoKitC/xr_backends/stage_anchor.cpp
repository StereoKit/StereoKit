#include "stage_anchor.h"
#include "../asset_types/anchor.h"
#include "../libraries/stref.h"

#include <stdio.h>

namespace sk {

typedef struct stage_anchor_sys_t {
	anchor_type_id    id;
	array_t<anchor_t> persistant;
} stage_anchor_sys_t;

typedef struct stage_anchor_t {
	pose_t relative_pose;
} stage_anchor_t;

stage_anchor_sys_t stage_anchor_sys = {};

const char* stage_anchor_store_filename = "anchors.txt";

///////////////////////////////////////////

anchor_t stage_anchor_on_create      (stage_anchor_sys_t* context, pose_t pose, const char* name_utf8);
void     stage_anchor_on_clear_stored(stage_anchor_sys_t* context);
bool32_t stage_anchor_on_persist     (stage_anchor_sys_t* context, anchor_t anchor, bool32_t persist);

///////////////////////////////////////////

void stage_anchor_init() {
	stage_anchor_sys = {};

	anchor_system_t anchor_sys = {};
	anchor_sys.name       = "Stage Anchor";
	anchor_sys.properties = anchor_props_storable;
	anchor_sys.context    = &stage_anchor_sys;
	anchor_sys.on_create       = (anchor_t(*)(void*, pose_t, const char*)) stage_anchor_on_create;
	anchor_sys.on_clear_stored = (void    (*)(void*))                      stage_anchor_on_clear_stored;
	anchor_sys.on_persist      = (bool32_t(*)(void*, anchor_t, bool32_t))  stage_anchor_on_persist;

	stage_anchor_sys.id = anchors_register_type(anchor_sys);

	// Read anchors from a text file
	char*  anchor_data      = nullptr;
	size_t anchor_data_size = 0;
	if (platform_read_file(stage_anchor_store_filename, (void**)&anchor_data, &anchor_data_size)) {
		stref_t data_stref = stref_make(anchor_data);
		stref_t line = {};
		while (stref_nextline(data_stref, line)) {
			stref_t word = {};

			vec3 pos = {};
			if (!stref_nextword(line, word, ' ')) break;
			pos.x = stref_to_f(word);
			if (!stref_nextword(line, word, ' ')) break;
			pos.y = stref_to_f(word);
			if (!stref_nextword(line, word, ' ')) break;
			pos.z = stref_to_f(word);

			quat rot = {};
			if (!stref_nextword(line, word, ' ')) break;
			rot.x = stref_to_f(word);
			if (!stref_nextword(line, word, ' ')) break;
			rot.y = stref_to_f(word);
			if (!stref_nextword(line, word, ' ')) break;
			rot.z = stref_to_f(word);
			if (!stref_nextword(line, word, ' ')) break;
			rot.w = stref_to_f(word);

			if (!stref_nextword(line, word, ' ')) break;
			char* name = stref_copy(stref_substr(word.start, line.length - (word.start - line.start)));

			// Create a StereoKit anchor
			stage_anchor_t* anchor_data = sk_malloc_t(stage_anchor_t, 1);
			anchor_t        anchor      = anchor_create_manual(stage_anchor_sys.id, { pos, rot }, name, (void*)anchor_data);
			stage_anchor_on_persist(&stage_anchor_sys, anchor, true);
			anchor_notify_discovery(anchor);

			log_diagf("Discovered stage anchor: %s", name);
			sk_free(name);
		}
		sk_free(anchor_data);
	}
}

///////////////////////////////////////////

void stage_anchor_shutdown() {
	// Write our persistent anchors to file
	char* file_data = string_copy("");
	char  line[512];
	for (int32_t i = 0; i < stage_anchor_sys.persistant.count; i++) {
		anchor_t a = stage_anchor_sys.persistant[i];
		snprintf(line, sizeof(line), "%.3g %.3g %.3g %.3g %.3g %.3g %.3g %s\n",
			a->pose.position.x, a->pose.position.y, a->pose.position.z, 
			a->pose.orientation.x, a->pose.orientation.y, a->pose.orientation.z, a->pose.orientation.w,
			a->name);
		file_data = string_append(file_data, 1, line);
	}
	platform_write_file_text(stage_anchor_store_filename, file_data);
	sk_free(file_data);

	// Release the persisted anchors and free the array
	for (int32_t i = 0; i < stage_anchor_sys.persistant.count; i++) {
		anchor_release(stage_anchor_sys.persistant[i]);
	}
	stage_anchor_sys.persistant.free();
}

///////////////////////////////////////////

void stage_anchor_on_clear_stored(stage_anchor_sys_t* context) {
	// Release the persisted anchors and free the array
	for (int32_t i = 0; i < stage_anchor_sys.persistant.count; i++) {
		stage_anchor_sys.persistant[i]->persisted = false;
		anchor_release(stage_anchor_sys.persistant[i]);
	}
	stage_anchor_sys.persistant.free();
	platform_write_file_text(stage_anchor_store_filename, "");
}

///////////////////////////////////////////

anchor_t stage_anchor_on_create(stage_anchor_sys_t* context, pose_t pose, const char* name_utf8) {
	pose_t relative_pose = pose;
	if (world_has_bounds()) {
		matrix to_relative = matrix_invert( pose_matrix(world_get_bounds_pose()) );
		relative_pose = matrix_transform_pose(to_relative, pose);
	}

	// Create a StereoKit anchor
	stage_anchor_t* anchor_data = sk_malloc_t(stage_anchor_t, 1);
	anchor_data->relative_pose = pose;
	anchor_t result = anchor_create_manual(context->id, pose, name_utf8, (void*)anchor_data);
	anchor_notify_discovery(result);
	return result;
}

///////////////////////////////////////////

bool32_t stage_anchor_on_persist(stage_anchor_sys_t* context, anchor_t anchor, bool32_t persist) {
	if (anchor->persisted == persist) return true;
	anchor->persisted = persist;

	array_t<anchor_t>* persist_list = &context->persistant;
	if (persist) {
		anchor_addref(anchor);
		persist_list->add(anchor);
	} else {
		persist_list->remove(persist_list->index_of(anchor));
		anchor_release(anchor);
	}

	return true;
}

} // namespace sk