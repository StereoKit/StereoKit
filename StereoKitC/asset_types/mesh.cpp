#include "../stereokit.h"
#include "../systems/d3d.h"
#include "mesh.h"
#include "assets.h"

#include <stdio.h>

namespace sk {

///////////////////////////////////////////

ID3D11InputLayout *vert_t_layout = nullptr;

///////////////////////////////////////////

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
			log_err("mesh_set_verts: Failed to create vertex buffer");
		DX11ResType(mesh->vert_buffer, "verts");
	} else {
		log_err("mesh_set_verts: We don't support dynamic meshes quite yet.");
	}
}

///////////////////////////////////////////

void mesh_set_inds (mesh_t mesh, vind_t *indices,  int32_t index_count) {
	if (mesh->ind_buffer != nullptr) { 
		// Here would be a good place to not release stuff if doing dynamic meshes
		mesh->ind_buffer->Release();
		mesh->ind_buffer = nullptr; 
	}
	mesh->ind_count = index_count;
	mesh->ind_draw  = index_count;

	if (mesh->ind_buffer == nullptr) {
		D3D11_SUBRESOURCE_DATA ind_buff_data = { indices };
		CD3D11_BUFFER_DESC     ind_buff_desc(sizeof(vind_t) * index_count, D3D11_BIND_INDEX_BUFFER);
		if (FAILED(d3d_device->CreateBuffer(&ind_buff_desc, &ind_buff_data, &mesh->ind_buffer)))
			log_err("mesh_set_inds: Failed to create index buffer");
		DX11ResType(mesh->ind_buffer,  "inds");
	} else {
		log_err("mesh_set_inds: We don't support dynamic meshes quite yet.");
	}
}

///////////////////////////////////////////

void mesh_set_draw_inds(mesh_t mesh, int32_t index_count) {
	if (index_count > mesh->ind_count) {
		index_count = mesh->ind_count;
		log_warn("mesh_set_draw_inds: Can't render more indices than the mesh has! Capping...");
	}
	mesh->ind_draw  = index_count;
}

///////////////////////////////////////////

mesh_t mesh_find(const char *id) {
	mesh_t result = (mesh_t)assets_find(id);
	if (result != nullptr) {
		assets_addref(result->header);
		return result;
	}
	return nullptr;
}

///////////////////////////////////////////

void mesh_set_id(mesh_t mesh, const char *id) {
	assets_set_id(mesh->header, id);
}

///////////////////////////////////////////

mesh_t mesh_create() {
	mesh_t result = (_mesh_t*)assets_allocate(asset_type_mesh);
	return result;
}

///////////////////////////////////////////

void mesh_release(mesh_t mesh) {
	if (mesh == nullptr)
		return;
	assets_releaseref(mesh->header);
}

///////////////////////////////////////////

void mesh_destroy(mesh_t mesh) {
	if (mesh->ind_buffer  != nullptr) mesh->ind_buffer ->Release();
	if (mesh->vert_buffer != nullptr) mesh->vert_buffer->Release();
	*mesh = {};
}

///////////////////////////////////////////

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

///////////////////////////////////////////

mesh_t mesh_gen_cube(vec3 dimensions, int32_t subdivisions) {
	mesh_t result = mesh_create();

	subdivisions = max(0,subdivisions) + 2;

	int vert_count = 6*subdivisions*subdivisions;
	int ind_count  = 6*(subdivisions-1)*(subdivisions-1)*6;
	vert_t *verts = (vert_t *)malloc(vert_count * sizeof(vert_t));
	vind_t *inds  = (vind_t *)malloc(ind_count  * sizeof(vind_t));

	vec3 size   = dimensions / 2;
	int  ind    = 0;
	int  offset = 0;
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

	DX11ResType(result->ind_buffer,  "inds_gen_cube" );
	DX11ResType(result->vert_buffer, "verts_gen_cube");
	return result;
}

///////////////////////////////////////////

