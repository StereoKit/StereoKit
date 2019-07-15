#pragma once

#include <stdint.h>
#include <d3d11.h>

struct vert_t {
	float   pos [3];
	float   norm[3];
	float   uv  [2];
	uint8_t col [4];
};

struct mesh_t {
	void         *verts;
	int           vert_count;
	ID3D11Buffer *vert_buffer;
	uint16_t     *inds;
	int           ind_count;
	ID3D11Buffer *ind_buffer;
};

void mesh_create        (mesh_t &mesh);
bool mesh_create_file   (mesh_t &mesh, const char *file);
bool mesh_create_filemem(mesh_t &mesh, uint8_t *file_data, uint64_t file_size);
void mesh_destroy   (mesh_t &mesh);
void mesh_set_verts (mesh_t &mesh, vert_t   *verts, int vert_count);
void mesh_set_inds  (mesh_t &mesh, uint16_t *inds,  int ind_count);
void mesh_set_active(mesh_t &mesh);
void mesh_draw      (mesh_t &mesh);