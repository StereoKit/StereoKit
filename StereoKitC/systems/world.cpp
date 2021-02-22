#include "../stereokit.h"
#include "../_stereokit.h"
#include "platform/openxr.h"

namespace sk {

bool32_t world_has_bounds() {
	return xr_has_bounds;
}

///////////////////////////////////////////

vec2 world_get_bounds_size() {
	return xr_bounds_size;
}

///////////////////////////////////////////

pose_t world_get_bounds_pose() { 
	return xr_bounds_pose; 
}

} // namespace sk