#pragma once

#include "../stereokit.h"
#include "../libraries/array.h"
#include "assets.h"
#include "animation.h"

namespace sk {

struct model_visual_t {
	model_node_id node;
	mesh_t        mesh;
	material_t    material;
	matrix        transform_model;
	bool32_t      visible;
};

struct model_node_t {
	char    *name;
	matrix   transform_local;
	matrix   transform_model;
	int32_t  visual;
	int32_t  parent;
	int32_t  child;
	int32_t  sibling;
	bool32_t solid;
};

struct _model_t {
	asset_header_t          header;
	array_t<model_visual_t> visuals;
	array_t<model_node_t>   nodes;
	int32_t                 nodes_used;
	bool32_t                transforms_changed;
	anim_data_t             anim_data;
	anim_inst_t             anim_inst;
	bounds_t                bounds;
};

bool modelfmt_obj (model_t model, const char *filename, void *file_data, size_t file_size, shader_t shader);
bool modelfmt_gltf(model_t model, const char *filename, void *file_data, size_t file_size, shader_t shader);
bool modelfmt_stl (model_t model, const char *filename, void *file_data, size_t file_size, shader_t shader);
bool modelfmt_ply (model_t model, const char *filename, void *file_data, size_t file_size, shader_t shader);
void model_destroy(model_t model);

} // namespace sk
