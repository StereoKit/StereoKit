#include "demo_anchors.h"

#include <stereokit.h>
#include <stereokit_ui.h>

using namespace sk;

#include <vector>
using namespace std;

vector<anchor_t> anchors;

///////////////////////////////////////////

void demo_anchors_init() {
	for (int32_t i = 0; i < anchor_get_count(); i++)
		anchors.push_back(anchor_get_index(i));
}

///////////////////////////////////////////

void demo_anchors_update() {
	// Show settings window
	static pose_t   window_pose     = pose_t{ {0.4f,0.0,-0.4f}, quat_lookat({}, {-1,0,1}) };
	static bool32_t persist_anchors = (anchor_get_capabilities() & anchor_caps_storable) != 0;
	ui_window_begin("Anchor Options", &window_pose);
	ui_push_enabled((anchor_get_capabilities() & anchor_caps_storable) != 0);
	ui_toggle("Persist New Anchors", persist_anchors);
	ui_pop_enabled();
	if (ui_button("Clear Stored Anchors")) {
		anchor_clear_stored();
		demo_anchors_shutdown();
	}
	ui_window_end();

	// Draw a pose for each anchor
	for (size_t i = 0; i < anchors.size(); i++) {
		line_add_axis(anchor_get_pose(anchors[i]), 0.1f);
		text_add_at(anchor_get_name(anchors[i]), pose_matrix(anchor_get_pose(anchors[i])), 0, pivot_top_center);
	}

	// Some interaction for adding anchors
	if (!ui_is_interacting(handed_right) && input_hand(handed_right)->pinch_state & button_state_just_active || input_key(key_space) & button_state_just_active) {
		pose_t pose = input_head();
		pose.position += pose.orientation * vec3_forward * 0.5f;
		if (input_hand(handed_right)->tracked_state & button_state_active)
			pose.position = input_hand(handed_right)->pinch_pt;
		else if (input_hand(handed_left)->tracked_state & button_state_active)
			pose.position = input_hand(handed_left)->pinch_pt;
		pose.orientation = quat_lookat(pose.position, input_head().position);

		anchor_t anch = anchor_create(pose);
		if (anch) {
			anchors.push_back(anch);
			if (persist_anchors) anchor_try_set_persistent(anch, true);
		} else {
			log_info("Failed to create anchor!");
		}
	}

	for (int32_t i = 0; i < anchor_get_new_count(); i++) {
		anchor_t anchor = anchor_get_new_index(i);
		log_infof("New anchor: %s", anchor_get_name(anchor));
		anchor_release(anchor);
	}
}

///////////////////////////////////////////

void demo_anchors_shutdown() {
	for (size_t i = 0; i < anchors.size(); i++)
		anchor_release(anchors[i]);
	anchors.clear();
}