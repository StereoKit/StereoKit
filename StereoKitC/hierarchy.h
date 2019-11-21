#pragma once

#include "stereokit.h"

#include <vector>

namespace sk {

struct hierarchy_item_t {
	matrix transform;
	matrix transform_inv;
	bool32_t has_inverse;
};

///////////////////////////////////////////

extern std::vector<hierarchy_item_t> hierarchy_stack;
extern bool32_t                      hierarchy_enabled;
extern bool32_t                      hierarchy_userenabled;

}