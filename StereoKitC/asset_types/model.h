#pragma once

#include "../stereokit.h"
#include "assets.h"

namespace sk {

struct model_subset_t {
	char       *name;
	mesh_t      mesh;
	material_t  material;
	matrix      transform;
};

typedef enum model_anim_element_ {
	model_anim_element_translation,
	model_anim_element_rotation,
	model_anim_element_scale,
} model_anim_element_;

struct model_anim_curve_t {
	int32_t             node_id;
	int32_t             keyframe_count;
	float              *keyframe_times;
	void               *keyframe_values;
	model_anim_element_ applies_to;
};

struct model_anim_t {
	char               *name;
	float               duration;
	int32_t             curve_count;
	model_anim_curve_t *curves;
	void               *anim_memory;
};

struct anim_inst_subset_t {
	mesh_t modified_mesh;
	matrix transform;
};

struct anim_inst_t {
	model_t             model;
	int32_t             anim_id;
	float               anim_time;
	int32_t            *curve_last_keyframe;
	anim_inst_subset_t *subsets;
};

struct _model_t {
	asset_header_t  header;
	model_subset_t *subsets;
	int32_t         subset_count;
	model_anim_t   *anims;
	int32_t         anim_count;
	bounds_t        bounds;
};

bool modelfmt_fbx (model_t model, const char *filename, void *file_data, size_t file_size, shader_t shader);
bool modelfmt_obj (model_t model, const char *filename, void *file_data, size_t file_size, shader_t shader);
bool modelfmt_gltf(model_t model, const char *filename, void *file_data, size_t file_size, shader_t shader);
bool modelfmt_stl (model_t model, const char *filename, void *file_data, size_t file_size, shader_t shader);
bool modelfmt_ply (model_t model, const char *filename, void *file_data, size_t file_size, shader_t shader);
void model_destroy(model_t model);

} // namespace sk