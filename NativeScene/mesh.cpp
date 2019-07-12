#include "mesh.h"

#include "d3d.h"

#include <stdio.h>

ID3D11InputLayout *vert_t_layout = nullptr;

void mesh_set_verts(mesh_t &mesh, vert_t *verts, int vert_count) {
	if (mesh.verts       != nullptr) free(mesh.verts);
	if (mesh.vert_buffer != nullptr) { 
		// Here would be a good place to not release stuff if doing dynamic meshes
		mesh.vert_buffer->Release();
		mesh.vert_buffer = nullptr; 
	}
	mesh.verts      = verts;
	mesh.vert_count = vert_count;

	if (mesh.vert_buffer == nullptr) {
		D3D11_SUBRESOURCE_DATA vert_buff_data = { mesh.verts };
		CD3D11_BUFFER_DESC     vert_buff_desc(sizeof(vert_t) * vert_count, D3D11_BIND_VERTEX_BUFFER);
		if (FAILED(d3d_device->CreateBuffer(&vert_buff_desc, &vert_buff_data, &mesh.vert_buffer)))
			MessageBox(0, "Failed to create vertex buffer\n", "Error", 0);
	} else {
		// We don't support dynamic meshes quite yet.
		throw;
	}
}
void mesh_set_inds (mesh_t &mesh, uint16_t *inds,  int ind_count) {
	if (mesh.inds       != nullptr) free(mesh.inds);
	if (mesh.ind_buffer != nullptr) { 
		// Here would be a good place to not release stuff if doing dynamic meshes
		mesh.ind_buffer->Release();
		mesh.ind_buffer = nullptr; 
	}
	mesh.inds      = inds;
	mesh.ind_count = ind_count;

	if (mesh.ind_buffer == nullptr) {
		D3D11_SUBRESOURCE_DATA ind_buff_data = { mesh.inds };
		CD3D11_BUFFER_DESC     ind_buff_desc(sizeof(uint16_t) * ind_count, D3D11_BIND_INDEX_BUFFER);
		if (FAILED(d3d_device->CreateBuffer(&ind_buff_desc, &ind_buff_data, &mesh.ind_buffer)))
			MessageBox(0, "Failed to create index buffer\n", "Error", 0);
	} else {
		// We don't support dynamic meshes quite yet.
		throw;
	}
}

void mesh_create(mesh_t &mesh) {
	mesh = {};
}
void mesh_destroy(mesh_t &mesh) {
	if (mesh.ind_buffer  != nullptr) mesh.ind_buffer->Release();
	if (mesh.vert_buffer != nullptr) mesh.vert_buffer->Release();
	//if (mesh.inds        != nullptr) free(mesh.inds);
	//if (mesh.verts       != nullptr) free(mesh.verts);
	mesh = {};
}

void mesh_set_active(mesh_t &mesh) {
	UINT strides[] = { sizeof(vert_t) };
	UINT offsets[] = { 0 };
	d3d_context->IASetVertexBuffers    (0, 1, &mesh.vert_buffer, strides, offsets);
	d3d_context->IASetIndexBuffer      (mesh.ind_buffer, DXGI_FORMAT_R16_UINT, 0);
	d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void mesh_draw(mesh_t &mesh) {
	d3d_context->DrawIndexed(mesh.ind_count, 0, 0);
}