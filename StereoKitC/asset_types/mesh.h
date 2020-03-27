#pragma once

#include <stdint.h>
#include <d3d11.h>

#include "../stereokit.h"
#include "assets.h"

namespace sk {

struct mesh_collision_t {
	vec3*    pts;
	plane_t* planes;
};

struct _mesh_t {
	asset_header_t header;
	int            vert_count;
	int            vert_capacity;
	bool32_t       vert_dynamic;
	ID3D11Buffer*  vert_buffer;
	int            ind_count;
	int            ind_capacity;
	bool32_t       ind_dynamic;
	ID3D11Buffer*  ind_buffer;
	int            ind_draw;
	bounds_t       bounds;
	bool32_t       discard_data;
	vert_t*        verts;
	vind_t*        inds;
	mesh_collision_t collision_data;
};

const mesh_collision_t *mesh_get_collision_data(mesh_t mesh);
void mesh_destroy(mesh_t mesh);

} // namespace sk