#include "stereokit.h"
#include "hierarchy.h"
#include "sk_math.h"
#include "libraries/array.h"

#include <DirectXMath.h> // Matrix math functions and objects
using namespace DirectX;

///////////////////////////////////////////

using namespace sk;

struct hierarchy_item_t {
	matrix   transform;
	matrix   transform_inv;
	bool32_t has_inverse;
};

struct hierarchy_state_t {
	array_t<hierarchy_item_t> stack;
	bool32_t                  enabled;
	bool32_t                  userenabled;
};
static hierarchy_state_t _local = {};
static hierarchy_state_t *local = {};

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

void hierarchy_init() {
	local = &_local;
	*local = {};
	local->userenabled = true;
}

///////////////////////////////////////////

void hierarchy_shutdown() {
	local->stack.free();
	*local = {};
	local = nullptr;
}

///////////////////////////////////////////

void hierarchy_step() {
	if (local->stack.count > 0) {
		log_err("Render transform stack doesn't have matching begin/end calls!");
		local->stack.clear();
	}
}

///////////////////////////////////////////

void hierarchy_push(const matrix &transform, hierarchy_parent_ parent_behavior) {
	local->stack.add(hierarchy_item_t{transform, matrix_identity, false});
	local->enabled = local->userenabled;

	int32_t size = local->stack.count;
	if (size > 1 && parent_behavior == hierarchy_parent_inherit)
		matrix_mul(local->stack[size - 1].transform, local->stack[size - 2].transform, local->stack[size - 1].transform);
}

///////////////////////////////////////////

void hierarchy_pop() {
	local->stack.pop();
	if (local->stack.count == 0)
		local->enabled = false;
}

///////////////////////////////////////////

void hierarchy_set_enabled(bool32_t enabled) {
	local->userenabled = enabled;
	local->enabled     = local->stack.count != 0 && local->userenabled == (bool32_t)true;
}

///////////////////////////////////////////

bool32_t hierarchy_is_enabled() {
	return local->userenabled;
}

///////////////////////////////////////////

bool32_t hierarchy_use_top() {
	return local->enabled;
}

///////////////////////////////////////////

const matrix *hierarchy_to_world() {
	return local->enabled ? &local->stack.last().transform : &matrix_identity;
}

///////////////////////////////////////////

const matrix *hierarchy_to_local() {
	if (local->enabled) {
		hierarchy_item_t &item = local->stack.last();
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

ray_t hierarchy_to_local_ray(ray_t world_ray) {
	return matrix_transform_ray(*hierarchy_to_local(), world_ray);
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

///////////////////////////////////////////

ray_t hierarchy_to_world_ray(ray_t local_ray) {
	return matrix_transform_ray(*hierarchy_to_world(), local_ray);
}

///////////////////////////////////////////

matrix hierarchy_top() {
	return local->stack.last().transform;
}

} // namespace sk