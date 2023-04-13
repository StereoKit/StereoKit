#pragma once

#include "../stereokit.h"
#include "assets.h"

namespace sk {

typedef int32_t anchor_type_id;

struct _anchor_t {
	asset_header_t header;
	anchor_type_id source_system;
	pose_t         pose;
	bool32_t       changed;
	bool32_t       persisted;
	button_state_  tracked;
	char*          name;
	void*          data;
};

typedef struct anchor_system_t {
	anchor_type_id id;
	anchor_props_  properties;
	const char*    name;
	void*          context;
	int32_t        priority;

	bool32_t (*on_enable        )(void* context);
	anchor_t (*on_create        )(void* context, pose_t pose, const char *name_utf8);
	bool32_t (*on_persist       )(void* context, anchor_t anchor, bool32_t persist);
	void     (*on_destroy_anchor)(void* context, anchor_t anchor, void* data);
	void     (*on_clear_stored  )(void* context);
} anchor_system_t;

void           anchor_destroy         (anchor_t anchor);
void           anchor_update_manual   (anchor_t anchor, pose_t pose);
anchor_t       anchor_create_manual   (anchor_type_id system_id, pose_t pose, const char* name_utf8, void* data);
void           anchor_notify_discovery(anchor_t anchor);
void           anchor_mark_dirty      (anchor_t anchor);
void           anchor_clear_all_dirty ();
anchor_type_id anchors_register_type  (anchor_system_t system);

}