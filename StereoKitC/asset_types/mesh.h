#pragma once

#include <stdint.h>
#include <d3d11.h>

#include "../stereokit.h"
#include "assets.h"

namespace sk {

struct _mesh_t {
	asset_header_t header;
	int            vert_count;
	ID3D11Buffer  *vert_buffer;
	int            ind_count;
	ID3D11Buffer  *ind_buffer;
	int            ind_draw;
};

void mesh_destroy(mesh_t mesh);

} // namespace sk