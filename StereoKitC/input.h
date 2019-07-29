#pragma once

#include "stereokit.h"

enum hand_ {
	hand_left = 0,
	hand_right = 1
};
struct hand_t {
	hand_ handedness;
	vec3 pos;
	quat rot;
	bool tracked;
};

int        input_add_pointer(pointer_source_ source);
pointer_t *input_get_pointer(int id);

void input_init    ();
void input_shutdown();
void input_update  ();