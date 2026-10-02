#include "../stereokit.h"
#include "../sk_memory.h"
#include "../libraries/stref.h"
#include "../systems/vert_format.h"
#include "../platforms/platform.h"
#include "mesh.h"
#include "mesh_file.h"
#include "mesh_svg.h"
#include "model.h"

#include <stdio.h>
#include <string.h>
#include <stddef.h>

namespace sk {

typedef enum mesh_format_ {
	mesh_format_none,
	mesh_format_svg,
	mesh_format_ply,
	mesh_format_stl,
} mesh_format_;

typedef struct mesh_file_load_t {
	mesh_load_t   mesh; // first, so the shared mesh load actions can cast to it
	mesh_format_  format;
	svg_options_t svg_options;
	char*         filename;
	void*         file_data;
	size_t        file_size;
} mesh_file_load_t;
static_assert(offsetof(mesh_file_load_t, mesh) == 0, "mesh_load_* actions cast load data to mesh_load_t");

///////////////////////////////////////////

// Only formats that hold exactly one mesh and no materials belong here,
// anything richer needs to come in as a Model.
static mesh_format_ mesh_file_format(const char* filename) {
	if (string_endswith(filename, ".svg", false)) return mesh_format_svg;
	if (string_endswith(filename, ".ply", false)) return mesh_format_ply;
	if (string_endswith(filename, ".stl", false)) return mesh_format_stl;
	return mesh_format_none;
}

///////////////////////////////////////////

static bool32_t mesh_file_read(asset_task_t*, asset_header_t* asset, void* data) {
	mesh_file_load_t* load = (mesh_file_load_t*)data;
	if (!platform_read_file(load->filename, &load->file_data, &load->file_size)) {
		log_warnf("Mesh file failed to load: %s", load->filename);
		asset->state = asset_state_error_not_found;
		return false;
	}
	return true;
}

///////////////////////////////////////////

static bool32_t mesh_file_build(asset_task_t*, asset_header_t* asset, void* data) {
	mesh_file_load_t* load = (mesh_file_load_t*)data;

	bool built = false;
	switch (load->format) {
	case mesh_format_svg: built = svg_build_mem     (load->file_data, load->file_size, load->filename, load->svg_options, &load->mesh, nullptr); break;
	case mesh_format_ply: built = modelfmt_ply_build(load->file_data, load->file_size, load->filename, &load->mesh); break;
	case mesh_format_stl: built = modelfmt_stl_build(load->file_data, load->file_size, load->filename, &load->mesh); break;
	default: break;
	}
	sk_free(load->file_data);
	load->file_size = 0;
	if (!built) asset->state = asset_state_error_unsupported;
	return built;
}

///////////////////////////////////////////

static void mesh_file_free(asset_header_t* asset, void* data) {
	mesh_file_load_t* load = (mesh_file_load_t*)data;
	sk_free(load->filename);
	sk_free(load->file_data);
	mesh_load_free(asset, data);
}

///////////////////////////////////////////

// The build steps set a specific error state before failing.
static void mesh_file_on_failure(asset_header_t* asset, void*) {
	if (asset->state >= 0) asset->state = asset_state_error;
}

///////////////////////////////////////////

static void mesh_file_add_task(mesh_t mesh, mesh_file_load_t* load, const asset_load_action_t* actions, int32_t action_count, int32_t priority, size_t complexity_bytes) {
	mesh->header.state     = asset_state_loading;
	load->mesh.calc_bounds = true;
	load->mesh.vert_format = VERT_FORMAT_DEFAULT;
	vert_format_addref(VERT_FORMAT_DEFAULT);

	asset_task_t task = {};
	task.asset        = &mesh->header;
	task.load_data    = load;
	task.actions      = (asset_load_action_t*)actions;
	task.action_count = action_count;
	task.free_data    = mesh_file_free;
	task.on_failure   = mesh_file_on_failure;
	task.priority     = priority;
	task.sort         = asset_sort(priority, asset_complexity_bytes(complexity_bytes));
	assets_add_task(task);
}

///////////////////////////////////////////

mesh_t mesh_create_mem_ex(const char* filename_utf8, const void* data, size_t data_size, int32_t priority, svg_options_t svg_options) {
	mesh_format_ format = mesh_file_format(filename_utf8);
	if (format == mesh_format_none) {
		log_errf("Issue loading %s! Unrecognized file extension.", filename_utf8);
		return nullptr;
	}

	mesh_file_load_t* load = sk_malloc_zero_t(mesh_file_load_t, 1);
	load->format      = format;
	load->svg_options = svg_options;
	load->filename    = string_copy(filename_utf8);
	load->file_size   = data_size;
	load->file_data   = sk_malloc(data_size);
	memcpy(load->file_data, data, data_size);

	static const asset_load_action_t actions[] = {
		mesh_file_build,
		mesh_load_process,
		mesh_load_upload,
	};
	mesh_t result = mesh_create();
	mesh_file_add_task(result, load, actions, _countof(actions), priority, data_size);
	return result;
}

///////////////////////////////////////////

mesh_t mesh_create_file_ex(const char* filename_utf8, int32_t priority, svg_options_t svg_options) {
	mesh_format_ format = mesh_file_format(filename_utf8);
	if (format == mesh_format_none) {
		log_errf("Issue loading %s! Unrecognized file extension.", filename_utf8);
		return nullptr;
	}

	// Same id the Model loaders give their mesh, so both paths share it.
	char id[512];
	snprintf(id, sizeof(id), "%s/mesh", filename_utf8);
	mesh_t result = mesh_find(id);
	if (result != nullptr) return result;

	mesh_file_load_t* load = sk_malloc_zero_t(mesh_file_load_t, 1);
	load->format      = format;
	load->svg_options = svg_options;
	load->filename    = string_copy(filename_utf8);

	static const asset_load_action_t actions[] = {
		mesh_file_read,
		mesh_file_build,
		mesh_load_process,
		mesh_load_upload,
	};
	result = mesh_create();
	mesh_set_id(result, id);
	mesh_file_add_task(result, load, actions, _countof(actions), priority, platform_file_size(filename_utf8));
	return result;
}

///////////////////////////////////////////

mesh_t mesh_create_file(const char* filename_utf8, int32_t priority) {
	return mesh_create_file_ex(filename_utf8, priority, svg_options_default);
}

///////////////////////////////////////////

mesh_t mesh_create_mem(const char* filename_utf8, const void* data, size_t data_size, int32_t priority) {
	return mesh_create_mem_ex(filename_utf8, data, data_size, priority, svg_options_default);
}

}
