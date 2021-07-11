#define _CRT_SECURE_NO_WARNINGS 1

#include "model.h"
#include "mesh.h"
#include "texture.h"
#include "../sk_math.h"
#include "../sk_memory.h"
#include "../libraries/ferr_hash.h"
#include "../systems/platform/platform_utils.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning( disable : 26451 )
#else
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#endif
#define CGLTF_IMPLEMENTATION
#include "../libraries/cgltf.h"
#ifdef _MSC_VER
#pragma warning(pop)
#else
#pragma clang diagnostic pop
#endif

namespace sk {

///////////////////////////////////////////

mesh_t gltf_parsemesh(cgltf_mesh *mesh, int node_id, int primitive_id, const char *filename) {
	cgltf_mesh      *m = mesh;
	cgltf_primitive *p = &m->primitives[primitive_id];

	if (p->type != cgltf_primitive_type_triangles) {
		log_warnf("[%s] Unimplemented GLTF primitive mode: %d", filename, p->type);
		return nullptr;
	}
	if (p->has_draco_mesh_compression) {
		log_warnf("[%s] GLTF Draco Mesh Compression not currently supported", filename);
		return nullptr;
	}

	char id[512];
	snprintf(id, sizeof(id), "%s/mesh/%d_%d_%s", filename, node_id, primitive_id, m->name);
	mesh_t result = mesh_find(id);
	if (result != nullptr) {
		return result;
	}

	vert_t *verts = nullptr;
	int     vert_count = 0;

	bool has_normals = false;
	for (size_t a = 0; a < p->attributes_count; a++) {
		cgltf_attribute   *attr   = &p->attributes[a];
		cgltf_buffer_view *buff   = attr->data->buffer_view;
		size_t             offset = buff->offset + attr->data->offset;
		
		// Make sure we have memory for our verts
		if (vert_count < attr->data->count) {
			vert_count = (int)attr->data->count;
			verts      = sk_realloc_t(vert_t, verts, vert_count);
			for (size_t i = 0; i < vert_count; i++) {
				verts[i] = vert_t{ vec3_zero, vec3_zero, vec2_zero, {255,255,255,255} };
			}
		}

		// Check what info is in this attribute, and copy it over to our mesh
		if (attr->type == cgltf_attribute_type_position) {
			if (!attr->data->is_sparse && attr->data->component_type == cgltf_component_type_r_32f && attr->data->type == cgltf_type_vec3) {
				// Ideal case is vec3 floats
				for (size_t v = 0; v < attr->data->count; v++) {
					vec3 *pos = (vec3 *)(((uint8_t *)buff->buffer->data) + (sizeof(vec3) * v) + offset);
					verts[v].pos = *pos;
				}
			} else {
				// For everything else, we'll convert to floats, and use that
				size_t       count  = cgltf_accessor_unpack_floats(attr->data, nullptr, 0);
				cgltf_float *floats = sk_malloc_t(cgltf_float, count);
				cgltf_accessor_unpack_floats(attr->data, floats, count);

				if (attr->data->type == cgltf_type_vec3) {
					for (size_t v = 0; v < attr->data->count; v++) {
						vec3 *pos = (vec3*)&floats[v * 3];
						verts[v].pos = *pos;
					}
				} else {
					log_errf("[%s] Unimplemented vertex position format", filename);
				}
				free(floats);
			}
		} else if (attr->type == cgltf_attribute_type_normal) {
			has_normals = true;
			if (!attr->data->is_sparse && attr->data->component_type == cgltf_component_type_r_32f && attr->data->type == cgltf_type_vec3) {
				// Ideal case is vec3 floats
				for (size_t v = 0; v < attr->data->count; v++) {
					vec3 *norm = (vec3 *)(((uint8_t *)buff->buffer->data) + (sizeof(vec3) * v) + offset);
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
					log_errf("[%s] Unimplemented vertex normal format", filename);
				}
				free(floats);
			}
		} else if (attr->type == cgltf_attribute_type_texcoord) {
			if (!attr->data->is_sparse && attr->data->component_type == cgltf_component_type_r_32f && attr->data->type == cgltf_type_vec2) {
				// Ideal case is vec2 floats
				for (size_t v = 0; v < attr->data->count; v++) {
					vec2 *uv = (vec2 *)(((uint8_t *)buff->buffer->data) + (sizeof(vec2) * v) + offset);
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
					log_errf("[%s] Unimplemented vertex uv format", filename);
				}
				free(floats);
			}
		} else if (attr->type == cgltf_attribute_type_color) {
			if (!attr->data->is_sparse && attr->data->component_type == cgltf_component_type_r_8u && attr->data->type == cgltf_type_vec4) {
				// Ideal case is vec4 uint8_t colors
				for (size_t v = 0; v < attr->data->count; v++) {
					color32 *col = (color32 *)(((uint8_t *)buff->buffer->data) + (sizeof(color32) * v) + offset);
					verts[v].col = *col;
				}
			} else if (!attr->data->is_sparse && attr->data->component_type == cgltf_component_type_r_32f && attr->data->type == cgltf_type_vec4) {
				// vec4 float colors are also pretty straightforward
				for (size_t v = 0; v < attr->data->count; v++) {
					color128 *col = (color128 *)(((uint8_t *)buff->buffer->data) + (sizeof(color128) * v) + offset);
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
					log_errf("[%s] Unimplemented vertex color format", filename);
				}
				free(floats);
			}
		}
	}

	// Now grab the mesh indices
	int     ind_count = (int)p->indices->count;
	vind_t *inds      = sk_malloc_t(vind_t, ind_count);
	if (!p->indices->is_sparse && p->indices->component_type == cgltf_component_type_r_8u) {
		cgltf_buffer_view *buff   = p->indices->buffer_view;
		size_t             offset = buff->offset + p->indices->offset;
		for (size_t v = 0; v < ind_count; v++) {
			uint8_t *ind = (uint8_t *)(((uint8_t *)buff->buffer->data) + (sizeof(uint8_t) * v) + offset);
			inds[v] = *ind;
		}
	} else if (!p->indices->is_sparse && p->indices->component_type == cgltf_component_type_r_16u) {
		cgltf_buffer_view *buff   = p->indices->buffer_view;
		size_t             offset = buff->offset + p->indices->offset;
		for (size_t v = 0; v < ind_count; v++) {
			uint16_t *ind = (uint16_t *)(((uint8_t *)buff->buffer->data) + (sizeof(uint16_t) * v) + offset);
			inds[v] = *ind;
		}
	} else if (!p->indices->is_sparse && p->indices->component_type == cgltf_component_type_r_32u) {
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
	} else {
		log_errf("[%s] Unimplemented vertex index format", filename);
	}

	if (!has_normals) {
		mesh_calculate_normals(verts, vert_count, inds, ind_count);
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
	return result;
}

///////////////////////////////////////////

material_t gltf_parsematerial(cgltf_data *data, cgltf_material *material, const char *filename, shader_t shader) {
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
			result = material_copy_id(default_id_material_unlit);
		} else if (material->has_pbr_metallic_roughness) {
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
		if (tex != nullptr && material_has_param(result, "diffuse", material_param_texture))
			material_set_texture(result, "diffuse", gltf_parsetexture(data, tex->image, filename, true));

		tex = material->pbr_metallic_roughness.metallic_roughness_texture.texture;
		if (tex != nullptr && material_has_param(result, "metal", material_param_texture))
			material_set_texture(result, "metal", gltf_parsetexture(data, tex->image, filename, false));

		float *c = material->pbr_metallic_roughness.base_color_factor;
		if (material_has_param(result, "color", material_param_color128))
			material_set_color(result, "color", { c[0], c[1], c[2], c[3] });

		if (material_has_param(result, "metallic",  material_param_float))
			material_set_float(result, "metallic",  material->pbr_metallic_roughness.metallic_factor);
		if (material_has_param(result, "roughness", material_param_float))
			material_set_float(result, "roughness", material->pbr_metallic_roughness.roughness_factor);
	}
	if (material->double_sided)
		material_set_cull(result, cull_none);
	if (material->alpha_mode == cgltf_alpha_mode_blend)
		material_set_transparency(result, transparency_blend);

	tex = material->normal_texture.texture;
	if (tex != nullptr && material_has_param(result, "normal", material_param_texture))
		material_set_texture(result, "normal", gltf_parsetexture(data, tex->image, filename, false));

	tex = material->occlusion_texture.texture;
	if (tex != nullptr && material_has_param(result, "occlusion", material_param_texture))
		material_set_texture(result, "occlusion", gltf_parsetexture(data, tex->image, filename, false));

	tex = material->emissive_texture.texture;
	if (tex != nullptr && material_has_param(result, "emission", material_param_texture))
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
		return false;
	}

	result = cgltf_load_buffers(&options, data, model_file);
	if (result != cgltf_result_success) {
		log_diagf("[%s] gltf buffer load err %d", filename, result);
		cgltf_free(data);
		return false;
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
		matrix offset = transform * orientation_correction;

		for (int32_t p = 0; p < n->mesh->primitives_count; p++) {
			mesh_t mesh = gltf_parsemesh(n->mesh, i, p, filename);
			if (mesh == nullptr) continue;

			material_t material = gltf_parsematerial(data, n->mesh->primitives[p].material, filename, shader);

			model_add_named_subset(model, n->name, mesh, material, offset);

			mesh_release    (mesh);
			material_release(material);
		}
	}
	cgltf_free(data);
	return true;
}

}