#include "demo_anchors.h"

#include <stereokit.h>
#include <stereokit_ui.h>

using namespace sk;

#include <vector>
using namespace std;

vector<anchor_t> anchors;

///////////////////////////////////////////

void on_anchor_found(void* context, anchor_t anchor) {
	anchors.push_back(anchor);
}

///////////////////////////////////////////

void demo_anchors_init() {
	anchor_subscribe(on_anchor_found, nullptr, false);
}

///////////////////////////////////////////

void demo_anchors_update() {
	// Show settings window
	static pose_t   window_pose     = pose_t{ {0.4f,0.0,-0.4f}, quat_lookat({}, {-1,0,1}) };
	static bool32_t persist_anchors = anchor_get_capabilities() & anchor_caps_storable != 0;
	ui_window_begin("Anchor Options", window_pose);
	ui_push_enabled(anchor_get_capabilities() & anchor_caps_storable != 0);
	ui_toggle("Persist New Anchors", persist_anchors);
	ui_pop_enabled();
	if (ui_button("Clear Stored Anchors")) {
		anchor_clear_stored();
		for (size_t i = 0; i < anchors.size(); i++) {
			anchor_release(anchors[i]);
		}
		anchors.clear();
	}
	ui_window_end();

	// Draw a pose for each anchor
	for (size_t i = 0; i < anchors.size(); i++) {
		line_add_axis(anchor_get_pose(anchors[i]), 0.1f);
		text_add_at(anchor_get_name(anchors[i]), pose_matrix(anchor_get_pose(anchors[i])), 0, text_align_top_center);
	}

	// Some interaction for adding anchors
	if (!ui_is_interacting(handed_right) && input_hand(handed_right)->pinch_state & button_state_just_active || input_key(key_space) & button_state_just_active) {
		pose_t pose = *input_head();
		pose.position += pose.orientation * vec3_forward * 0.5f;
		if (input_hand(handed_right)->tracked_state & button_state_active)
			pose.position = input_hand(handed_right)->pinch_pt;
		else if (input_hand(handed_left)->tracked_state & button_state_active)
			pose.position = input_hand(handed_left)->pinch_pt;
		pose.orientation = quat_lookat(pose.position, input_head()->position);

		anchor_t anch = anchor_create(nullptr, pose);
		if (anch && persist_anchors)
			anchor_set_persistent(anch, true);
	}
}

///////////////////////////////////////////

void demo_anchors_shutdown() {
	anchor_unsubscribe(on_anchor_found, nullptr);

	for (size_t i = 0; i < anchors.size(); i++)
		anchor_release(anchors[i]);
	anchors.clear();
}