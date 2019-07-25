#include "stereokit.h"
#include "mesh.h"

#include "d3d.h"
#include "assets.h"

#include <stdio.h>

ID3D11InputLayout *vert_t_layout = nullptr;

void mesh_set_verts(mesh_t mesh, vert_t *vertices, int vertex_count) {
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
			MessageBox(0, "Failed to create vertex buffer\n", "Error", 0);
	} else {
		// We don't support dynamic meshes quite yet.
		throw;
	}
}
void mesh_set_inds (mesh_t mesh, uint16_t *indices,  int index_count) {
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
			MessageBox(0, "Failed to create index buffer\n", "Error", 0);
	} else {
		// We don't support dynamic meshes quite yet.
		throw;
	}
}

mesh_t mesh_create(const char *name) {
	mesh_t result = (_mesh_t*)assets_allocate(asset_type_mesh, name);
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