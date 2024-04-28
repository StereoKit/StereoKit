#pragma once

#include "stereokit.h"
#include "libraries/array.h"

namespace sk {



///////////////////////////////////////////

void hierarchy_init    ();
void hierarchy_shutdown();
void hierarchy_step    ();

matrix   hierarchy_top    ();
bool32_t hierarchy_use_top();

}