#include "stereokit.h"
#include "hierarchy.h"
#include "sk_math.h"
#include "libraries/array.h"

#include <DirectXMath.h> // Matrix math functions and objects
using namespace DirectX;

namespace sk {

///////////////////////////////////////////

array_t<hierarchy_item_t> hierarchy_stack       = {};
bool32_t                  hierarchy_enabled     = false;
bool32_t                  hierarchy_userenabled = true;

///////////////////////////////////////////

void hierarchy_push(const matrix &transform) {
	hierarchy_stack.add(hierarchy_item_t{transform, matrix_identity, false});
	hierarchy_enabled = hierarchy_userenabled;

	int32_t size = hierarchy_stack.count;
	if (size > 1)
		matrix_mul(hierarchy_stack[size - 1].transform, hierarchy_stack[size - 2].transform, hierarchy_stack[size - 1].transform);
}

///////////////////////////////////////////

void hierarchy_pop() {
	hierarchy_stack.pop();
	if (hierarchy_stack.count == 0)
		hierarchy_enabled = false;
}

///////////////////////////////////////////

void hierarchy_set_enabled(bool32_t enabled) {
	hierarchy_userenabled = enabled;
	hierarchy_enabled = hierarchy_stack.count != 0 && hierarchy_userenabled;
}

///////////////////////////////////////////

bool32_t hierarchy_is_enabled() {
	return hierarchy_userenabled;
}

///////////////////////////////////////////

const matrix *hierarchy_to_world() {
	return hierarchy_enabled ? &hierarchy_stack.last().transform : &matrix_identity;
}

///////////////////////////////////////////

const matrix *hierarchy_to_local() {
	if (hierarchy_enabled) {
		hierarchy_item_t &item = hierarchy_stack.last();
		if (!item.has_inverse)
			matrix_inverse(item.transform, item.transform_inv);
		return &item.transform_inv;
	} else {
		return &matrix_identity;
	}
}

///////////////////////////////////////////

vec3 hierarchy_to_local_point(const vec3 &world_pt) {
	return matrix_transform_pt(*hierarchy_to_local(), world_pt);
}

///////////////////////////////////////////

vec3 hierarchy_to_local_direction(const vec3 &world_dir) {
	return matrix_transform_dir(*hierarchy_to_local(), world_dir);
}

///////////////////////////////////////////

quat hierarchy_to_local_rotation(const quat &world_orientation) {
	return matrix_transform_quat(*hierarchy_to_local(), world_orientation);
}

///////////////////////////////////////////

pose_t hierarchy_to_local_pose(const pose_t &world_pose) {
	return matrix_transform_pose(*hierarchy_to_local(), world_pose);
}

///////////////////////////////////////////

vec3 hierarchy_to_world_point(const vec3 &local_pt) {
	return matrix_transform_pt(*hierarchy_to_world(), local_pt);
}

///////////////////////////////////////////

vec3 hierarchy_to_world_direction(const vec3 &local_dir) {
	return matrix_transform_dir(*hierarchy_to_world(), local_dir);
}

///////////////////////////////////////////

quat hierarchy_to_world_rotation(const quat &local_orientation) {
	return matrix_transform_quat(*hierarchy_to_world(), local_orientation);
}

///////////////////////////////////////////

pose_t hierarchy_to_world_pose(const pose_t &local_pose) {
	return matrix_transform_pose(*hierarchy_to_world(), local_pose);
}

} // namespace sk