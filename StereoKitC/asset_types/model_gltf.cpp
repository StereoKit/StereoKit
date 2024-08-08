// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2024 Nick Klingensmith
// Copyright (c) 2024 Qualcomm Technologies, Inc.

#define _CRT_SECURE_NO_WARNINGS 1

#include "model.h"
#include "mesh_.h"
#include "texture_.h"
#include "../sk_math.h"
#include "../sk_memory.h"
#include "../systems/defaults.h"
#include "../libraries/ferr_hash.h"
#include "../libraries/stref.h"
#include "../platforms/platform.h"
#include "../libraries/cgltf.h"

#include <meshoptimizer.h>

#include <stdio.h>

namespace sk {

// GLTF spec can be found here:
// https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html

// GLTF uses a right-handed system, but it also defines +Z as forward. Here, we 
// rotate the gltf matrices so that they use -Z as forward, simplifying lookat math
matrix gltf_orientation_correction = matrix_trs(vec3_zero, quat_from_angles(0, 180, 0));

///////////////////////////////////////////

matrix gltf_build_node_matrix (cgltf_node *curr);
matrix gltf_build_world_matrix(cgltf_node *curr, cgltf_node *root);
void   gltf_add_warning       (array_t<const char *> *warnings, const char *text);

// This needs to be in cgltf.cpp due to the location of the json parser
void gltf_parse_extras(model_t model, model_node_id node, const char* extras_json, size_t extras_size);

///////////////////////////////////////////

void gltf_add_warning(array_t<const char *> *warnings, const char *text) {
	for (int32_t i = 0; i < warnings->count; i++) {
		if (warnings->data[i] == text)
			return;
	}
	warnings->add(text);
}

///////////////////////////////////////////

bool gltf_parseskin(mesh_t sk_mesh, cgltf_node *node, int primitive_id, const char *filename) {
	if (node->skin == nullptr)
		return false;
	
	cgltf_mesh      *m = node->mesh;
	cgltf_primitive *p = &m->primitives[primitive_id];

	uint16_t *bone_ids   = nullptr;
	int32_t   bone_id_ct = 0;
	vec4     *weights    = nullptr;
	int32_t   weight_ct  = 0;
	matrix   *bone_trs   = nullptr;
	int32_t   bone_tr_ct = 0;

	// Load the mesh's joint bindings and weights
	for (size_t a = 0; a < p->attributes_count; a++) {
		const cgltf_attribute*attr           = &p->attributes[a];
		const cgltf_accessor* accessor       =  p->attributes[a].data;
		const uint8_t*        source_buff    = cgltf_buffer_view_data(accessor->buffer_view) + accessor->offset;
		const cgltf_size      component_num  = cgltf_num_components  (accessor->type);
		const cgltf_size      component_size = cgltf_component_size  (accessor->component_type);

		if (attr->type == cgltf_attribute_type_joints && attr->index == 0) {
			bone_id_ct = (int32_t)accessor->count;
			bone_ids   = sk_malloc_t(uint16_t, (size_t)bone_id_ct * 4);
			memset(bone_ids, 0, sizeof(uint16_t) * bone_id_ct * 4);

			if (accessor->is_sparse) {
				log_errf("[%s] Sparse joints not implemented", filename);
			} else if (accessor->component_type == cgltf_component_type_r_16u) {
				cgltf_size com_bytes = component_num * component_size;
				for (cgltf_size i = 0; i < accessor->count; i++)
					memcpy(&bone_ids[i * 4], &source_buff[i * accessor->stride], com_bytes);
			} else if (accessor->component_type == cgltf_component_type_r_8u) {
				for (cgltf_size i = 0; i < accessor->count; i++) {
					uint8_t*  source = (uint8_t *)&source_buff[i * accessor->stride];
					uint16_t* dest   =            &bone_ids   [i * 4];

					for (int32_t c = 0; c < component_num; c++) dest[c] = source[c];
				}
			} else if (accessor->component_type == cgltf_component_type_r_32u){
				for (cgltf_size i = 0; i < accessor->count; i++) {
					uint32_t* source = (uint32_t*)&source_buff[i * accessor->stride];
					uint16_t* dest   =            &bone_ids   [i * 4];

					for (int32_t c = 0; c < component_num; c++) dest[c] = (uint16_t)source[c];
				}
			} else {
				log_errf("[%s] joint format (%d) not implemented", filename, accessor->component_type);
				sk_free(bone_ids);
				bone_ids = nullptr;
			}
		} else if (attr->type == cgltf_attribute_type_weights && attr->index == 0) {
			size_t       count  = cgltf_accessor_unpack_floats(accessor, nullptr, 0);
			cgltf_float *floats = sk_malloc_t(cgltf_float, count);
			cgltf_accessor_unpack_floats(attr->data, floats, count);

			weight_ct = (int32_t)attr->data->count;
			weights   = sk_malloc_t(vec4, weight_ct);
			memset(weights, 0, sizeof(vec4) *weight_ct);

			if (component_num == 1) for (int32_t j = 0; j < weight_ct; j++) {
				weights[j].x = 1; // one weight? Must be 1
			} else if (component_num == 2) for (int32_t j = 0; j < weight_ct; j++) {
				cgltf_size jc = j * component_num;
				vec4   *w  = &weights[j];
				w->x = floats[jc];
				w->y = floats[jc+1];
				float sum = 1/(w->x + w->y);
				w->x = w->x * sum;
				w->y = w->y * sum;
			} else if (component_num == 3) for (int32_t j = 0; j < weight_ct; j++) {
				cgltf_size jc = j * component_num;
				vec4   *w  = &weights[j];
				w->x = floats[jc];
				w->y = floats[jc+1];
				w->z = floats[jc+2];
				float sum = 1/(w->x + w->y + w->z);
				w->x = w->x * sum;
				w->y = w->y * sum;
				w->z = w->z * sum;
			}
			if (component_num == 4) {
				free(weights);
				weights = (vec4*)floats;
				for (int32_t j = 0; j < weight_ct; j++) {
					vec4 *w   = &weights[j];
					float sum = 1/(w->x + w->y + w->z + w->w);
					w->x = w->x * sum;
					w->y = w->y * sum;
					w->z = w->z * sum;
					w->w = w->w * sum;
				}
			} else {
				sk_free(floats);
			}
		}
	}

	if (!bone_ids || !weights) {
		log_errf("[%s] mesh skin incomplete", filename);
		sk_free(bone_ids);
		sk_free(weights);
		return false;
	}

	// Find the skeleton for the mesh
	bone_tr_ct = (int32_t)node->skin->joints_count;
	bone_trs   = sk_malloc_t(matrix, bone_tr_ct);
	if (node->skin->inverse_bind_matrices != nullptr) { 
		cgltf_buffer_view *buff      = node->skin->inverse_bind_matrices->buffer_view;
		size_t             offset    = buff->offset + node->skin->inverse_bind_matrices->offset;
		uint8_t           *attr_data = ((uint8_t *)buff->buffer->data) + offset;

		memcpy(bone_trs, attr_data, sizeof(matrix) *bone_tr_ct);
		for (int32_t i = 0; i < bone_tr_ct; i++) {
			bone_trs[i] = matrix_invert(bone_trs[i]);
		}
	} else {
		for (int32_t i = 0; i < bone_tr_ct; i++) {
			bone_trs[i] = gltf_build_world_matrix(node->skin->joints[i], node->skin->skeleton ? node->skin->skeleton : node);
		}
	}

	// And assign the skin!
	mesh_set_skin(sk_mesh, bone_ids, bone_id_ct, weights, weight_ct, bone_trs, bone_tr_ct);

	sk_free(bone_ids);
	sk_free(weights);
	sk_free(bone_trs);

	return true;
}

///////////////////////////////////////////

bool gltf_material_is_lightmap(cgltf_material *mat) {
	return
		mat != nullptr &&
		mat->emissive_texture .texture != nullptr &&
		mat->occlusion_texture.texture != nullptr &&
		mat->occlusion_texture.texcoord == 1 &&
		mat->pbr_metallic_roughness.base_color_texture.texture == nullptr &&
		mat->pbr_metallic_roughness.metallic_roughness_texture.texture == nullptr;
}

///////////////////////////////////////////

void gltf_meshopt_decode(cgltf_data* gltf_data) {
	for (cgltf_size i = 0; i < gltf_data->buffer_views_count; i++) {
		if (gltf_data->buffer_views[i].has_meshopt_compression == false) continue;
		cgltf_meshopt_compression* comp = &gltf_data->buffer_views[i].meshopt_compression;

		const uint8_t* meshopt_buff = (const uint8_t*)comp->buffer->data + comp->offset;
		uint8_t*       data         = sk_malloc_t(uint8_t, comp->stride * comp->count);

		int32_t result = 0;
		switch (comp->mode) {
		case cgltf_meshopt_compression_mode_attributes: result = meshopt_decodeVertexBuffer (data, comp->count, comp->stride, meshopt_buff, comp->size); break;
		case cgltf_meshopt_compression_mode_triangles:  result = meshopt_decodeIndexBuffer  (data, comp->count, comp->stride, meshopt_buff, comp->size); break;
		case cgltf_meshopt_compression_mode_indices:    result = meshopt_decodeIndexSequence(data, comp->count, comp->stride, meshopt_buff, comp->size); break;
		default: result = -1; break;
		}

		if (result != 0) { log_warnf("Meshopt decode failed: %d", result); sk_free(data); continue; }

		switch (comp->filter) {
		case cgltf_meshopt_compression_filter_none: break;
		case cgltf_meshopt_compression_filter_octahedral:  meshopt_decodeFilterOct (data, comp->count, comp->stride); break;
		case cgltf_meshopt_compression_filter_quaternion:  meshopt_decodeFilterQuat(data, comp->count, comp->stride); break;
		case cgltf_meshopt_compression_filter_exponential: meshopt_decodeFilterExp (data, comp->count, comp->stride); break;
		default: sk_free(data); continue;
		}

		gltf_data->buffer_views[i].data   = data;
		gltf_data->buffer_views[i].offset = 0;
	}
}

///////////////////////////////////////////

void gltf_view_to_vert_f(void* destination_buffer, size_t dest_step, size_t vert_offset, cgltf_accessor *accessor) {
	uint8_t*       destination = ((uint8_t*)destination_buffer) + vert_offset;
	const uint8_t* source_buff = cgltf_buffer_view_data(accessor->buffer_view) + accessor->offset;

	cgltf_size component_num  = cgltf_num_components(accessor->type);
	cgltf_size component_size = cgltf_component_size(accessor->component_type);
	bool standard_path = accessor->is_sparse ||
		(accessor->type == cgltf_type_mat3 && component_size == 2) ||
		(accessor->type == cgltf_type_mat3 && component_size == 1) ||
		(accessor->type == cgltf_type_mat2 && component_size == 1);

	// Here we provide a number of fast paths that also avoid a memory allocation
	if (standard_path == false && accessor->component_type == cgltf_component_type_r_32f) {
		cgltf_size com_bytes = component_num * component_size;
		for (cgltf_size i = 0; i < accessor->count; i++)
			memcpy(&destination[i * dest_step], &source_buff[i * accessor->stride], com_bytes);

	} else if (standard_path == false && accessor->component_type == cgltf_component_type_r_16) {
		for (cgltf_size i = 0; i < accessor->count; i++) {
			int16_t* source = (int16_t*)&source_buff[i * accessor->stride];
			float*   dest   = (float  *)&destination[i * dest_step];

			if (accessor->normalized) for (int32_t c = 0; c < component_num; c++) dest[c] = (float)source[c] / (float)INT16_MAX;
			else                      for (int32_t c = 0; c < component_num; c++) dest[c] = (float)source[c];
		}
	} else if (standard_path == false && accessor->component_type == cgltf_component_type_r_16u) {
		for (cgltf_size i = 0; i < accessor->count; i++) {
			uint16_t* source = (uint16_t*)&source_buff[i * accessor->stride];
			float*    dest   = (float   *)&destination[i * dest_step];

			if (accessor->normalized) for (int32_t c = 0; c < component_num; c++) dest[c] = (float)source[c] / (float)UINT16_MAX;
			else                      for (int32_t c = 0; c < component_num; c++) dest[c] = (float)source[c];
		}
	} else if (standard_path == false && accessor->component_type == cgltf_component_type_r_8) {
		for (cgltf_size i = 0; i < accessor->count; i++) {
			int8_t* source = (int8_t*)&source_buff[i * accessor->stride];
			float*  dest   = (float *)&destination[i * dest_step];

			if (accessor->normalized) for (int32_t c = 0; c < component_num; c++) dest[c] = (float)source[c] / (float)INT8_MAX;
			else                      for (int32_t c = 0; c < component_num; c++) dest[c] = (float)source[c];
		}
	} else if (standard_path == false && accessor->component_type == cgltf_component_type_r_8u) {
		for (cgltf_size i = 0; i < accessor->count; i++) {
			uint8_t* source = (uint8_t*)&source_buff[i * accessor->stride];
			float*   dest   = (float  *)&destination[i * dest_step];

			if (accessor->normalized) for (int32_t c = 0; c < component_num; c++) dest[c] = (float)source[c] / (float)UINT8_MAX;
			else                      for (int32_t c = 0; c < component_num; c++) dest[c] = (float)source[c];
		}
	} else {
		// For everything else, we'll use cgltf to convert to floats, and use that
		size_t       count  = cgltf_accessor_unpack_floats(accessor, nullptr, 0);
		cgltf_float *floats = sk_malloc_t(cgltf_float, count);
		cgltf_accessor_unpack_floats(accessor, floats, count);

		cgltf_size com_bytes = component_num * component_size;
		for (cgltf_size i = 0; i < accessor->count; i++)
			memcpy(&destination[i * dest_step], &floats[i * component_num], com_bytes);
		sk_free(floats);
	}
}

///////////////////////////////////////////

mesh_t gltf_parsemesh(cgltf_mesh *mesh, int node_id, int primitive_id, const char *filename, array_t<const char *> *warnings) {
	cgltf_mesh      *m = mesh;
	cgltf_primitive *p = &m->primitives[primitive_id];

	if (p->type != cgltf_primitive_type_triangles) {
		log_errf("[%s] Unimplemented GLTF primitive mode: %d", filename, p->type);
		return nullptr;
	}
	if (p->has_draco_mesh_compression) {
		gltf_add_warning(warnings, "GLTF Draco Mesh Compression not currently supported");
		return nullptr;
	}

	char id[512];
	snprintf(id, sizeof(id), "%s/mesh/%d_%d_%s", filename, node_id, primitive_id, m->name);
	mesh_t result = mesh_find(id);
	if (result != nullptr) {
		return result;
	}

	vert_t *verts = nullptr;
	int32_t vert_count = 0;

	bool has_normals      = false;
	bool has_lightmap_uvs = false;
	for (size_t a = 0; a < p->attributes_count; a++) {
		cgltf_attribute* attr = &p->attributes[a];
		const uint8_t*   buff = cgltf_buffer_view_data(attr->data->buffer_view) + attr->data->offset;
		
		// Make sure we have memory for our verts
		if (vert_count < (int32_t)attr->data->count) {
			vert_count = (int32_t)attr->data->count;
			verts      = sk_realloc_t(vert_t, verts, vert_count);
			for (int32_t i = 0; i < vert_count; i++) {
				verts[i] = vert_t{ vec3_zero, vec3_zero, vec2_zero, {255,255,255,255} };
			}
		}

		// Check what info is in this attribute, and copy it over to our mesh
		if (attr->type == cgltf_attribute_type_position) {
			if (attr->index != 0) {
				gltf_add_warning(warnings, "Too many vertex position channels! Only one supported, the rest will be ignored.");
			} else gltf_view_to_vert_f(verts, sizeof(vert_t), offsetof(vert_t, pos), attr->data);
		} else if (attr->type == cgltf_attribute_type_normal && has_lightmap_uvs == false) {
			has_normals = true;
			if (attr->index != 0) {
				gltf_add_warning(warnings, "Too many vertex normal channels! Only one supported, the rest will be ignored.");
			} else gltf_view_to_vert_f(verts, sizeof(vert_t), offsetof(vert_t, norm), attr->data);
		} else if (attr->type == cgltf_attribute_type_texcoord) {
			if (attr->index == 1 && gltf_material_is_lightmap(p->material)) {
				// If this is lightmapped, we don't need normals and can just pack our data in there.
				gltf_view_to_vert_f(verts, sizeof(vert_t), offsetof(vert_t, norm), attr->data);
				has_lightmap_uvs = true;
			} else if (attr->index != 0) {
				gltf_add_warning(warnings, "Too many texture coordinate channels! Only one supported, the rest will be ignored.");
			} else gltf_view_to_vert_f(verts, sizeof(vert_t), offsetof(vert_t, uv), attr->data);
		} else if (attr->type == cgltf_attribute_type_color) {
			if (attr->index != 0) {
				gltf_add_warning(warnings, "Too many vertex color channels! Only one supported, the rest will be ignored.");
			} else if (!attr->data->is_sparse && attr->data->component_type == cgltf_component_type_r_8u && attr->data->type == cgltf_type_vec4) {
				// Ideal case is vec4 uint8_t colors
				for (size_t v = 0; v < attr->data->count; v++) {
					color32 *col = (color32 *)(buff + (attr->data->stride * v));
					verts[v].col = *col;
				}
			} else if (!attr->data->is_sparse && attr->data->component_type == cgltf_component_type_r_32f && attr->data->type == cgltf_type_vec4) {
				// vec4 float colors are also pretty straightforward
				for (size_t v = 0; v < attr->data->count; v++) {
					color128 *col = (color128 *)(buff + (attr->data->stride * v));
					verts[v].col = color_to_32(*col);
				}
			} else {
				// For everything else, we'll just load them as floats, and
				// then convert down to color32
				size_t       count  = cgltf_accessor_unpack_floats(attr->data, nullptr, 0);
				cgltf_float *floats = sk_malloc_t(cgltf_float, count);
				cgltf_accessor_unpack_floats(attr->data, floats, count);

				if (attr->data->type == cgltf_type_vec4) {
					for (size_t v = 0; v < attr->data->count; v++) {
						color128 *col = (color128 *)&floats[v * 4];
						verts[v].col = color_to_32(*col);
					}
				} else if (attr->data->type == cgltf_type_vec3) {
					for (size_t v = 0; v < attr->data->count; v++) {
						cgltf_float *col = (cgltf_float *)&floats[v * 3];
						verts[v].col = color_to_32({ col[0], col[1], col[2], 1 });
					}
				} else {
					log_errf("[%s] Unimplemented vertex color type (%d)", filename, attr->data->type);
				}
				sk_free(floats);
			}
		}
	}

	// Now grab the mesh indices
	
	size_t  ind_count = 0;
	vind_t *inds      = nullptr;
	if (p->indices == nullptr) {
		// No indices listed, create indices that map to one index per vertex
		ind_count = vert_count;
		inds      = sk_malloc_t(vind_t, ind_count);
		for (size_t i = 0; i < ind_count; i++) {
			inds[i] = (vind_t)i;
		}
	} else {
		// Extract indices from the index buffer
		ind_count = p->indices->count;
		inds      = sk_malloc_t(vind_t, ind_count);

		const uint8_t* buff = cgltf_buffer_view_data(p->indices->buffer_view) + p->indices->offset;
		if (!p->indices->is_sparse && p->indices->component_type == cgltf_component_type_r_8u) {
			for (size_t v = 0; v < ind_count; v++) {
				uint8_t *ind = (uint8_t *)(buff + (p->indices->stride * v));
				inds[v] = *ind;
			}
		} else if (!p->indices->is_sparse && p->indices->component_type == cgltf_component_type_r_16u) {
			for (size_t v = 0; v < ind_count; v++) {
				uint16_t *ind = (uint16_t *)(buff + (p->indices->stride * v));
				inds[v] = *ind;
			}
		} else if (!p->indices->is_sparse && p->indices->component_type == cgltf_component_type_r_32u) {
			if (p->indices->stride == sizeof(uint32_t)) {
				// Fast path!
				memcpy(inds, buff, ind_count * sizeof(uint32_t));
			} else {
				for (size_t v = 0; v < ind_count; v++) {
					uint32_t *ind = (uint32_t *)(buff + (p->indices->stride * v));
					inds[v] = *ind;
				}
			}
		} else {
			gltf_add_warning(warnings, "Unimplemented vertex index format");
		}
	}

	if (!has_normals) {
		mesh_calculate_normals(verts, vert_count, inds, (int32_t)ind_count);
	}

	/*uint32_t* remap = sk_malloc_t(uint32_t, vert_count);
	size_t    final_vertex_count = meshopt_generateVertexRemap(remap, inds, ind_count, verts, vert_count, sizeof(vert_t));
	vert_t*   final_verts        = sk_malloc_t(vert_t, final_vertex_count);

	meshopt_remapIndexBuffer   (inds, inds, ind_count, &remap[0]);
	meshopt_remapVertexBuffer  (final_verts, &verts[0], vert_count, sizeof(vert_t), &remap[0]);
	meshopt_optimizeVertexCache(inds, inds, ind_count, final_vertex_count);
	meshopt_optimizeOverdraw   (inds, inds, ind_count, &final_verts[0].pos.x, final_vertex_count, sizeof(vert_t), 1.05f);
	meshopt_optimizeVertexFetch(final_verts, inds, ind_count, final_verts,    final_vertex_count, sizeof(vert_t));
	
	result = mesh_create();
	mesh_set_data(result, final_verts, final_vertex_count, inds, (int32_t)ind_count);

	sk_free(final_verts);
	sk_free(remap);
	*/

	result = mesh_create();
	mesh_set_data(result, verts, vert_count, inds, (int32_t)ind_count);
	mesh_set_id  (result, id);
	sk_free(verts);
	sk_free(inds );

	return result;
}

///////////////////////////////////////////

void gltf_imagename(cgltf_data *data, cgltf_image *image, const char *filename, char *dest, int dest_length) {
	if (image->uri != nullptr && strncmp(image->uri, "data:", 5) != 0 && strstr(image->uri, "://") == nullptr) {
		char *last1 = strrchr((char *)filename, '/');
		char *last2 = strrchr((char *)filename, '\\');
		char *last  = (char*)maxi((uint64_t)last1, (uint64_t)last2);
		if (last == nullptr) {
			snprintf(dest, dest_length, "%s", image->uri);
		} else {
			snprintf(dest, dest_length, "%.*s/%s", (int)(last - filename), filename, image->uri);
		}
		return;
	}

	for (size_t i = 0; i < data->images_count; i++) {
		if (&data->images[i] == image) {
			snprintf(dest, dest_length, "%s/tex/%d", filename, (int)i);
			return;
		}
	}

	snprintf(dest, dest_length, "%s/unknown_image", filename);
}


///////////////////////////////////////////

void gltf_apply_sampler(tex_t to_tex, cgltf_sampler *sampler) {
	if (sampler == nullptr) return;

	tex_sample_ sample = tex_get_sample(to_tex);
	switch (sampler->mag_filter) {
	case 9728: // NEAREST
	case 9984: // NEAREST_MIPMAP_NEAREST
	case 9986: // NEAREST_MIPMAP_LINEAR
		sample = tex_sample_point;
		break;
	default:
		sample = tex_sample_linear;
		break;
	}

	tex_address_ address = tex_get_address(to_tex);
	switch (sampler->wrap_s) {
	case 33071: address = tex_address_clamp;  break;
	case 33648: address = tex_address_mirror; break;
	case 10497: address = tex_address_wrap;   break;
	}
	tex_set_options(to_tex, sample, address);
}

///////////////////////////////////////////

tex_t gltf_parsetexture(cgltf_data* data, cgltf_texture *tex, const char *filename, bool srgb_data, int32_t priority, array_t<const char*>* warnings) {
	cgltf_image *image = tex->has_basisu
		? tex->basisu_image
		: tex->image;
	if (image == nullptr) {
		return nullptr;
	}

	// Check if we've already loaded this image
	char id[512];
	gltf_imagename(data, image, filename, id, 512);
	tex_t result = tex_find(id);
	if (result != nullptr) {
		return result;
	}

	if (image->buffer_view != nullptr) {
		// If it's already a loaded buffer, like in a .glb
		result = tex_create_mem((void*)((uint8_t*)image->buffer_view->buffer->data + image->buffer_view->offset), image->buffer_view->size, srgb_data, priority);
		if (result == nullptr) 
			log_warnf("[%s] Couldn't load texture: %s", filename, image->name);
		else
			tex_set_id(result, id);
	} else if (image->uri != nullptr && strncmp(image->uri, "data:", 5) == 0) {
		// If it's an image file encoded in a base64 string
		char* start = strchr(image->uri, ',');
		if (start != nullptr && start - image->uri >= 7 && strncmp(start - 7, ";base64", 7) == 0) {
			void*         buffer  = nullptr;
			cgltf_options options = {};

			char*  base64_start = start + 1;
			size_t base64_len   = strlen(base64_start);

			// A base64 string may end with 0, 1 or 2 '=' padding characters,
			// padding is present to ensure data is a multiple of 3.
			size_t base64_size = 3 * (base64_len / 4);
			if (base64_len >= 1 && base64_start[base64_len-1] == '=') { base64_size -= 1; }
			if (base64_len >= 2 && base64_start[base64_len-2] == '=') { base64_size -= 1; }

			 // find the size of the data in bytes, there's 6 bits of data encoded in 8 bits of base64
			cgltf_load_buffer_base64(&options, base64_size, base64_start, &buffer);

			if (buffer != nullptr) {
				result = tex_create_mem(buffer, base64_size, srgb_data, priority);
				tex_set_id(result, id);
				sk_free(buffer);
			}
		}
	} else if (image->uri != nullptr && strstr(image->uri, "://") == nullptr) {
		// If it's a file path to an external image file
		result = tex_create_file(id, srgb_data, priority);
	}
	if (result != nullptr)
		gltf_apply_sampler(result, tex->sampler);

	return result;
}

///////////////////////////////////////////

void gltf_set_material_transform(material_t material, const cgltf_texture_view *view) {
	if (view->has_transform == false || material_has_param(material, "tex_trans", material_param_vector4) == false)
		return;

	material_set_vector4(material, "tex_trans", {
		view->transform.offset[0],
		view->transform.offset[1],
		view->transform.scale[0],
		view->transform.scale[1] });
}

///////////////////////////////////////////

material_t gltf_parsematerial(cgltf_data *data, cgltf_material *material, const char *filename, shader_t shader, array_t<const char*> *warnings) {
	// Check if we've already loaded this material
	char id[512];
	if (material == nullptr)
		snprintf(id, sizeof(id), "%s/mat/null", filename);
	else if (material->name == nullptr)
		snprintf(id, sizeof(id), "%s/mat/%u", filename, hash_fnv32_data(material, sizeof(cgltf_material)));
	else
		snprintf(id, sizeof(id), "%s/mat/%s", filename, material->name);
	material_t result = material_find(id);
	if (result != nullptr) {
		return result;
	}

	// Use the shader that was provided, or pick a shader based on the 
	// material's attributes.
	bool is_lightmap = gltf_material_is_lightmap(material);
	if (shader != nullptr) {
		result = material_create(shader);
	} else {
		if (material == nullptr) {
			result = material_copy_id(default_id_material);
		} else if (is_lightmap) {
			result = material_create(shader_find(default_id_shader_lightmap));
		} else if (material->unlit) {
			if (material->alpha_mode == cgltf_alpha_mode_mask)
				result = material_copy_id(default_id_material_unlit_clip);
			else
				result = material_copy_id(default_id_material_unlit);
		} else if (material->has_pbr_metallic_roughness) {
			if (material->alpha_mode == cgltf_alpha_mode_mask)
				result = material_copy_id(default_id_material_pbr_clip);
			else
				result = material_copy_id(default_id_material_pbr);
		} else {
			result = material_copy_id(default_id_material);
		}
	}

	// If we failed to create a material, we can just stop here
	if (result == nullptr)
		return nullptr;

	material_set_id(result, id);

	// If we don't actually have material information, ditch out here using
	// the base material's defaults
	if (material == nullptr)
		return result;

	cgltf_texture *tex = nullptr;
	if (material->has_pbr_metallic_roughness) {
		tex = material->pbr_metallic_roughness.base_color_texture.texture;
		if (tex != nullptr && material_has_param(result, "diffuse", material_param_texture)) {
			if (material->pbr_metallic_roughness.base_color_texture.texcoord != 0) gltf_add_warning(warnings, "StereoKit doesn't support loading multiple texture coordinate channels yet.");
			gltf_set_material_transform(result, &material->pbr_metallic_roughness.base_color_texture);
			tex_t parse_tex = gltf_parsetexture(data, tex, filename, true, 10, warnings);
			if (parse_tex != nullptr) {
				material_set_texture(result, "diffuse", parse_tex);
				tex_release(parse_tex);
			}
		}

		tex = material->pbr_metallic_roughness.metallic_roughness_texture.texture;
		if (tex != nullptr && material_has_param(result, "metal", material_param_texture)) {
			if (material->pbr_metallic_roughness.metallic_roughness_texture.texcoord != 0) gltf_add_warning(warnings, "StereoKit doesn't support loading multiple texture coordinate channels yet.");
			tex_t parse_tex = gltf_parsetexture(data, tex, filename, false, 13, warnings);
			if (parse_tex != nullptr) {
				tex_set_fallback(parse_tex, sk_default_tex_rough);
				material_set_texture(result, "metal", parse_tex);
				tex_release(parse_tex);
			}
		}

		float *c = material->pbr_metallic_roughness.base_color_factor;
		if (material_has_param(result, "color", material_param_color128))
			material_set_color(result, "color", color_to_gamma({ c[0], c[1], c[2], c[3] }));

		if (material_has_param(result, "metallic",  material_param_float))
			material_set_float(result, "metallic",  material->pbr_metallic_roughness.metallic_factor);
		if (material_has_param(result, "roughness", material_param_float))
			material_set_float(result, "roughness", material->pbr_metallic_roughness.roughness_factor);
	} else if (material->has_pbr_specular_glossiness) {
		gltf_add_warning(warnings, "StereoKit doesn't fully support specular/glossy PBR yet.");

		tex = material->pbr_specular_glossiness.diffuse_texture.texture;
		if (tex != nullptr && material_has_param(result, "diffuse", material_param_texture)) {
			if (material->pbr_specular_glossiness.diffuse_texture.texcoord != 0) gltf_add_warning(warnings, "StereoKit doesn't support multiple texture coordinate channels yet.");
			gltf_set_material_transform(result, &material->pbr_specular_glossiness.diffuse_texture);
			tex_t parse_tex = gltf_parsetexture(data, tex, filename, true, 10, warnings);
			if (parse_tex != nullptr) {
				material_set_texture(result, "diffuse", parse_tex);
				tex_release(parse_tex);
			}
		}

		float *c = material->pbr_specular_glossiness.diffuse_factor;
		if (material_has_param(result, "color", material_param_color128))
			material_set_color(result, "color", color_to_gamma({ c[0], c[1], c[2], c[3] }));
	}
	if (material->double_sided)
		material_set_cull(result, cull_none);
	if (material->alpha_mode == cgltf_alpha_mode_blend)
		material_set_transparency(result, transparency_blend);
	if (material->alpha_mode == cgltf_alpha_mode_mask && material_has_param(result, "cutoff", material_param_float))
		material_set_float(result, "cutoff", material->alpha_cutoff);
	if (material_has_param  (result, "emission_factor", material_param_vector4))
		material_set_vector4(result, "emission_factor", { material->emissive_factor[0], material->emissive_factor[1], material->emissive_factor[2], 0 });

	tex = material->normal_texture.texture;
	if (tex != nullptr && material_has_param(result, "normal", material_param_texture)) {
		if (material->normal_texture.texcoord != 0) gltf_add_warning(warnings, "StereoKit doesn't support multiple texture coordinate channels yet.");
		tex_t parse_tex = gltf_parsetexture(data, tex, filename, false, 13, warnings);
		if (parse_tex != nullptr) {
			tex_set_fallback(parse_tex, sk_default_tex_flat);
			material_set_texture(result, "normal", parse_tex);
			tex_release(parse_tex);
		}
	}

	tex = material->occlusion_texture.texture;
	const char* param = is_lightmap ? "lightmap" : "occlusion";
	if (tex != nullptr && material_has_param(result, param, material_param_texture)) {
		if (material->occlusion_texture.texcoord != 0 && is_lightmap == false) gltf_add_warning(warnings, "StereoKit doesn't support multiple texture coordinate channels yet.");
		tex_t parse_tex = gltf_parsetexture(data, tex, filename, is_lightmap ? true : false, 11, warnings);
		if (parse_tex != nullptr) {
			tex_set_fallback(parse_tex, sk_default_tex);
			material_set_texture(result, param, parse_tex);
			tex_release(parse_tex);
		}
	}

	tex = material->emissive_texture.texture;
	param = is_lightmap ? "diffuse" : "emission";
	if (tex != nullptr && material_has_param(result, param, material_param_texture)) {
		if (material->emissive_texture.texcoord != 0) gltf_add_warning(warnings, "StereoKit doesn't support multiple texture coordinate channels yet.");
		gltf_set_material_transform(result, &material->emissive_texture);
		tex_t parse_tex = gltf_parsetexture(data, tex, filename, true, 12, warnings);
		if (parse_tex != nullptr) {
			tex_set_fallback(parse_tex, sk_default_tex_black);
			material_set_texture(result, param, parse_tex);
			tex_release(parse_tex);
		}
	}

	return result;
}

///////////////////////////////////////////

anim_t gltf_parseanim(const cgltf_animation *anim, hashmap_t<cgltf_node*, model_node_id> *node_map) {
	anim_t result = {};
	result.name = anim->name ? string_copy(anim->name) : string_copy("(none)");

	for (size_t c = 0; c < anim->channels_count; c++) {
		cgltf_animation_channel *ch = &anim->channels[c];

		anim_curve_t curve = {};
		curve.node_id = *node_map->get(ch->target_node);

		switch (ch->sampler->interpolation) {
		case cgltf_interpolation_type_linear:       curve.interpolation = anim_interpolation_linear; break;
		case cgltf_interpolation_type_step:         curve.interpolation = anim_interpolation_step;   break;
		case cgltf_interpolation_type_cubic_spline: curve.interpolation = anim_interpolation_cubic;  break;
		}
		switch (ch->target_path) {
		case cgltf_animation_path_type_translation: curve.applies_to = anim_element_translation; break;
		case cgltf_animation_path_type_rotation:    curve.applies_to = anim_element_rotation;    break;
		case cgltf_animation_path_type_scale:       curve.applies_to = anim_element_scale;       break;
		case cgltf_animation_path_type_weights:     curve.applies_to = anim_element_weights;     break;
		case cgltf_animation_path_type_invalid: {
			log_errf("Got invalid animation path type");
		} break;
		}

		size_t output_size    =          cgltf_accessor_unpack_floats(ch->sampler->output, nullptr, 0);
		curve.keyframe_count  = (int32_t)cgltf_accessor_unpack_floats(ch->sampler->input,  nullptr, 0);
		curve.keyframe_times  = sk_malloc_t(float, curve.keyframe_count);
		curve.keyframe_values = sk_malloc(sizeof(float) * output_size);
		cgltf_accessor_unpack_floats(ch->sampler->input,                curve.keyframe_times,  curve.keyframe_count);
		cgltf_accessor_unpack_floats(ch->sampler->output, (cgltf_float*)curve.keyframe_values, output_size);

		if (result.duration < curve.keyframe_times[curve.keyframe_count-1])
			result.duration = curve.keyframe_times[curve.keyframe_count-1];

		// We only need to apply transform corrections to this if it's a root node!
		if (ch->target_node->parent == nullptr) {
			const int32_t skip   = curve.interpolation == anim_interpolation_cubic?3:1;
			const int32_t offset = curve.interpolation == anim_interpolation_cubic?1:0;

			switch (curve.applies_to) {
			case anim_element_translation: {
				vec3 *tr = (vec3*)curve.keyframe_values + offset;
				for (int32_t k = 0; k < curve.keyframe_count; k++)
					tr[k*skip] = matrix_transform_pt(gltf_orientation_correction, tr[k*skip]);
			} break;
			case anim_element_scale: {
				vec3 *sc = (vec3*)curve.keyframe_values + offset;
				for (int32_t k = 0; k < curve.keyframe_count; k++)
					sc[k*skip] = matrix_transform_dir(gltf_orientation_correction, sc[k*skip]);
			} break;
			case anim_element_rotation: {
				quat *rot = (quat*)curve.keyframe_values + offset;
				quat r = matrix_extract_rotation(gltf_orientation_correction);
				for (int32_t k = 0; k < curve.keyframe_count; k++)
					rot[k*skip] = rot[k*skip] * r;
			} break;
			case anim_element_weights: {
				log_warnf("Animated weights unsupported");
			}
			}
		}

		result.curves.add(curve);
	}

	return result;
}

///////////////////////////////////////////

matrix gltf_build_node_matrix(cgltf_node *curr) {
	matrix mat;
	if (!curr->has_matrix) {
		vec3   pos   = curr->has_translation ? vec3{ curr->translation[0], curr->translation[1], curr->translation[2] } : vec3_zero;
		vec3   scale = curr->has_scale       ? vec3{ curr->scale      [0], curr->scale      [1], curr->scale      [2] } : vec3_one;
		quat   rot   = curr->has_rotation    ? quat{ curr->rotation   [0], curr->rotation   [1], curr->rotation   [2], curr->rotation[3] } : quat_identity;
		mat = matrix_trs(pos, rot, scale);
	} else {
		memcpy(&mat, curr->matrix, sizeof(matrix));
	}
	return mat;
}

///////////////////////////////////////////

matrix gltf_build_world_matrix(cgltf_node *curr, cgltf_node *root) {
	return curr->parent == nullptr || curr == root 
		? gltf_build_node_matrix(curr)
		: gltf_build_node_matrix(curr) * gltf_build_world_matrix(curr->parent, root);
}

///////////////////////////////////////////

int32_t gltf_node_index(cgltf_data *data, cgltf_node *node) {
	return (int32_t)(((uint8_t*)node - (uint8_t*)data->nodes)/sizeof(cgltf_node));
}

///////////////////////////////////////////

void gltf_add_node(model_t model, shader_t shader, model_node_id parent, const char *filename, cgltf_data *data, cgltf_node *node, hashmap_t<cgltf_node*, model_node_id> *node_map, array_t<const char *> *warnings) {
	int32_t       index   = (int32_t)(node - data->nodes);
	model_node_id node_id = -1;

	matrix transform = gltf_build_node_matrix(node);
	if (parent == -1)
		transform = transform * gltf_orientation_correction;

	for (cgltf_size p = 0; node->mesh && p < node->mesh->primitives_count; p++) {
		mesh_t mesh = gltf_parsemesh(node->mesh, index, (int)p, filename, warnings);
		if (mesh == nullptr) continue;

		// If we're splitting this node into multiple meshes, then add the
		// additional nodes as children of the first, which should help
		// preserve animations and such.
		model_node_id primitive_parent = parent;
		matrix        node_transform   = transform;
		if (node_id != -1) {
			primitive_parent = node_id;
			node_transform   = matrix_identity;
		}

		material_t    material = gltf_parsematerial(data, node->mesh->primitives[p].material, filename, shader, warnings);
		model_node_id new_node = model_node_add_child(model, primitive_parent, node->name, node_transform, mesh, material);
		if (node->skin) 
			gltf_parseskin(model_node_get_mesh(model, new_node), node, (int)p, filename);
		if (node_id == -1)
			node_id = new_node;

		mesh_release    (mesh);
		material_release(material);
	}

	if (node_id == -1) {
		node_id = model_node_add_child(model, parent, node->name, transform, nullptr, nullptr);
	}
	node_map->set(node, node_id);

	// Copy the GLTF's extras into a dictionary
	if (node->extras.data) {
		gltf_parse_extras(model, node_id, node->extras.data, strlen(node->extras.data));
	}

	for (size_t i = 0; i < node->children_count; i++) {
		gltf_add_node(model, shader, node_id, filename, data, node->children[i], node_map, warnings);
	}
}

///////////////////////////////////////////

bool modelfmt_gltf(model_t model, const char *filename, const void *file_data, size_t file_size, shader_t shader) {
	cgltf_options options = {};
	options.file.read = [](const struct cgltf_memory_options*, const struct cgltf_file_options*, const char* path, cgltf_size* size, void** data) {
		return platform_read_file_direct(path, data, size)
			? cgltf_result_success
			: cgltf_result_file_not_found;
	};
	options.file.release = [](const struct cgltf_memory_options*, const struct cgltf_file_options*, void* data) {
		sk_free(data);
	};
	options.memory.alloc_func = [](void *, cgltf_size size) { return sk_malloc(size); };
	options.memory.free_func  = [](void *, void*      data) { sk_free(data); };

	cgltf_data*  data   = nullptr;
	cgltf_result result = cgltf_parse(&options, file_data, file_size, &data);
	if (result != cgltf_result_success) {
		log_diagf("[%s] gltf parse err %d", filename, result);
		cgltf_free(data);
		return false;
	}

	char* model_file = assets_file(filename);
	result = cgltf_load_buffers(&options, data, model_file);
	sk_free(model_file);
	if (result != cgltf_result_success) {
		log_diagf("[%s] gltf buffer load err %d", filename, result);
		cgltf_free(data);
		return false;
	}

	array_t<const char *> warnings = {};

	// Decompress any meshopt data
	gltf_meshopt_decode(data);

	// Load each root node
	hashmap_t<cgltf_node*, model_node_id> node_map = {};
	for (cgltf_size i = 0; i < data->nodes_count; i++) {
		cgltf_node *n = &data->nodes[i];
		if (n->parent == nullptr)
			gltf_add_node(model, shader, -1, filename, data, n, &node_map, &warnings);
	}

	// Load each animation
	for (cgltf_size i = 0; i < data->animations_count; i++) {
		model->anim_data.anims.add( gltf_parseanim(&data->animations[i], &node_map) );
	}

	// Load all the skeletons/skins
	for (size_t i = 0; i < data->nodes_count; i++) {
		if (data->nodes[i].skin == nullptr) continue;
		cgltf_skin *skin = data->nodes[i].skin;
		cgltf_node *node = &data->nodes[i];

		// Each GLTF skin node has an sk_mesh node for each of its primitives.
		for (cgltf_size p = 0; node->mesh && p < node->mesh->primitives_count; p++) {
			anim_skeleton_t skel = {};
			skel.bone_count       = (int32_t)skin->joints_count;
			skel.bone_to_node_map = sk_malloc_t(int32_t, skel.bone_count);
			skel.skin_node        = *node_map.get(&data->nodes[i]) + (int)p;
			for (int32_t b = 0; b < skel.bone_count; b++) {
				skel.bone_to_node_map[b] = *node_map.get(skin->joints[b]);
			}
			model->anim_data.skeletons.add(skel);
		}
	}

	for (int32_t i = 0; i < warnings.count; i++) {
		log_warnf("[%s] %s", filename, warnings[i]);
	}

	warnings.free();
	node_map.free();
	cgltf_free(data);
	return true;
}

}