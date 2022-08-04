#pragma once

#include "../stereokit.h"
#include "../libraries/array.h"
#include "assets.h"

namespace sk {

typedef enum anim_element_ {
	anim_element_translation,
	anim_element_rotation,
	anim_element_scale,
	anim_element_weights,
} anim_element_;

typedef enum anim_interpolation_ {
	anim_interpolation_linear,
	anim_interpolation_step,
	anim_interpolation_cubic,
} anim_interpolation_;

struct anim_curve_t {
	model_node_id       node_id;
	int32_t             keyframe_count;
	float              *keyframe_times;
	void               *keyframe_values;
	anim_element_       applies_to;
	anim_interpolation_ interpolation;
};

struct anim_skeleton_t {
	model_node_id skin_node;
	int32_t       bone_count;
	int32_t      *bone_to_node_map;
};

struct anim_t {
	char                 *name;
	float                 duration;
	array_t<anim_curve_t> curves;
	void                 *anim_memory;
};

struct anim_data_t {
	array_t<anim_t>          anims;
	array_t<anim_skeleton_t> skeletons;
};

struct anim_transform_t {
	vec3 translation;
	vec3 scale;
	quat rotation;
	bool dirty;
};

struct anim_inst_subset_t {
	mesh_t  original_mesh;
	mesh_t  modified_mesh;
	matrix *bone_transforms;
};

struct anim_inst_t {
	int32_t             anim_id;
	int32_t             skinned_mesh_count;
	int32_t             node_count;
	float               start_time;
	float               last_update;
	anim_mode_          mode;
	int32_t            *curve_last_keyframe;
	int32_t             curve_last_capacity;
	anim_inst_subset_t *skinned_meshes;
	anim_transform_t   *node_transforms;
};

void anim_update_model(model_t model);
void anim_update_skin (model_t model);
void anim_inst_play   (model_t model, int32_t anim_id, anim_mode_ mode);
void anim_inst_destroy(anim_inst_t *inst);
void anim_data_destroy(anim_data_t *data);
anim_data_t anim_data_copy(anim_data_t *data);

void anim_update();
void anim_shutdown();

} // namespace sk
