#include "anchor_stage.h"
#include "../asset_types/anchor.h"
#include "../libraries/stref.h"

#include <stdio.h>

namespace sk {

typedef struct anchor_stage_sys_t {
	anchor_type_id    id;
	array_t<anchor_t> persistant;
	bool32_t          loaded;
} anchor_stage_sys_t;

typedef struct anchor_stage_t {
	pose_t relative_pose;
} anchor_stage_t;

anchor_stage_sys_t anchor_stage_sys = {};

const char* anchor_stage_store_filename = "anchors.txt";

///////////////////////////////////////////

bool32_t anchor_stage_init() {
	anchor_stage_sys = {};

	// Read anchors from a text file
	char*  anchor_data      = nullptr;
	size_t anchor_data_size = 0;
	if (platform_read_file(anchor_stage_store_filename, (void**)&anchor_data, &anchor_data_size)) {
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
			anchor_stage_t* anchor_data = sk_malloc_t(anchor_stage_t, 1);
			anchor_t        anchor      = anchor_create_manual(anchor_stage_sys.id, { pos, rot }, name, (void*)anchor_data);
			anchor_stage_persist(anchor, true);
			anchor_notify_discovery(anchor);

			sk_free(name);
		}
		sk_free(anchor_data);
	}
	anchor_stage_sys.loaded = true;
	return true;
}

///////////////////////////////////////////

void anchor_stage_shutdown() {
	if (!anchor_stage_sys.loaded) return;

	// Write our persistent anchors to file
	char* file_data = string_copy("");
	char  line[512];
	for (int32_t i = 0; i < anchor_stage_sys.persistant.count; i++) {
		anchor_t a = anchor_stage_sys.persistant[i];
		snprintf(line, sizeof(line), "%.3g %.3g %.3g %.3g %.3g %.3g %.3g %s\n",
			a->pose.position.x, a->pose.position.y, a->pose.position.z, 
			a->pose.orientation.x, a->pose.orientation.y, a->pose.orientation.z, a->pose.orientation.w,
			a->name);
		file_data = string_append(file_data, 1, line);
	}
	platform_write_file_text(anchor_stage_store_filename, file_data);
	sk_free(file_data);

	// Release the persisted anchors and free the array
	for (int32_t i = 0; i < anchor_stage_sys.persistant.count; i++) {
		anchor_release(anchor_stage_sys.persistant[i]);
	}
	anchor_stage_sys.persistant.free();
}

///////////////////////////////////////////

void anchor_stage_clear_stored() {
	// Release the persisted anchors and free the array
	for (int32_t i = 0; i < anchor_stage_sys.persistant.count; i++) {
		anchor_stage_sys.persistant[i]->persisted = false;
		anchor_release(anchor_stage_sys.persistant[i]);
	}
	anchor_stage_sys.persistant.free();
	platform_write_file_text(anchor_stage_store_filename, "");
}

///////////////////////////////////////////

anchor_t anchor_stage_create(pose_t pose, const char* name_utf8) {
	pose_t relative_pose = pose;
	if (world_has_bounds()) {
		matrix to_relative = matrix_invert( pose_matrix(world_get_bounds_pose()) );
		relative_pose = matrix_transform_pose(to_relative, pose);
	}

	// Create a StereoKit anchor
	anchor_stage_t* anchor_data = sk_malloc_t(anchor_stage_t, 1);
	anchor_data->relative_pose = pose;
	anchor_t result = anchor_create_manual(anchor_stage_sys.id, pose, name_utf8, (void*)anchor_data);
	anchor_notify_discovery(result);
	return result;
}

///////////////////////////////////////////

bool32_t anchor_stage_persist(anchor_t anchor, bool32_t persist) {
	if (anchor->persisted == persist) return true;
	anchor->persisted = persist;

	array_t<anchor_t>* persist_list = &anchor_stage_sys.persistant;
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