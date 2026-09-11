#pragma once

#include "../stereokit.h"
#include "../libraries/array.h"
#include "assets.h"
#include "animation.h"
#include "mesh.h"

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
	dictionary_t<char*> info;
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
	bool32_t                bounds_dirty;
};

bool modelfmt_gltf_metadata(model_t model, const char *filename, const void *file_data, size_t file_size, shader_t shader, int32_t priority, void **out_format_data);
bool modelfmt_gltf_meshes  (model_t model, const char *filename, shader_t shader, int32_t priority, void *format_data);
void modelfmt_gltf_free    (void *format_data);

bool modelfmt_obj_metadata (model_t model, const char *filename, const void *file_data, size_t file_size, shader_t shader, int32_t priority, void **out_format_data);
bool modelfmt_obj_meshes   (model_t model, const char *filename, shader_t shader, int32_t priority, void *format_data);
void modelfmt_obj_free     (void *format_data);

bool modelfmt_stl_metadata (model_t model, const char *filename, const void *file_data, size_t file_size, shader_t shader, int32_t priority, void **out_format_data);
bool modelfmt_stl_meshes   (model_t model, const char *filename, shader_t shader, int32_t priority, void *format_data);
void modelfmt_stl_free     (void *format_data);

bool modelfmt_ply_metadata (model_t model, const char *filename, const void *file_data, size_t file_size, shader_t shader, int32_t priority, void **out_format_data);
bool modelfmt_ply_meshes   (model_t model, const char *filename, shader_t shader, int32_t priority, void *format_data);
void modelfmt_ply_free     (void *format_data);

bool modelfmt_svg_metadata (model_t model, const char *filename, const void *file_data, size_t file_size, shader_t shader, int32_t priority, void **out_format_data);
bool modelfmt_svg_meshes   (model_t model, const char *filename, shader_t shader, int32_t priority, void *format_data);
void modelfmt_svg_free     (void *format_data);

// Single mesh formats parse into vertex data the caller owns, so Mesh can
// load them directly as well. 'name' is only for log messages.
bool modelfmt_stl_build    (const void *file_data, size_t file_size, const char *name, mesh_load_t *out_mesh);
bool modelfmt_ply_build    (const void *file_data, size_t file_size, const char *name, mesh_load_t *out_mesh);

void model_destroy(model_t model);

} // namespace sk
