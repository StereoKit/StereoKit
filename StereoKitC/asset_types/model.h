#pragma once

#include "../stereokit.h"
#include "assets.h"

namespace sk {

struct model_subset_t {
	mesh_t      mesh;
	material_t  material;
	matrix      offset;
};

struct _model_t {
	asset_header_t  header;
	model_subset_t *subsets;
	int             subset_count;
	bounds_t        bounds;
};

bool modelfmt_fbx (model_t model, const char *filename, void *file_data, size_t file_size, shader_t shader);
bool modelfmt_obj (model_t model, const char *filename, void *file_data, size_t file_size, shader_t shader);
bool modelfmt_gltf(model_t model, const char *filename, void *file_data, size_t file_size, shader_t shader);
bool modelfmt_stl (model_t model, const char *filename, void *file_data, size_t file_size, shader_t shader);
bool modelfmt_ply (model_t model, const char *filename, void *file_data, size_t file_size, shader_t shader);
void model_destroy(model_t model);

} // namespace sk