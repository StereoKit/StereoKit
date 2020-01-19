#define _CRT_SECURE_NO_WARNINGS 1

#include "../math.h"
#include "model.h"
#include "mesh.h"
#include "material.h"
#include "texture.h"

#pragma warning( disable : 26451 )
#define CGLTF_IMPLEMENTATION
#include "../libraries/cgltf.h"
#pragma warning( default: 26451 )

#include <vector>
#include <map>
using namespace std;

namespace sk {

///////////////////////////////////////////

bool modelfmt_obj (model_t model, const char *filename, void *file_data, size_t file_size, shader_t shader);
bool modelfmt_gltf(model_t model, const char *filename, void *file_data, size_t file_size, shader_t shader);

///////////////////////////////////////////

model_t model_create() {
	model_t result = (_model_t*)assets_allocate(asset_type_model);
	return result;
}

///////////////////////////////////////////

void model_set_id(model_t model, const char *id) {
	assets_set_id(model->header, id);
}

///////////////////////////////////////////

model_t model_find(const char *id) {
	model_t result = (model_t)assets_find(id, asset_type_model);
	if (result != nullptr) {
		assets_addref(result->header);
		return result;
	}
	return nullptr;
}

///////////////////////////////////////////

model_t model_create_mesh(mesh_t mesh, material_t material) {
	model_t result = model_create();

	model_add_subset(result, mesh, material, matrix_identity);

	return result;
}

///////////////////////////////////////////

model_t model_create_file(const char *filename, shader_t shader) {
	model_t result = model_find(filename);
	if (result != nullptr)
		return result;
	result = model_create();
	model_set_id(result, filename);

	const char *model_file = assets_file(filename);
	// Open file
	FILE *fp;
	if (fopen_s(&fp, model_file, "rb") != 0 || fp == nullptr) {
		log_errf("Can't find file %s!", model_file);
		return nullptr;
	}

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

	if        (modelfmt_gltf(result, filename, data, length, shader)) {
	} else if (modelfmt_obj (result, filename, data, length, shader)) {
	} else {
		log_errf("Issue loading %s! Can't recognize the file format.", filename);
	}

	free(data);

	return result;
}

///////////////////////////////////////////

material_t model_get_material(model_t model, int32_t subset) {
	return model->subsets[subset].material;
}

///////////////////////////////////////////

int32_t model_subset_count(model_t model) {
	return model->subset_count;
}

///////////////////////////////////////////

int32_t model_add_subset(model_t model, mesh_t mesh, material_t material, const matrix &transform) {
	model->subsets                      = (model_subset_t *)realloc(model->subsets, sizeof(model_subset_t) * (model->subset_count + 1));
	model->subsets[model->subset_count] = model_subset_t{ mesh, material, transform };
	assets_addref(mesh->header);
	assets_addref(material->header);

	// Get initial size
	vec3 min, max;
	if (model->subset_count > 0) {
		min = model->bounds.center - model->bounds.dimensions / 2;
		max = model->bounds.center + model->bounds.dimensions / 2;
	} else {
		min = max = matrix_mul_point( transform, bounds_corner(mesh->bounds, 0) );
	}

	// Add the size of this subset
	for (int32_t i = 1; i < 8; i += 1) {
		vec3 pt = matrix_mul_point( transform, bounds_corner(mesh->bounds, i) );
		min.x = fminf(pt.x, min.x);
		min.y = fminf(pt.y, min.y);
		min.z = fminf(pt.z, min.z);

		max.x = fmaxf(pt.x, max.x);
		max.y = fmaxf(pt.y, max.y);
		max.z = fmaxf(pt.z, max.z);
	}
	model->bounds = bounds_t{ min / 2 + max / 2, max - min };

	model->subset_count += 1;
	return model->subset_count - 1;
}

///////////////////////////////////////////

void model_release(model_t model) {
	if (model == nullptr)
		return;

	assets_releaseref(model->header);
}

///////////////////////////////////////////

void model_set_bounds(model_t model, const bounds_t &bounds) {
	model->bounds = bounds;
}

///////////////////////////////////////////

bounds_t model_get_bounds(model_t model) {
	return model->bounds;
}

///////////////////////////////////////////

void model_destroy(model_t model) {
	for (size_t i = 0; i < model->subset_count; i++) {
		mesh_release    (model->subsets[i].mesh);
		material_release(model->subsets[i].material);
	}
	free(model->subsets);
	*model = {};
}

///////////////////////////////////////////

inline int meshfmt_obj_idx(int i1, int i2, int i3, int vSize, int nSize) {
	return i1 + (vSize+1) * (i2 + (nSize+1) * i3);
}

///////////////////////////////////////////

int indexof(int iV, int iT, int iN, vector<vec3> &verts, vector<vec3> &norms, vector<vec2> &uvs, map<int, vind_t> indmap, vector<vert_t> &mesh_verts) {
	int  id = meshfmt_obj_idx(iV, iN, iT, (int)verts.size(), (int)norms.size());
	map<int, vind_t>::iterator item = indmap.find(id);
	if (item == indmap.end()) {
		mesh_verts.push_back({ verts[iV - 1LL], norms[iN - 1LL], uvs[iT - 1LL], {255,255,255,255} });
		indmap[id] = (vind_t)(mesh_verts.size() - 1);
		return (int)mesh_verts.size() - 1;
	}
	return item->second;
}

///////////////////////////////////////////

bool modelfmt_obj(model_t model, const char *filename, void *file_data, size_t file_length, shader_t shader) {

	// Parse the file
	size_t len = file_length;
	const char *line = (const char *)file_data;
	int         read = 0;

	vector<vec3> poss;
	vector<vec3> norms;
	vector<vec2> uvs;

	map<int, vind_t> indmap;
	vector<vert_t>   verts;
	vector<vind_t> faces;

	vec3 in;
	int inds[12];
	int count = 0;
	while ((size_t)(line-(const char *)file_data)+1 < len) {
		if        (sscanf_s(line, "v %f %f %f\n%n",  &in.x, &in.y, &in.z, &read) > 0) {
			poss.push_back(in);
		} else if (sscanf_s(line, "vn %f %f %f\n%n", &in.x, &in.y, &in.z, &read) > 0) {
			norms.push_back(in);
		} else if (sscanf_s(line, "vt %f %f\n%n",    &in.x, &in.y,        &read) > 0) {
			vec2 uv = { in.x, in.y };
			uvs.push_back(uv);
		} else if ((count = sscanf_s(line, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n%n", &inds[0], &inds[1], &inds[2],&inds[3],&inds[4],&inds[5],&inds[6],&inds[7],&inds[8],&inds[9],&inds[10],&inds[11],  &read)) > 0) {
			vind_t id1 = (vind_t)indexof(inds[0], inds[1],  inds[2],  poss, norms, uvs, indmap, verts);
			vind_t id2 = (vind_t)indexof(inds[3], inds[4],  inds[5],  poss, norms, uvs, indmap, verts);
			vind_t id3 = (vind_t)indexof(inds[6], inds[7],  inds[8],  poss, norms, uvs, indmap, verts);
			faces.push_back(id1); faces.push_back(id2); faces.push_back(id3);
			if (count > 9) {
				vind_t id4 = (vind_t)indexof(inds[9], inds[10], inds[11], poss, norms, uvs, indmap, verts);
				faces.push_back(id1); faces.push_back(id3); faces.push_back(id4);
			}
		} else {
			char str[512];
			sscanf_s(line, "%[^\n]\n%n", str, 512, &read);
		}
		line += read;
	}

	char id[512];
	sprintf_s(id, 512, "%s/mesh", filename);
	mesh_t mesh = mesh_create();
	mesh_set_id   (mesh, id);
	mesh_set_verts(mesh, &verts[0], (int32_t)verts.size());
	mesh_set_inds (mesh, &faces[0], (int32_t)faces.size());

	model_add_subset(model, mesh, shader == nullptr ? material_find("default/material") : material_create(shader), matrix_identity);

	mesh_release(mesh);

	return true;
}

///////////////////////////////////////////

mesh_t gltf_parsemesh(cgltf_mesh *mesh, int node_id, const char *filename) {
	cgltf_mesh      *m = mesh;
	cgltf_primitive *p = &m->primitives[0];

	if (p->type != cgltf_primitive_type_triangles)
		log_warnf("Unimplemented gltf primitive mode: %d", p->type);

	char id[512];
	sprintf_s(id, 512, "%s/node_%d_%s", filename, node_id, m->name);
	mesh_t result = mesh_find(id);
	if (result != nullptr) {
		return result;
	}

	vert_t *verts = nullptr;
	int     vert_count = 0;

	for (size_t a = 0; a < p->attributes_count; a++) {
		cgltf_attribute   *attr   = &p->attributes[a];
		cgltf_buffer_view *buff   = attr->data->buffer_view;
		size_t             offset = buff->offset + attr->data->offset;

		// Make sure we have memory for our verts
		if (vert_count < attr->data->count) {
			vert_count = (int)attr->data->count;
			verts = (vert_t *)realloc(verts, sizeof(vert_t) * vert_count);
			for (size_t i = 0; i < vert_count; i++) {
				verts[i] = vert_t{ vec3_zero, vec3_zero, vec2_zero, {255,255,255,255} };
			}
		}

		// Check what info is in this attribute, and copy it over to our mesh
		if (attr->type == cgltf_attribute_type_position) {
			for (size_t v = 0; v < attr->data->count; v++) {
				vec3 *pos = (vec3 *)(((uint8_t *)buff->buffer->data) + (sizeof(vec3) * v) + offset);
				verts[v].pos = *pos;
			}
		} else if (attr->type == cgltf_attribute_type_normal) {
			for (size_t v = 0; v < attr->data->count; v++) {
				vec3 *norm = (vec3 *)(((uint8_t *)buff->buffer->data) + (sizeof(vec3) * v) + offset);
				verts[v].norm = *norm;
			}
		} else if (attr->type == cgltf_attribute_type_texcoord) {
			for (size_t v = 0; v < attr->data->count; v++) {
				vec2 *uv = (vec2 *)(((uint8_t *)buff->buffer->data) + (sizeof(vec2) * v) + offset);
				verts[v].uv = *uv;
			}
		} else if (attr->type == cgltf_attribute_type_color) {
			for (size_t v = 0; v < attr->data->count; v++) {
				color32 *col = (color32 *)(((uint8_t *)buff->buffer->data) + (sizeof(color32) * v) + offset);
				memcpy(&verts[v].col, col, sizeof(color32));
			}
		}
	}

	// Now grab the mesh indices
	int ind_count = (int)p->indices->count;
	vind_t *inds = (vind_t *)malloc(sizeof(vind_t) * ind_count);
	if (p->indices->component_type == cgltf_component_type_r_16u) {
		cgltf_buffer_view *buff   = p->indices->buffer_view;
		size_t             offset = buff->offset + p->indices->offset;
		for (size_t v = 0; v < ind_count; v++) {
			uint16_t *ind = (uint16_t *)(((uint8_t *)buff->buffer->data) + (sizeof(uint16_t) * v) + offset);
			inds[v] = *ind;
		}
	} else if (p->indices->component_type == cgltf_component_type_r_32u) {
		cgltf_buffer_view *buff   = p->indices->buffer_view;
		size_t             offset = buff->offset + p->indices->offset;
		for (size_t v = 0; v < ind_count; v++) {
			uint32_t *ind = (uint32_t *)(((uint8_t *)buff->buffer->data) + (sizeof(uint32_t) * v) + offset);
#ifdef SK_32BIT_INDICES
			inds[v] = *ind;
#else
			inds[v] = *ind > 0x0000FFFF ? 0 : (uint16_t)*ind;
#endif
		}
	}

	result = mesh_create();
	mesh_set_id   (result, id);
	mesh_set_verts(result, verts, vert_count);
	mesh_set_inds (result, inds,  ind_count);
	free(verts);
	free(inds );

	return result;
}

///////////////////////////////////////////

void gltf_imagename(cgltf_data *data, cgltf_image *image, const char *filename, char *dest, int dest_length) {
	if (image->uri != nullptr && strncmp(image->uri, "data:", 5) != 0 && strstr(image->uri, "://") == nullptr) {
		char *last1 = strrchr((char*)filename, '/');
		char *last2 = strrchr((char*)filename, '\\');
		char *last  = max(last1, last2);
		sprintf_s(dest, dest_length, "%.*s/%s", (int)(last - filename), filename, image->uri);
		return;
	}

	for (size_t i = 0; i < data->images_count; i++) {
		if (&data->images[i] == image) {
			sprintf_s(dest, dest_length, "%s/image%d", filename, (int)i);
			return;
		}
	}

	sprintf_s(dest, dest_length, "%s/unknown_image", filename);
}

///////////////////////////////////////////

tex_t gltf_parsetexture(cgltf_data* data, cgltf_image *image, const char *filename, bool srgb_data) {
	// Check if we've already loaded this image
	char id[512];
	gltf_imagename(data, image, filename, id, 512);
	tex_t result = tex_find(id);
	if (result != nullptr) {
		return result;
	}
	
	if (image->buffer_view != nullptr) {
		// If it's already a loaded buffer, like in a .glb
		result = tex_create_mem((void*)((uint8_t*)image->buffer_view->buffer->data + image->buffer_view->offset), image->buffer_view->size, srgb_data);
		if (result == nullptr) 
			log_warnf("Couldn't load %s texture for %s!", image->name, filename);
		else
			tex_set_id(result, id);
	} else if (image->uri != nullptr && strncmp(image->uri, "data:", 5) == 0) {
		// If it's an image file encoded in a base64 string
		void         *buffer = nullptr;
		cgltf_options options = {};
		
		char*  start = strchr(image->uri, ',') + 1; // start of base64 data
		char*  end   = strchr(image->uri, '=');     // end of base64 data
		size_t size = ((end-start) * 6) / 8;        // find the size of the data in bytes, there's 6 bits of data encoded in 8 bits of base64
		cgltf_load_buffer_base64(&options, size, start, &buffer);

		if (buffer != nullptr) {
			result = tex_create_mem(buffer, size, srgb_data);
			tex_set_id(result, id);
			free(buffer);
		}
	} else if (image->uri != nullptr && strstr(image->uri, "://") == nullptr) {
		// If it's a file path to an external image file
		result = tex_create_file(id, srgb_data);
	}
	return result;
}

///////////////////////////////////////////

material_t gltf_parsematerial(cgltf_data *data, cgltf_material *material, const char *filename, shader_t shader) {
	// Check if we've already loaded this material
	const char *mat_name = material == nullptr ? "null" : material->name;
	char id[512];
	sprintf_s(id, 512, "%s/%s", filename, mat_name);
	material_t result = material_find(id);
	if (result != nullptr) {
		return result;
	}

	result = shader == nullptr ? material_copy_id("default/material") : material_create(shader);
	material_set_id(result, id);

	// If it's a null material, we can just stop here
	if (material == nullptr)
		return result;

	cgltf_texture *tex = nullptr;
	if (material->has_pbr_metallic_roughness) {
		tex = material->pbr_metallic_roughness.base_color_texture.texture;
		if (tex != nullptr)
			material_set_texture(result, "diffuse", gltf_parsetexture(data, tex->image, filename, true));

		tex = material->pbr_metallic_roughness.metallic_roughness_texture.texture;
		if (tex != nullptr)
			material_set_texture(result, "metal", gltf_parsetexture(data, tex->image, filename, false));

		float *c = material->pbr_metallic_roughness.base_color_factor;
		material_set_color(result, "color", { c[0], c[1], c[2], c[3] });
		
		material_set_float(result, "metallic",  material->pbr_metallic_roughness.metallic_factor);
		material_set_float(result, "roughness", material->pbr_metallic_roughness.roughness_factor);
	}
	if (material->double_sided)
		material_set_cull(result, cull_none);
	if (material->alpha_mode == cgltf_alpha_mode_blend)
		material_set_transparency(result, transparency_blend);
	if (material->alpha_mode == cgltf_alpha_mode_mask)
		material_set_transparency(result, transparency_clip);

	tex = material->normal_texture.texture;
	if (tex != nullptr)
		material_set_texture(result, "normal", gltf_parsetexture(data, tex->image, filename, false));

	tex = material->occlusion_texture.texture;
	if (tex != nullptr)
		material_set_texture(result, "occlusion", gltf_parsetexture(data, tex->image, filename, false));

	tex = material->emissive_texture.texture;
	if (tex != nullptr)
		material_set_texture(result, "emission", gltf_parsetexture(data, tex->image, filename, true));

	return result;
}

///////////////////////////////////////////

void gltf_build_node_matrix(cgltf_node *curr, matrix &result) {
	if (curr->parent != nullptr) {
		gltf_build_node_matrix(curr->parent, result);
	}
	matrix mat;
	if (!curr->has_matrix) {
		vec3   pos   = curr->has_translation ? vec3{ curr->translation[0], curr->translation[1], curr->translation[2] } : vec3_zero;
		vec3   scale = curr->has_scale       ? vec3{ curr->scale      [0], curr->scale      [1], curr->scale      [2] } : vec3_one;
		quat   rot   = curr->has_rotation    ? quat{ curr->rotation   [0], curr->rotation   [1], curr->rotation   [2], curr->rotation[3] } : quat_identity;
		mat = matrix_trs(pos, rot, scale);
	} else {
		memcpy(&mat, curr->matrix, sizeof(matrix));
	}
	matrix_mul(mat, result, result);
}

///////////////////////////////////////////

bool modelfmt_gltf(model_t model, const char *filename, void *file_data, size_t file_size, shader_t shader) {
	cgltf_options options = {};
	cgltf_data*   data    = NULL;
	const char *model_file = assets_file(filename);
	if (cgltf_parse(&options, file_data, file_size, &data) != cgltf_result_success)
		return false;
	if (cgltf_load_buffers(&options, data, model_file) != cgltf_result_success) {
		cgltf_free(data);
		return true;
	}

	// GLTF uses a right-handed system, but it also defines +Z as forward. Here, we 
	// rotate the gltf matrices so that they use -Z as forward, simplifying lookat math
	matrix orientation_correction = matrix_trs(vec3_zero, quat_from_angles(0, 180, 0));

	// Load each subset
	for (int32_t i = 0; i < data->nodes_count; i++) {
		cgltf_node *n = &data->nodes[i];
		if (n->mesh == nullptr)
			continue;

		matrix transform = matrix_identity;
		gltf_build_node_matrix(n, transform);
		matrix     offset   = transform * orientation_correction;
		mesh_t     mesh     = gltf_parsemesh    (n->mesh, i, filename);
		material_t material = gltf_parsematerial(data, n->mesh->primitives[0].material, filename, shader);

		model_add_subset(model, mesh, material, offset);

		mesh_release    (mesh);
		material_release(material);
	}
	cgltf_free(data);
	return true;
}

} // namespace sk