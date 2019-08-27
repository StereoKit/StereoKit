#pragma once

#include "../stereokit.h"

int        input_add_pointer(input_source_ source);
pointer_t *input_get_pointer(int id);

bool input_init    ();
void input_shutdown();
void input_update  ();