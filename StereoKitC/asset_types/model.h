#pragma once

#include "../stereokit.h"
#include "assets.h"

struct model_subset_t {
	mesh_t      mesh;
	material_t  material;
	transform_t offset;
};

struct _model_t {
	asset_header_t  header;
	model_subset_t *subsets;
	int             subset_count;
};

void model_destroy(model_t model);