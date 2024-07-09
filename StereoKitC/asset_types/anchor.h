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

typedef enum anchor_system_ {
	anchor_system_none,
	anchor_system_stage,
	anchor_system_openxr_msft,
} anchor_system_;

void           anchor_destroy         (anchor_t anchor);
void           anchor_update_manual   (anchor_t anchor, pose_t pose);
anchor_t       anchor_create_manual   (anchor_type_id system_id, pose_t pose, const char* name_utf8, void* data);
void           anchor_mark_dirty      (anchor_t anchor);
void           anchor_clear_all_dirty ();
void           anchors_init           (anchor_system_ system);
void           anchors_shutdown       ();
void           anchors_step_begin     ();
void           anchors_step_end       ();

}