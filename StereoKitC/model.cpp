#define _CRT_SECURE_NO_WARNINGS 1

#include "model.h"
#include "mesh.h"
#include "material.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include <vector>
#include <map>
using namespace std;

bool modelfmt_obj (model_t model, const char *filename);
bool modelfmt_gltf(model_t model, const char *filename);

model_t model_create(const char *id) {
	model_t result = (_model_t*)assets_allocate(asset_type_model, id);
	return result;
}
model_t model_create_file(const char *filename) {
	model_t result = (model_t)assets_find(filename);
	if (result != nullptr) {
		assets_addref(result->header);
		return result;
	}
	result = model_create(filename);

	if        (modelfmt_gltf(result, filename)) {
	} else if (modelfmt_obj (result, filename)) {
	}

	return result;
}
void model_release(model_t model) {
	assets_releaseref(model->header);
}
void model_destroy(model_t model) {
	for (size_t i = 0; i < model->subset_count; i++) {
		mesh_release    (model->subsets[i].mesh);
		material_release(model->subsets[i].material);
	}
	free(model->subsets);
	*model = {};
}

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
bool modelfmt_obj(model_t model, const char *filename) {
	// Open file
	FILE *fp;
	if (fopen_s(&fp, filename, "r") != 0 || fp == nullptr)
		return false;

	// Get length of file
	fseek(fp, 0L, SEEK_END);
	uint64_t length = ftell(fp);
	rewind(fp);

	// Read the data
	uint8_t *data = (uint8_t *)malloc(sizeof(uint8_t) *length+1);
	if (data == nullptr) { fclose(fp); return false; }
	fread(data, 1, length, fp);
	fclose(fp);
	data[length] = '\0';

	// Parse the file
	size_t len = length;
	const char *line = (const char *)data;
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
	while ((size_t)(line-(const char *)data)+1 < len) {
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

	model->subset_count = 1;
	model->subsets = (model_subset_t*)malloc(sizeof(model_subset_t));
	transform_initialize(model->subsets[0].offset);
	model->subsets[0].material = (material_t)assets_find("default/material");
	assets_addref(model->subsets[0].material->header);

	char id[512];
	sprintf_s(id, 512, "%s/mesh", filename);
	model->subsets[0].mesh = mesh_create(id);
	mesh_set_verts(model->subsets[0].mesh, &verts[0], verts.size());
	mesh_set_inds (model->subsets[0].mesh, &faces[0], faces.size());

	free(data);

	return true;
}

mesh_t modelfmt_gltf_parsemesh(cgltf_mesh *mesh, const char *filename) {
	cgltf_mesh      *m = mesh;
	cgltf_primitive *p = &m->primitives[0];

	vert_t *verts = nullptr;
	int     vert_count = 0;

	for (size_t a = 0; a < p->attributes_count; a++) {
		cgltf_attribute   *attr = &p->attributes[a];
		cgltf_buffer_view *buff = attr->data->buffer_view;

		// Make sure we have memory for our verts
		if (vert_count < attr->data->count) {
			vert_count = attr->data->count;
			verts = (vert_t *)realloc(verts, sizeof(vert_t) * vert_count);
		}

		// Check what info is in this attribute, and copy it over to our mesh
		if (attr->type == cgltf_attribute_type_position) {
			for (size_t v = 0; v < attr->data->count; v++) {
				vec3 *pos = (vec3 *)(((uint8_t *)buff->buffer->data) + (sizeof(vec3) * v) + buff->offset);
				verts[v].pos = *pos;
			}
		} else if (attr->type == cgltf_attribute_type_normal) {
			for (size_t v = 0; v < attr->data->count; v++) {
				vec3 *norm = (vec3 *)(((uint8_t *)buff->buffer->data) + (sizeof(vec3) * v) + buff->offset);
				verts[v].norm = *norm;
			}
		} else if (attr->type == cgltf_attribute_type_texcoord) {
			for (size_t v = 0; v < attr->data->count; v++) {
				vec2 *uv = (vec2 *)(((uint8_t *)buff->buffer->data) + (sizeof(vec2) * v) + buff->offset);
				verts[v].uv = *uv;
			}
		} else if (attr->type == cgltf_attribute_type_color) {
			for (size_t v = 0; v < attr->data->count; v++) {
				uint8_t *col = (uint8_t *)(((uint8_t *)buff->buffer->data) + (sizeof(uint8_t) * 4 * v) + buff->offset);
				memcpy(verts[v].col, col, sizeof(uint8_t) * 4);
			}
		}
	}

	// Now grab the mesh indices
	int ind_count = p->indices->count;
	uint16_t *inds = (uint16_t *)malloc(sizeof(uint16_t) * ind_count);
	if (p->indices->component_type == cgltf_component_type_r_16u) {
		cgltf_buffer_view *buff = p->indices->buffer_view;
		for (size_t v = 0; v < ind_count; v++) {
			uint16_t *ind = (uint16_t *)(((uint8_t *)buff->buffer->data) + (sizeof(uint16_t) * v) + buff->offset);
			inds[v] = *ind;
		}
	}

	char id[512];
	sprintf_s(id, 512, "%s/%s", filename, m->name);
	mesh_t result = mesh_create(id);
	mesh_set_verts(result, verts, vert_count);
	mesh_set_inds (result, inds,  ind_count);

	return result;
}
tex2d_t modelfmt_gltf_parsetexture(cgltf_image *image, const char *filename) {
	tex2d_t result = (tex2d_t)assets_find("default/tex2d");
	assets_addref(result->header);
	return result;
}
material_t modelfmt_gltf_parsematerial(cgltf_material *material, const char *filename) {
	material_t result = (material_t)assets_find("default/material");
	assets_addref(result->header);
	return result;
}
bool modelfmt_gltf(model_t model, const char *filename) {
	cgltf_options options = {};
	cgltf_data*   data    = NULL;
	if (cgltf_parse_file(&options, filename, &data) != cgltf_result_success)
		return false;
	if (cgltf_load_buffers(&options, data, filename) != cgltf_result_success) {
		cgltf_free(data);
		return true;
	}
	
	model->subset_count = data->nodes_count;
	model->subsets = (model_subset_t*)malloc(sizeof(model_subset_t) * model->subset_count);

	for (size_t i = 0; i < data->nodes_count; i++) {
		cgltf_node *n = &data->nodes[i];
		model->subsets[i].mesh = modelfmt_gltf_parsemesh(n->mesh, filename);
		model->subsets[i].material = modelfmt_gltf_parsematerial(n->mesh->primitives[0].material, filename);

		vec3 pos = { n->translation[0], n->translation[1], n->translation[2] };
		vec3 scale = { n->scale[0], n->scale[1], n->scale[2] };
		quat rot = { n->rotation[0], n->rotation[1], n->rotation[2], n->rotation[3] };
		transform_set(model->subsets[i].offset, pos, scale, rot);
	}
	cgltf_free(data);
	return true;
}