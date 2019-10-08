#pragma once

#include "../stereokit.h"

namespace sk {

extern mouse_t input_mouse;

int        input_add_pointer(input_source_ source);
pointer_t *input_get_pointer(int id);

bool input_init    ();
void input_shutdown();
void input_update  ();

inline button_state_ button_make_state(bool32_t was, bool32_t is) { 
	button_state_ result = is ? button_state_down : button_state_up;
	if (was && !is)
		result |= button_state_just_up;
	if (is && !was)
		result |= button_state_just_down;
	return result;
}

} // namespace sk