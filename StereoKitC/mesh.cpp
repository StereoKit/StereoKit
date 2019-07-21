#include "stereokit.h"
#include "mesh.h"

#include "d3d.h"
#include "assets.h"

#include <stdio.h>

ID3D11InputLayout *vert_t_layout = nullptr;

void meshfmt_obj(mesh_t mesh, const char *file);

void mesh_set_verts(mesh_t mesh, vert_t *verts, int vert_count) {
	if (mesh->verts       != nullptr) free(mesh->verts);
	if (mesh->vert_buffer != nullptr) { 
		// Here would be a good place to not release stuff if doing dynamic meshes
		mesh->vert_buffer->Release();
		mesh->vert_buffer = nullptr; 
	}
	mesh->verts      = verts;
	mesh->vert_count = vert_count;

	if (mesh->vert_buffer == nullptr) {
		D3D11_SUBRESOURCE_DATA vert_buff_data = { mesh->verts };
		CD3D11_BUFFER_DESC     vert_buff_desc(sizeof(vert_t) * vert_count, D3D11_BIND_VERTEX_BUFFER);
		if (FAILED(d3d_device->CreateBuffer(&vert_buff_desc, &vert_buff_data, &mesh->vert_buffer)))
			MessageBox(0, "Failed to create vertex buffer\n", "Error", 0);
	} else {
		// We don't support dynamic meshes quite yet.
		throw;
	}
}
void mesh_set_inds (mesh_t mesh, uint16_t *inds,  int ind_count) {
	if (mesh->inds       != nullptr) free(mesh->inds);
	if (mesh->ind_buffer != nullptr) { 
		// Here would be a good place to not release stuff if doing dynamic meshes
		mesh->ind_buffer->Release();
		mesh->ind_buffer = nullptr; 
	}
	mesh->inds      = inds;
	mesh->ind_count = ind_count;

	if (mesh->ind_buffer == nullptr) {
		D3D11_SUBRESOURCE_DATA ind_buff_data = { mesh->inds };
		CD3D11_BUFFER_DESC     ind_buff_desc(sizeof(uint16_t) * ind_count, D3D11_BIND_INDEX_BUFFER);
		if (FAILED(d3d_device->CreateBuffer(&ind_buff_desc, &ind_buff_data, &mesh->ind_buffer)))
			MessageBox(0, "Failed to create index buffer\n", "Error", 0);
	} else {
		// We don't support dynamic meshes quite yet.
		throw;
	}
}

mesh_t mesh_create() {
	mesh_t result = (_mesh_t*)assets_allocate(asset_type_mesh);
	return result;
}
mesh_t mesh_create_file(const char *file) {
	// Open file
	FILE *fp;
	if (fopen_s(&fp, file, "r") != 0 || fp == nullptr)
		return nullptr;

	// Get length of file
	fseek(fp, 0L, SEEK_END);
	uint64_t length = ftell(fp);
	rewind(fp);

	// Read the data
	uint8_t *data = (uint8_t *)malloc(sizeof(uint8_t) *length+1);
	if (data == nullptr) { fclose(fp); return false; }
	fread(data, 1, length, fp);
	fclose(fp);

	// Make the verts
	mesh_t result = mesh_create_filemem(data, length);
	free(data);

	return result;
}
mesh_t mesh_create_filemem(uint8_t *file_data, uint64_t file_size) {
	mesh_t result = mesh_create();
	file_data[file_size] = '\0';
	meshfmt_obj(result, (const char *)file_data);
	return result;
}
void mesh_destroy(mesh_t mesh) {
	if (mesh->ind_buffer  != nullptr) mesh->ind_buffer ->Release();
	if (mesh->vert_buffer != nullptr) mesh->vert_buffer->Release();
	//if (verts.inds        != nullptr) free(verts.inds);
	//if (verts.poss       != nullptr) free(verts.poss);
	mesh = {};
}

void mesh_set_active(mesh_t mesh) {
	UINT strides[] = { sizeof(vert_t) };
	UINT offsets[] = { 0 };
	d3d_context->IASetVertexBuffers    (0, 1, &mesh->vert_buffer, strides, offsets);
	d3d_context->IASetIndexBuffer      (mesh->ind_buffer, DXGI_FORMAT_R16_UINT, 0);
	d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void mesh_draw(mesh_t mesh) {
	d3d_context->DrawIndexed(mesh->ind_count, 0, 0);
}

#include <vector>
#include <map>
using namespace std;

inline int meshfmt_obj_idx(int i1, int i2, int i3, int vSize, int nSize) {
	return i1 + (vSize+1) * (i2 + (nSize+1) * i3);
}
int indexof(int iV, int iT, int iN, vector<vec3> &verts, vector<vec3> &norms, vector<vec2> &uvs, map<int, uint16_t> indmap, vector<vert_t> &mesh_verts) {
	int  id = meshfmt_obj_idx(iV, iN, iT, verts.size(), norms.size());
	map<int, uint16_t>::iterator item = indmap.find(id);
	if (item == indmap.end()) {
		mesh_verts.push_back({ verts[iV - 1], norms[iN - 1], uvs[iT - 1], {255,255,255,255} });
		indmap[id] = mesh_verts.size() - 1;
		return mesh_verts.size() - 1;
	}
	return item->second;
}
void meshfmt_obj(mesh_t mesh, const char *file) {
	size_t len = strlen(file);
	const char *line = file;
	int         read = 0;

	vector<vec3> poss;
	vector<vec3> norms;
	vector<vec2> uvs;

	map<int, uint16_t> indmap;
	vector<vert_t>   verts;
	vector<uint16_t> faces;

	vec3 in;
	int inds[12];
	int count = 0;
	while ((size_t)(line-file)+1 < len) {
		if        (sscanf_s(line, "v %f %f %f\n%n",  &in.x, &in.y, &in.z, &read) > 0) {
			poss.push_back(in);
		} else if (sscanf_s(line, "vn %f %f %f\n%n", &in.x, &in.y, &in.z, &read) > 0) {
			norms.push_back(in);
		} else if (sscanf_s(line, "vt %f %f\n%n",    &in.x, &in.y,        &read) > 0) {
			vec2 uv = { in.x, in.y };
			uvs.push_back(uv);
		} else if ((count = sscanf_s(line, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n%n", &inds[0], &inds[1], &inds[2],&inds[3],&inds[4],&inds[5],&inds[6],&inds[7],&inds[8],&inds[9],&inds[10],&inds[11],  &read)) > 0) {
			int id1 = indexof(inds[0], inds[1],  inds[2],  poss, norms, uvs, indmap, verts);
			int id2 = indexof(inds[3], inds[4],  inds[5],  poss, norms, uvs, indmap, verts);
			int id3 = indexof(inds[6], inds[7],  inds[8],  poss, norms, uvs, indmap, verts);
			faces.push_back(id1); faces.push_back(id2); faces.push_back(id3);
			if (count > 9) {
				int id4 = indexof(inds[9], inds[10], inds[11], poss, norms, uvs, indmap, verts);
				faces.push_back(id1); faces.push_back(id3); faces.push_back(id4);
			}
		} else {
			char str[512];
			sscanf_s(line, "%[^\n]\n%n", str, 512, &read);
		}
		line += read;
	}

	mesh_set_verts(mesh, &verts[0], verts.size());
	mesh_set_inds (mesh, &faces[0], faces.size());
}