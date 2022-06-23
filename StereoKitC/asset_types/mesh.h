#pragma once

#include <stdint.h>

#include "../libraries/sk_gpu.h"

#include "../stereokit.h"
#include "../sk_math_dx.h"
#include "../systems/bvh.h"
#include "assets.h"
#include "mesh_.h"

namespace sk {


struct mesh_weights_t {
	uint16_t *bone_ids;
	vec4     *weights;
	matrix   *bone_inverse_transforms;
	DirectX::XMMATRIX *bone_transforms;
	vert_t   *deformed_verts;
	int32_t   bone_count;
};

struct _mesh_t {
	asset_header_t   header;
	uint32_t         vert_count;
	uint32_t         vert_capacity;
	bool32_t         vert_dynamic;
	skg_buffer_t     vert_buffer;
	uint32_t         ind_count;
	uint32_t         ind_capacity;
	bool32_t         ind_dynamic;
	skg_buffer_t     ind_buffer;
	uint32_t         ind_draw;
	skg_mesh_t       gpu_mesh;
	bounds_t         bounds;
	bool32_t         discard_data;
	vert_t*          verts;
	vind_t*          inds;
	mesh_collision_t collision_data;
	mesh_bvh_t*      bvh_data;
	mesh_weights_t   skin_data;
};

void mesh_destroy(mesh_t mesh);

} // namespace sk