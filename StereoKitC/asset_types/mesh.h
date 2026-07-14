#pragma once

#include <stdint.h>

#include <sk_renderer.h>

#include "../stereokit.h"
#include "../systems/bvh.h"
#include "assets.h"
#include "mesh_.h"

namespace sk {

struct mesh_weights_t {
	bone_weight_t *bone_data;
	matrix   *bone_inverse_transforms;
	matrix   *bone_transforms;
	// A full vertex buffer in the mesh's own vertex format and stride.
	void     *deformed_verts;
	int32_t   bone_count;
};

struct _mesh_t {
	asset_header_t   header;
	skr_mesh_t       gpu_mesh;
	uint32_t         ind_draw;
	bounds_t         bounds;
	bool32_t         discard_data;
	// Vertex format registry id, VERT_FORMAT_DEFAULT means verts is vert_t
	// data, anything else and verts is raw vert_stride sized vertices.
	int32_t          vert_format;
	uint32_t         vert_stride;
	vert_t*          verts;
	vind_t*          inds;
	uint32_t         vert_count;
	uint32_t         vert_capacity_bytes;
	uint32_t         ind_count;
	uint32_t         ind_capacity;
	mesh_collision_t collision_data;
	mesh_bvh_t*      bvh_data;
	mesh_weights_t   skin_data;
};

void mesh_destroy(mesh_t mesh);

} // namespace sk