mesh_t mesh_gen_sphere(float diameter, int32_t subdivisions) {
	mesh_t result = mesh_create();

	subdivisions = max(0,subdivisions) + 2;

	int vert_count = 6*subdivisions*subdivisions;
	int ind_count  = 6*(subdivisions-1)*(subdivisions-1)*6;
	vert_t *verts = (vert_t *)malloc(vert_count * sizeof(vert_t));
	vind_t *inds  = (vind_t *)malloc(ind_count  * sizeof(vind_t));

	vec3  size = vec3_one;
	float radius = diameter / 2;
	int   ind    = 0;
	int   offset = 0;
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
			vec2 ul = vec2_lerp(u1, u4, py);
			vec2 ur = vec2_lerp(u2, u3, py);

			for (int x = 0; x < subdivisions; x++) {
				float px = x / (float)(subdivisions-1);
				int   ptIndex = x + yOff;
				vert_t *pt = &verts[ptIndex];

				pt->norm= vec3_normalize(vec3_lerp(pl, pr, px));
				pt->pos = pt->norm*radius;
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

	DX11ResType(result->ind_buffer,  "inds_gen_sphere" );
	DX11ResType(result->vert_buffer, "verts_gen_sphere");
	return result;
}

///////////////////////////////////////////

mesh_t mesh_gen_rounded_cube(vec3 dimensions, float edge_radius, int32_t subdivisions) {
	mesh_t result = mesh_create();

	subdivisions = max(0,subdivisions) + 2;
	if (subdivisions % 2 == 1) // need an even number of subdivisions
		subdivisions += 1;

	int vert_count = 6*subdivisions*subdivisions;
	int ind_count  = 6*(subdivisions-1)*(subdivisions-1)*6;
	vert_t *verts = (vert_t *)malloc(vert_count * sizeof(vert_t));
	vind_t *inds  = (vind_t *)malloc(ind_count  * sizeof(vind_t));

	vec3  off = (dimensions / 2) - vec3_one*edge_radius;
	vec3  size = vec3_one;
	float radius = edge_radius;
	int   ind    = 0;
	int   offset = 0;
	for (int i = 0; i < 6*4; i+=4) {
		vec3 p1, p2, p3, p4;
		vec3 n1, n2, n3, n4;
		vec2 u1, u2, u3, u4;

		mesh_gen_cube_vert(i,   size, p1, n1, u1);
		mesh_gen_cube_vert(i+1, size, p2, n2, u2);
		mesh_gen_cube_vert(i+2, size, p3, n3, u3);
		mesh_gen_cube_vert(i+3, size, p4, n4, u4);

		float sizeU = vec3_magnitude((p4 - p1) * (dimensions/2));
		float sizeV = vec3_magnitude((p2 - p1) * (dimensions/2));

		offset = (i/4) * (subdivisions)*(subdivisions);
		int x, y;
		for (int sy = 0; sy < subdivisions; sy++) {
			bool first_half_y = sy < subdivisions / 2;
			y = first_half_y ? sy : sy-1;
			vec3 stretchA  = first_half_y ? p1 : p4;
			vec3 stretchB  = first_half_y ? p2 : p3;
			float stretchV = (radius*2)/sizeV;
			float offV     = first_half_y ? 0 : sizeV-(radius*2);
			
			float py    = y / (float)(subdivisions-2);
			float pv    = py * stretchV + offV;
			int   yOff  = offset + sy * subdivisions;
			int   yOffN = offset + (sy+1) * subdivisions;
			
			vec3 pl = vec3_lerp(p1, p4, py);
			vec3 pr = vec3_lerp(p2, p3, py);
			vec2 ul = vec2_lerp(u1, u4, pv);
			vec2 ur = vec2_lerp(u2, u3, pv);

			for (int sx = 0; sx < subdivisions; sx++) {
				bool first_half_x = sx < subdivisions / 2;
				x = first_half_x ? sx : sx-1;
				vec3  stretch = first_half_x ? stretchA : stretchB;
				float stretchU = (radius*2)/sizeU;
				float offU     = first_half_x ? 0 : sizeU-(radius*2);

				float px      = x / (float)(subdivisions-2);
				float pu      = px * stretchU + offU;
				int   ptIndex = sx + yOff;
				vert_t *pt    = &verts[ptIndex];

				pt->norm= vec3_normalize(vec3_lerp(pl, pr, px));
				pt->pos = pt->norm*radius + stretch*off;
				pt->uv = vec2_lerp(ul, ur, pu);
				pt->col = {255,255,255,255};

				if (sy != subdivisions-1 && sx != subdivisions-1) {
					inds[ind++] = (sx  ) + yOff;
					inds[ind++] = (sx+1) + yOff;
					inds[ind++] = (sx+1) + yOffN;

					inds[ind++] = (sx  ) + yOff;
					inds[ind++] = (sx+1) + yOffN;
					inds[ind++] = (sx  ) + yOffN;
				}
			}
		}
	}

	mesh_set_verts(result, verts, vert_count);
	mesh_set_inds (result, inds,  ind_count);

	free(verts);
	free(inds);

	DX11ResType(result->ind_buffer,  "inds_gen_rndcube" );
	DX11ResType(result->vert_buffer, "verts_gen_rndcube");
	return result;
}

} // namespace sk