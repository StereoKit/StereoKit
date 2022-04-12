#define _CRT_SECURE_NO_WARNINGS 1

#include "model.h"
#include "mesh.h"
#include "texture.h"
#include "../sk_math.h"
#include "../sk_memory.h"
#include "../systems/defaults.h"
#include "../libraries/ferr_hash.h"
#include "../libraries/stref.h"
#include "../systems/platform/platform_utils.h"
#include "../libraries/cgltf.h"

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

///////////////////////////////////////////

void gltf_add_warning(array_t<const char *> *warnings, const char *text) {
	for (size_t i = 0; i < warnings->count; i++) {
		if (warnings->data[i] == text)
			return;
	}
	warnings->add(text);
}

///////////////////////////////////////////

bool gltf_parseskin(mesh_t sk_mesh, cgltf_node *node, const char *filename) {
	if (node->skin == nullptr)
		return false;
	if (node->mesh->primitives_count > 1) {
		log_warnf("[%s] multimaterial skinned meshes not supported yet", filename);
		return false;
	}
	
	cgltf_mesh      *m = node->mesh;
	cgltf_primitive *p = &m->primitives[0];

	uint16_t *bone_ids   = nullptr;
	int32_t   bone_id_ct = 0;
	vec4     *weights    = nullptr;
	int32_t   weight_ct  = 0;
	matrix   *bone_trs   = nullptr;
	int32_t   bone_tr_ct = 0;

	// Load the mesh's joint bindings and weights
	for (size_t a = 0; a < p->attributes_count; a++) {
		cgltf_attribute   *attr      = &p->attributes[a];
		cgltf_buffer_view *buff      = attr->data->buffer_view;
		size_t             offset    = buff->offset + attr->data->offset;
		uint8_t           *attr_data = ((uint8_t *)buff->buffer->data) + offset;

		if (attr->type == cgltf_attribute_type_joints && attr->index == 0) {
			int32_t _components = 4;
			if      (attr->data->type == cgltf_type_vec4  ) _components = 4;
			else if (attr->data->type == cgltf_type_vec3  ) _components = 3;
			else if (attr->data->type == cgltf_type_vec2  ) _components = 2;
			else if (attr->data->type == cgltf_type_scalar) _components = 1;
			const int32_t components = _components;

			bone_id_ct = (int32_t)attr->data->count;
			bone_ids   = sk_malloc_t(uint16_t, (size_t)bone_id_ct * 4);
			memset(bone_ids, 0, sizeof(uint16_t) * bone_id_ct * 4);

			if (attr->data->is_sparse) {
				log_errf("[%s] Sparse joints not implemented", filename);
			} else if (attr->data->component_type == cgltf_component_type_r_8u){
				if (components == 1) for (int32_t j = 0; j < bone_id_ct; j++) {
					bone_ids[j*4  ] = attr_data[j];
				} else if (components == 2) for (int32_t j = 0; j < bone_id_ct; j++) {
					bone_ids[j*4  ] = attr_data[j*components];
					bone_ids[j*4+1] = attr_data[j*components+1];
				} else if (components == 3) for (int32_t j = 0; j < bone_id_ct; j++) {
					bone_ids[j*4  ] = attr_data[j*components];
					bone_ids[j*4+1] = attr_data[j*components+1];
					bone_ids[j*4+2] = attr_data[j*components+2];
				} else if (components == 4) for (int32_t j = 0; j < bone_id_ct; j++) {
					bone_ids[j*4  ] = attr_data[j*components];
					bone_ids[j*4+1] = attr_data[j*components+1];
					bone_ids[j*4+2] = attr_data[j*components+2];
					bone_ids[j*4+3] = attr_data[j*components+3];
				}
			} else if (attr->data->component_type == cgltf_component_type_r_16u){
				uint16_t *attr_data_16 = (uint16_t*)attr_data;
				if (components == 1) for (int32_t j = 0; j < bone_id_ct; j++) {
					bone_ids[j*4  ] = attr_data_16[j];
				} else if (components == 2) for (int32_t j = 0; j < bone_id_ct; j++) {
					bone_ids[j*4  ] = attr_data_16[j*components];
					bone_ids[j*4+1] = attr_data_16[j*components+1];
				} else if (components == 3) for (int32_t j = 0; j < bone_id_ct; j++) {
					bone_ids[j*4  ] = attr_data_16[j*components];
					bone_ids[j*4+1] = attr_data_16[j*components+1];
					bone_ids[j*4+2] = attr_data_16[j*components+2];
				} else if (components == 4) memcpy(bone_ids, attr_data_16, sizeof(uint16_t)*bone_id_ct*components);
			} else if (attr->data->component_type == cgltf_component_type_r_32u){
				uint32_t *attr_data_32 = (uint32_t*)attr_data;
				if (components == 1) for (int32_t j = 0; j < bone_id_ct; j++) {
					bone_ids[j*4  ] = (uint16_t)attr_data_32[j];
				} else if (components == 2) for (int32_t j = 0; j < bone_id_ct; j++) {
					bone_ids[j*4  ] = (uint16_t)attr_data_32[j*components];
					bone_ids[j*4+1] = (uint16_t)attr_data_32[j*components+1];
				} else if (components == 3) for (int32_t j = 0; j < bone_id_ct; j++) {
					bone_ids[j*4  ] = (uint16_t)attr_data_32[j*components];
					bone_ids[j*4+1] = (uint16_t)attr_data_32[j*components+1];
					bone_ids[j*4+2] = (uint16_t)attr_data_32[j*components+2];
				} else if (components == 4) for (int32_t j = 0; j < bone_id_ct; j++) {
					bone_ids[j*4  ] = (uint16_t)attr_data_32[j*components];
					bone_ids[j*4+1] = (uint16_t)attr_data_32[j*components+1];
					bone_ids[j*4+2] = (uint16_t)attr_data_32[j*components+2];
					bone_ids[j*4+3] = (uint16_t)attr_data_32[j*components+3];
				}
			} else {
				log_errf("[%s] joint format (%d) not implemented", filename, attr->data->component_type);
				free(bone_ids);
				bone_ids = nullptr;
			}
		} else if (attr->type == cgltf_attribute_type_weights && attr->index == 0) {
			int32_t _components = 4;
			if      (attr->data->type == cgltf_type_vec4  ) _components = 4;
			else if (attr->data->type == cgltf_type_vec3  ) _components = 3;
			else if (attr->data->type == cgltf_type_vec2  ) _components = 2;
			else if (attr->data->type == cgltf_type_scalar) _components = 1;
			const int32_t components = _components;

			if (attr->data->component_type == cgltf_component_type_r_32f) {
				size_t count = cgltf_accessor_unpack_floats(attr->data, nullptr, 0);
				if (count != components * attr->data->count)
					log_errf("[%s] mismatched weight count", filename); // Hostile asset?
				cgltf_float *floats = sk_malloc_t(cgltf_float, count);
				cgltf_accessor_unpack_floats(attr->data, floats, count);

				weight_ct = (int32_t)attr->data->count;
				weights   = sk_malloc_t(vec4, weight_ct);
				memset(weights, 0, sizeof(vec4) *weight_ct);

				if (components == 1) for (int32_t j = 0; j < weight_ct; j++) {
					weights[j].x = 1; // one weight? Must be 1
				} else if (components == 2) for (int32_t j = 0; j < weight_ct; j++) {
					int32_t jc = j * components;
					vec4   *w  = &weights[j];
					w->x = floats[jc];
					w->y = floats[jc+1];
					float sum = 1/(w->x + w->y);
					w->x = w->x * sum;
					w->y = w->y * sum;
				} else if (components == 3) for (int32_t j = 0; j < weight_ct; j++) {
					int32_t jc = j * components;
					vec4   *w  = &weights[j];
					w->x = floats[jc];
					w->y = floats[jc+1];
					w->z = floats[jc+2];
					float sum = 1/(w->x + w->y + w->z);
					w->x = w->x * sum;
					w->y = w->y * sum;
					w->z = w->z * sum;
				}
				if (components == 4) {
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
					free(floats);
				}
			} else {
				log_errf("[%s] weights format (%d) not implemented", filename, attr->data->component_type);
			}
		}
	}

	if (!bone_ids || !weights) {
		log_errf("[%s] mesh skin incomplete", filename);
		free(bone_ids);
		free(weights);
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

	return true;
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
		log_errf("[%s] GLTF Draco Mesh Compression not currently supported", filename);
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

	bool has_normals = false;
	for (size_t a = 0; a < p->attributes_count; a++) {
		cgltf_attribute   *attr   = &p->attributes[a];
		cgltf_buffer_view *buff   = attr->data->buffer_view;
		size_t             offset = buff->offset + attr->data->offset;
		
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
			} else if (!attr->data->is_sparse && attr->data->component_type == cgltf_component_type_r_32f && attr->data->type == cgltf_type_vec3) {
				// Ideal case is vec3 floats
				for (cgltf_size v = 0; v < attr->data->count; v++) {
					vec3 *pos = (vec3 *)(((uint8_t *)buff->buffer->data) + (attr->data->stride * v) + offset);
					verts[v].pos = *pos;
				}
			} else {
				// For everything else, we'll convert to floats, and use that
				size_t       count  = cgltf_accessor_unpack_floats(attr->data, nullptr, 0);
				cgltf_float *floats = sk_malloc_t(cgltf_float, count);
				cgltf_accessor_unpack_floats(attr->data, floats, count);

				if (attr->data->type == cgltf_type_vec3) {
					for (cgltf_size v = 0; v < attr->data->count; v++) {
						vec3 *pos = (vec3*)&floats[v * 3];
						verts[v].pos = *pos;
					}
				} else {
					log_errf("[%s] Unimplemented vertex position type (%d)", filename, attr->data->type);
				}
				free(floats);
			}
		} else if (attr->type == cgltf_attribute_type_normal) {
			has_normals = true;
			if (attr->index != 0) {
				gltf_add_warning(warnings, "Too many vertex normal channels! Only one supported, the rest will be ignored.");
			} else if (!attr->data->is_sparse && attr->data->component_type == cgltf_component_type_r_32f && attr->data->type == cgltf_type_vec3) {
				// Ideal case is vec3 floats
				for (size_t v = 0; v < attr->data->count; v++) {
					vec3 *norm = (vec3 *)(((uint8_t *)buff->buffer->data) + (attr->data->stride * v) + offset);
					verts[v].norm = *norm;
				}
			} else {
				// For everything else, we'll convert to floats, and use that
				size_t       count  = cgltf_accessor_unpack_floats(attr->data, nullptr, 0);
				cgltf_float *floats = sk_malloc_t(cgltf_float, count);
				cgltf_accessor_unpack_floats(attr->data, floats, count);

				if (attr->data->type == cgltf_type_vec3) {
					for (size_t v = 0; v < attr->data->count; v++) {
						vec3 *norm = (vec3*)&floats[v * 3];
						verts[v].norm = *norm;
					}
				} else {
					log_errf("[%s] Unimplemented vertex normal type (%d)", filename, attr->data->type);
				}
				free(floats);
			}
		} else if (attr->type == cgltf_attribute_type_texcoord) {
			if (attr->index != 0) {
				gltf_add_warning(warnings, "Too many texture coordinate channels! Only one supported, the rest will be ignored.");
			} else if (!attr->data->is_sparse && attr->data->component_type == cgltf_component_type_r_32f && attr->data->type == cgltf_type_vec2) {
				// Ideal case is vec2 floats
				for (size_t v = 0; v < attr->data->count; v++) {
					vec2 *uv = (vec2 *)(((uint8_t *)buff->buffer->data) + (attr->data->stride * v) + offset);
					verts[v].uv = *uv;
				}
			} else {
				// For everything else, we'll convert to floats, and use that
				size_t       count  = cgltf_accessor_unpack_floats(attr->data, nullptr, 0);
				cgltf_float *floats = sk_malloc_t(cgltf_float, count);
				cgltf_accessor_unpack_floats(attr->data, floats, count);

				if (attr->data->type == cgltf_type_vec2) {
					for (size_t v = 0; v < attr->data->count; v++) {
						vec2 *uv = (vec2*)&floats[v * 2];
						verts[v].uv = *uv;
					}
				} else {
					log_errf("[%s] Unimplemented vertex uv type (%d)", filename, attr->data->type);
				}
				free(floats);
			}
		} else if (attr->type == cgltf_attribute_type_color) {
			if (attr->index != 0) {
				gltf_add_warning(warnings, "Too many vertex color channels! Only one supported, the rest will be ignored.");
			} else if (!attr->data->is_sparse && attr->data->component_type == cgltf_component_type_r_8u && attr->data->type == cgltf_type_vec4) {
				// Ideal case is vec4 uint8_t colors
				for (size_t v = 0; v < attr->data->count; v++) {
					color32 *col = (color32 *)(((uint8_t *)buff->buffer->data) + (attr->data->stride * v) + offset);
					verts[v].col = *col;
				}
			} else if (!attr->data->is_sparse && attr->data->component_type == cgltf_component_type_r_32f && attr->data->type == cgltf_type_vec4) {
				// vec4 float colors are also pretty straightforward
				for (size_t v = 0; v < attr->data->count; v++) {
					color128 *col = (color128 *)(((uint8_t *)buff->buffer->data) + (attr->data->stride * v) + offset);
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
				free(floats);
			}
		}
	}

	// Now grab the mesh indices
	size_t  ind_count = p->indices->count;
	vind_t *inds      = sk_malloc_t(vind_t, ind_count);
	if (!p->indices->is_sparse && p->indices->component_type == cgltf_component_type_r_8u) {
		cgltf_buffer_view *buff   = p->indices->buffer_view;
		size_t             offset = buff->offset + p->indices->offset;
		for (size_t v = 0; v < ind_count; v++) {
			uint8_t *ind = (uint8_t *)(((uint8_t *)buff->buffer->data) + (p->indices->stride * v) + offset);
			inds[v] = *ind;
		}
	} else if (!p->indices->is_sparse && p->indices->component_type == cgltf_component_type_r_16u) {
		cgltf_buffer_view *buff   = p->indices->buffer_view;
		size_t             offset = buff->offset + p->indices->offset;
		for (size_t v = 0; v < ind_count; v++) {
			uint16_t *ind = (uint16_t *)(((uint8_t *)buff->buffer->data) + (p->indices->stride * v) + offset);
			inds[v] = *ind;
		}
	} else if (!p->indices->is_sparse && p->indices->component_type == cgltf_component_type_r_32u) {
		cgltf_buffer_view *buff   = p->indices->buffer_view;
		size_t             offset = buff->offset + p->indices->offset;
		for (size_t v = 0; v < ind_count; v++) {
			uint32_t *ind = (uint32_t *)(((uint8_t *)buff->buffer->data) + (p->indices->stride * v) + offset);
#ifdef SK_32BIT_INDICES
			inds[v] = *ind;
#else
			inds[v] = *ind > 0x0000FFFF ? 0 : (uint16_t)*ind;
#endif
		}
	} else {
		gltf_add_warning(warnings, "Unimplemented vertex index format");
	}

	if (!has_normals) {
		mesh_calculate_normals(verts, vert_count, inds, (int32_t)ind_count);
	}

	result = mesh_create();
	mesh_set_id  (result, id);
	mesh_set_data(result, verts, vert_count, inds, (int32_t)ind_count);
	free(verts);
	free(inds );

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

tex_t gltf_parsetexture(cgltf_data* data, cgltf_texture *tex, const char *filename, bool srgb_data) {
	cgltf_image *image = tex->image;

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
			log_warnf("[%s] Couldn't load texture: %s", filename, image->name);
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
	if (result != nullptr)
		gltf_apply_sampler(result, tex->sampler);

	return result;
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
	if (shader != nullptr) {
		result = material_create(shader);
	} else {
		if (material == nullptr) {
			result = material_copy_id(default_id_material);
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
			tex_t parse_tex = gltf_parsetexture(data, tex, filename, true);
			tex_set_fallback(parse_tex, sk_default_tex);
			if (parse_tex != nullptr) {
				material_set_texture(result, "diffuse", parse_tex);
				tex_release(parse_tex);
			}
		}

		tex = material->pbr_metallic_roughness.metallic_roughness_texture.texture;
		if (tex != nullptr && material_has_param(result, "metal", material_param_texture)) {
			if (material->pbr_metallic_roughness.metallic_roughness_texture.texcoord != 0) gltf_add_warning(warnings, "StereoKit doesn't support loading multiple texture coordinate channels yet.");
			tex_t parse_tex = gltf_parsetexture(data, tex, filename, false);
			tex_set_fallback(parse_tex, sk_default_tex_rough);
			if (parse_tex != nullptr) {
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
			tex_t parse_tex = gltf_parsetexture(data, tex, filename, true);
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
		tex_t parse_tex = gltf_parsetexture(data, tex, filename, false);
		tex_set_fallback(parse_tex, sk_default_tex_flat);
		if (parse_tex != nullptr) {
			material_set_texture(result, "normal", parse_tex);
			tex_release(parse_tex);
		}
	}

	tex = material->occlusion_texture.texture;
	if (tex != nullptr && material_has_param(result, "occlusion", material_param_texture)) {
		if (material->occlusion_texture.texcoord != 0) gltf_add_warning(warnings, "StereoKit doesn't support multiple texture coordinate channels yet.");
		tex_t parse_tex = gltf_parsetexture(data, tex, filename, false);
		tex_set_fallback(parse_tex, sk_default_tex);
		if (parse_tex != nullptr) {
			material_set_texture(result, "occlusion", parse_tex);
			tex_release(parse_tex);
		}
	}

	tex = material->emissive_texture.texture;
	if (tex != nullptr && material_has_param(result, "emission", material_param_texture)) {
		if (material->emissive_texture.texcoord != 0) gltf_add_warning(warnings, "StereoKit doesn't support multiple texture coordinate channels yet.");
		tex_t parse_tex = gltf_parsetexture(data, tex, filename, true);
		tex_set_fallback(parse_tex, sk_default_tex_black);
		if (parse_tex != nullptr) {
			material_set_texture(result, "emission", parse_tex);
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
					rot[k*skip] = r * rot[k*skip];
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
		if (node_id == -1)
			node_id = new_node;

		mesh_release    (mesh);
		material_release(material);
	}

	if (node->skin && node_id != -1)
		gltf_parseskin(model_node_get_mesh(model, node_id), node, filename);

	if (node_id == -1) {
		node_id = model_node_add_child(model, parent, node->name, transform, nullptr, nullptr);
	}
	node_map->add(node, node_id);

	for (size_t i = 0; i < node->children_count; i++) {
		gltf_add_node(model, shader, node_id, filename, data, node->children[i], node_map, warnings);
	}
}

///////////////////////////////////////////

bool modelfmt_gltf(model_t model, const char *filename, void *file_data, size_t file_size, shader_t shader) {
	cgltf_options options = {};
	options.file.read = [](const struct cgltf_memory_options*, const struct cgltf_file_options*, const char* path, cgltf_size* size, void** data) {
		return platform_read_file(path, data, size)
			? cgltf_result_success
			: cgltf_result_file_not_found;
	};
	options.file.release = [](const struct cgltf_memory_options*, const struct cgltf_file_options*, void* data) {
		free(data);
	};

	cgltf_data *data       = nullptr;
	const char *model_file = assets_file(filename);

	cgltf_result result = cgltf_parse(&options, file_data, file_size, &data);
	if (result != cgltf_result_success) {
		log_diagf("[%s] gltf parse err %d", filename, result);
		cgltf_free(data);
		return false;
	}

	result = cgltf_load_buffers(&options, data, model_file);
	if (result != cgltf_result_success) {
		log_diagf("[%s] gltf buffer load err %d", filename, result);
		cgltf_free(data);
		return false;
	}

	array_t<const char *> warnings = {};

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

		anim_skeleton_t skel = {};
		skel.bone_count       = (int32_t)skin->joints_count;
		skel.bone_to_node_map = sk_malloc_t(int32_t, skel.bone_count);
		skel.skin_node        = *node_map.get(&data->nodes[i]);
		for (int32_t b = 0; b < skel.bone_count; b++) {
			skel.bone_to_node_map[b] = *node_map.get(skin->joints[b]);
		}
		model->anim_data.skeletons.add(skel);
	}

	for (size_t i = 0; i < warnings.count; i++) {
		log_warnf("[%s] %s", filename, warnings[i]);
	}

	warnings.free();
	node_map.free();
	cgltf_free(data);
	return true;
}

}