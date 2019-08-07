#include "stereokit.h"
#include "mesh.h"

#include "d3d.h"
#include "assets.h"

#include <stdio.h>

ID3D11InputLayout *vert_t_layout = nullptr;

void mesh_set_verts(mesh_t mesh, vert_t *vertices, int32_t vertex_count) {
	if (mesh->vert_buffer != nullptr) { 
		// Here would be a good place to not release stuff if doing dynamic meshes
		mesh->vert_buffer->Release();
		mesh->vert_buffer = nullptr; 
	}
	mesh->vert_count = vertex_count;

	if (mesh->vert_buffer == nullptr) {
		D3D11_SUBRESOURCE_DATA vert_buff_data = { vertices };
		CD3D11_BUFFER_DESC     vert_buff_desc(sizeof(vert_t) * vertex_count, D3D11_BIND_VERTEX_BUFFER);
		if (FAILED(d3d_device->CreateBuffer(&vert_buff_desc, &vert_buff_data, &mesh->vert_buffer)))
			log_write(log_error, "Failed to create vertex buffer");
	} else {
		log_write(log_error, "We don't support dynamic meshes quite yet.");
	}
}
void mesh_set_inds (mesh_t mesh, uint16_t *indices,  int32_t index_count) {
	if (mesh->ind_buffer != nullptr) { 
		// Here would be a good place to not release stuff if doing dynamic meshes
		mesh->ind_buffer->Release();
		mesh->ind_buffer = nullptr; 
	}
	mesh->ind_count = index_count;

	if (mesh->ind_buffer == nullptr) {
		D3D11_SUBRESOURCE_DATA ind_buff_data = { indices };
		CD3D11_BUFFER_DESC     ind_buff_desc(sizeof(uint16_t) * index_count, D3D11_BIND_INDEX_BUFFER);
		if (FAILED(d3d_device->CreateBuffer(&ind_buff_desc, &ind_buff_data, &mesh->ind_buffer)))
			log_write(log_error, "Failed to create index buffer");
	} else {
		log_write(log_error, "We don't support dynamic meshes quite yet.");
	}
}

mesh_t mesh_find(const char *id) {
	mesh_t result = (mesh_t)assets_find(id);
	if (result != nullptr) {
		assets_addref(result->header);
		return result;
	}
	return nullptr;
}
mesh_t mesh_create(const char *id) {
	mesh_t result = (_mesh_t*)assets_allocate(asset_type_mesh, id);
	return result;
}

void mesh_release(mesh_t mesh) {
	assets_releaseref(mesh->header);
}

void mesh_destroy(mesh_t mesh) {
	if (mesh->ind_buffer  != nullptr) mesh->ind_buffer ->Release();
	if (mesh->vert_buffer != nullptr) mesh->vert_buffer->Release();
	*mesh = {};
}

void mesh_gen_cube_vert(int i, const vec3 &size, vec3 &pos, vec3 &norm, vec2 &uv) {
	float neg = (float)((i / 4) % 2 ? -1 : 1);
	int nx  = ((i+24) / 16) % 2;
	int ny  = (i / 8)       % 2;
	int nz  = (i / 16)      % 2;
	int u   = ((i+1) / 2)   % 2; // U: 0,1,1,0
	int v   = (i / 2)       % 2; // V: 0,0,1,1

	uv   = { (float)u, (float)v };
	norm = { nx*neg, ny*neg, nz*neg };
	pos  = {
		size.x * (nx ? neg : ny ? (u?-1:1)*neg : (u?1:-1)*neg),
		size.y * (nx || nz ? (v?1:-1) : neg),
		size.z * (nx ? (u?-1:1)*neg : ny ? (v?1:-1) : neg)
	};
}
mesh_t mesh_gen_cube(const char *id, vec3 size, int32_t subdivisions) {
	mesh_t result = (mesh_t)assets_find(id);
	if (result != nullptr) {
		assets_addref(result->header);
		return result;
	}
	result = mesh_create(id);

	subdivisions = max(0,subdivisions) + 2;

	int vert_count = 6*subdivisions*subdivisions;
	int ind_count  = 6*(subdivisions-1)*(subdivisions-1)*6;
	vert_t   *verts = (vert_t   *)malloc(vert_count * sizeof(vert_t));
	uint16_t *inds  = (uint16_t *)malloc(ind_count  * sizeof(uint16_t));

	int ind    = 0;
	int offset = 0;
	for (int i = 0; i < 6*4; i+=4) {
		vec3 p1, p2, p3, p4;
		vec3 n1, n2, n3, n4;
		vec2 u1, u2, u3, u4;

		mesh_gen_cube_vert(i,   size, p1, n1, u1);
		mesh_gen_cube_vert(i+1, size, p2, n2, u2);
		mesh_gen_cube_vert(i+2, size, p3, n3, u3);
		mesh_gen_cube_vert(i+3, size, p4, n4, u4);

		offset = (i/4) * (subdivisions)*(subdivisions);
		for (int y = 0; y < subdivisions; y++) {
			float py    = y / (float)(subdivisions-1);
			int   yOff  = offset + y * subdivisions;
			int   yOffN = offset + (y+1) * subdivisions;

			vec3 pl = vec3_lerp(p1, p4, py);
			vec3 pr = vec3_lerp(p2, p3, py);
			vec3 nl = vec3_lerp(n1, n4, py);
			vec3 nr = vec3_lerp(n2, n3, py);
			vec2 ul = vec2_lerp(u1, u4, py);
			vec2 ur = vec2_lerp(u2, u3, py);

			for (int x = 0; x < subdivisions; x++) {
				float px = x / (float)(subdivisions-1);
				int   ptIndex = x + yOff;
				vert_t *pt = &verts[ptIndex];

				pt->pos = vec3_lerp(pl, pr, px);
				pt->norm= vec3_lerp(nl, nr, px);
				pt->uv  = vec2_lerp(ul, ur, px);
				pt->col = {255,255,255,255};

				if (y != subdivisions-1 && x != subdivisions-1) {

					inds[ind++] = (x  ) + yOff;
					inds[ind++] = (x+1) + yOff;
					inds[ind++] = (x+1) + yOffN;

					inds[ind++] = (x  ) + yOff;
					inds[ind++] = (x+1) + yOffN;
					inds[ind++] = (x  ) + yOffN;
				}
			}
		}
	}

	mesh_set_verts(result, verts, vert_count);
	mesh_set_inds (result, inds,  ind_count);

	free(verts);
	free(inds);

	return result;
}