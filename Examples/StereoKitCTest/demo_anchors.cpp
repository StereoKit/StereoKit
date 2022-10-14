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
	anchors_subscribe(on_anchor_found, nullptr, false);
}

///////////////////////////////////////////

void demo_anchors_update() {
	for (size_t i = 0; i < anchors.size(); i++) {
		line_add_axis(anchor_get_pose(anchors[i]), 0.1f);
	}
	if (input_hand(handed_right)->grip_state & button_state_just_active)
		anchor_create(input_hand(handed_right)->palm);
}

///////////////////////////////////////////

void demo_anchors_shutdown() {
	anchors_unsubscribe(on_anchor_found, nullptr);

	for (size_t i = 0; i < anchors.size(); i++)
		anchor_release(anchors[i]);
	anchors.clear();
}