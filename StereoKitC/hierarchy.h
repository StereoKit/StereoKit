#pragma once

#include "stereokit.h"
#include "libraries/array.h"

namespace sk {

struct hierarchy_item_t {
	matrix transform;
	matrix transform_inv;
	bool32_t has_inverse;
};

///////////////////////////////////////////

extern array_t<hierarchy_item_t> hierarchy_stack;
extern bool32_t                  hierarchy_enabled;
extern bool32_t                  hierarchy_userenabled;

}