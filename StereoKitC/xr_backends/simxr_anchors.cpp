#include "simxr_anchors.h"
#include "../asset_types/anchor.h"

namespace sk {

///////////////////////////////////////////

void simxr_anchors_init() {
	anchor_system_t stage_anchor_sys = {};
	stage_anchor_sys.info.description       = "Stage Anchor";
	stage_anchor_sys.info.type              = anchor_type_stage_anchor;
	stage_anchor_sys.info.properties        = anchor_props_creatable;
	stage_anchor_sys.info.requires_enabling = false;

	anchors_register_type(stage_anchor_sys);
}

} // namespace sk