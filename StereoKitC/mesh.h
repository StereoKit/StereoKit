#pragma once

#include <stdint.h>
#include <d3d11.h>

#include "assets.h"

struct _mesh_t {
	asset_header_t header;
	void          *verts;
	int            vert_count;
	ID3D11Buffer  *vert_buffer;
	uint16_t      *inds;
	int            ind_count;
	ID3D11Buffer  *ind_buffer;
};