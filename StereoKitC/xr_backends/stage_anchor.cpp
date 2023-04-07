#include "stage_anchor.h"
#include "../asset_types/anchor.h"

namespace sk {

typedef struct stage_anchor_sys_t {
	anchor_type_id id;
} stage_anchor_sys_t;

typedef struct stage_anchor_t {
	pose_t relative_pose;
} stage_anchor_t;

stage_anchor_sys_t stage_anchor_sys = {};

///////////////////////////////////////////

anchor_t stage_anchor_on_create(stage_anchor_sys_t* context, pose_t pose);

///////////////////////////////////////////

void stage_anchor_init() {
	stage_anchor_sys = {};

	anchor_system_t stage_anchor_sys = {};
	stage_anchor_sys.info.description       = "Stage Anchor";
	stage_anchor_sys.info.type              = anchor_type_stage_anchor;
	stage_anchor_sys.info.properties        = anchor_props_creatable;
	stage_anchor_sys.info.requires_enabling = false;
	stage_anchor_sys.context   = &stage_anchor_sys;
	stage_anchor_sys.on_create = (anchor_t(*)(void* context, pose_t pose)) stage_anchor_on_create;

	stage_anchor_sys.id = anchors_register_type(stage_anchor_sys);
}

///////////////////////////////////////////

void stage_anchor_shutdown() {

}

///////////////////////////////////////////

anchor_t stage_anchor_on_create(stage_anchor_sys_t* context, pose_t pose) {
	pose_t relative_pose = pose;
	if (world_has_bounds()) {
		matrix to_relative = matrix_invert( pose_matrix(world_get_bounds_pose()) );
		relative_pose = matrix_transform_pose(to_relative, pose);
	}

	// Create a StereoKit anchor
	stage_anchor_t* anchor_data = sk_malloc_t(stage_anchor_t, 1);
	anchor_data->relative_pose = pose;
	return anchor_create_manual(context->id, pose, (void*)anchor_data);
}

} // namespace sk