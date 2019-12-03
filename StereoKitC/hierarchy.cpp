#include "stereokit.h"
#include "hierarchy.h"
#include "math.h"

#include <vector>
using namespace std;

#include <directxmath.h> // Matrix math functions and objects
using namespace DirectX;

namespace sk {

///////////////////////////////////////////

vector<hierarchy_item_t> hierarchy_stack;
bool32_t                 hierarchy_enabled     = false;
bool32_t                 hierarchy_userenabled = true;

///////////////////////////////////////////

void hierarchy_push(const matrix &transform) {
	hierarchy_stack.push_back(hierarchy_item_t{transform, matrix_identity, false});
	hierarchy_enabled = hierarchy_userenabled;

	size_t size = hierarchy_stack.size();
	if (size > 1)
		matrix_mul(hierarchy_stack[size - 1].transform, hierarchy_stack[size - 2].transform, hierarchy_stack[size - 1].transform);
}

///////////////////////////////////////////

void hierarchy_pop() {
	hierarchy_stack.pop_back();
	if (hierarchy_stack.size() == 0)
		hierarchy_enabled = false;
}

///////////////////////////////////////////

void hierarchy_set_enabled(bool32_t enabled) {
	hierarchy_userenabled = enabled;
	hierarchy_enabled = hierarchy_stack.size() != 0 && hierarchy_userenabled;
}

///////////////////////////////////////////

bool32_t hierarchy_is_enabled() {
	return hierarchy_userenabled;
}

///////////////////////////////////////////

const matrix &hierarchy_to_world() {
	return hierarchy_enabled ? hierarchy_stack.back().transform : matrix_identity;
}

///////////////////////////////////////////

const matrix &hierarchy_to_local() {
	if (hierarchy_enabled) {
		hierarchy_item_t &item = hierarchy_stack.back();
		if (!item.has_inverse)
			matrix_inverse(item.transform, item.transform_inv);
		return item.transform_inv;
	} else {
		return matrix_identity;
	}
}

///////////////////////////////////////////

vec3 hierarchy_to_local_point(const vec3 &world_pt) {
	return matrix_mul_point(hierarchy_to_local(), world_pt);
}

///////////////////////////////////////////

vec3 hierarchy_to_local_direction(const vec3 &world_dir) {
	return matrix_mul_direction(hierarchy_to_local(), world_dir);
}

///////////////////////////////////////////

quat hierarchy_to_local_rotation(const quat &world_orientation) {
	return matrix_mul_rotation(hierarchy_to_local(), world_orientation);
}

///////////////////////////////////////////

vec3 hierarchy_to_world_point(const vec3 &local_pt) {
	return matrix_mul_point(hierarchy_to_world(), local_pt);
}

///////////////////////////////////////////

vec3 hierarchy_to_world_direction(const vec3 &local_dir) {
	return matrix_mul_direction(hierarchy_to_world(), local_dir);
}

///////////////////////////////////////////

quat hierarchy_to_world_rotation(const quat &local_orientation) {
	return matrix_mul_rotation(hierarchy_to_world(), local_orientation);
}

} // namespace sk