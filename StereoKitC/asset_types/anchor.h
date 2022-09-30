#pragma once

#include "../stereokit.h"
#include "assets.h"

namespace sk {

struct _anchor_t {
	asset_header_t header;
	anchor_type_id source_system;
	bounds_t       bounds;
	pose_t         pose;
	bool32_t       changed;
	void*          data;
};

typedef struct anchor_system_t {
	anchor_type_t info;
	void*         context;
	int32_t       priority;

	bool32_t (*on_enable        )(void* context);
	void     (*on_destroy_anchor)(void* context, anchor_t anchor, void *data);
} anchor_system_t;

void           anchor_destroy        (anchor_t anchor);
void           anchor_update_manual  (anchor_t anchor, pose_t pose);
anchor_t       anchor_create_manual  (anchor_type_id system_id, pose_t pose, void* data);
void           anchor_mark_dirty     (anchor_t anchor);
void           anchor_clear_all_dirty();
anchor_type_id anchors_register_type (anchor_system_t system);

}