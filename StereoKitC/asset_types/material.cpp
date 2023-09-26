#include "material.h"
#include "shader.h"
#include "texture.h"
#include "../libraries/stref.h"
#include "../libraries/ferr_hash.h"
#include "../libraries/array.h"
#include "../sk_memory.h"
#include "../systems/defaults.h"

#include <stdio.h>
#include <string.h>

namespace sk {

_material_buffer_t material_buffers[14] = {};

///////////////////////////////////////////

void material_copy_pipeline(material_t dest, const material_t src);
void material_update_label (material_t material);

///////////////////////////////////////////

material_t material_find(const char *id) {
	material_t result = (material_t)assets_find(id, asset_type_material);
	if (result != nullptr) {
		material_addref(result);
		return result;
	}
	return result;
}

///////////////////////////////////////////

void material_set_id(material_t material, const char *id) {
	assets_set_id(&material->header, id);
	material_update_label(material);
}

///////////////////////////////////////////

const char* material_get_id(const material_t material) {
	return material->header.id_text;
}

///////////////////////////////////////////

void material_update_label(material_t material) {
#if !defined(SKG_OPENGL) && (defined(_DEBUG) || defined(SK_GPU_LABELS))
	if (material->header.id_text != nullptr) {
		skg_pipeline_name(&material->pipeline, material->header.id_text);
		skg_buffer_name  (&material->args.buffer_gpu, material->header.id_text);
	}
#else
	(void)material;
#endif
}

///////////////////////////////////////////

inline size_t material_param_size(material_param_ type) {
	switch (type) {
	case material_param_float:    return sizeof(float);
	case material_param_color128: return sizeof(color128);
	case material_param_vector4:  return sizeof(vec4);
	case material_param_vector3:  return sizeof(vec3);
	case material_param_vector2:  return sizeof(vec2);
	case material_param_matrix:   return sizeof(matrix);
	case material_param_texture:  return sizeof(tex_t);
	default: log_err("Bad material param type"); return 0;
	}
}

///////////////////////////////////////////

void material_create_arg_defaults(material_t material, shader_t shader) {
	const skg_shader_meta_t   *meta      = shader->shader.meta;
	const skg_shader_buffer_t *buff_info = meta->global_buffer_id != -1
		? &meta->buffers[meta->global_buffer_id]
		: nullptr;
	uint32_t buff_size = buff_info ? (uint32_t)buff_info->size : 0;

	material->args.buffer       = buff_size > 0 ? sk_malloc(buff_size) : nullptr;
	material->args.buffer_size  = buff_size;
	material->args.buffer_bind  = buff_info ? buff_info->bind : skg_bind_t{};
	material->args.buffer_dirty = false;
	if (buff_info && buff_info->defaults != nullptr) memcpy(material->args.buffer, buff_info->defaults, buff_size);
	else                                             memset(material->args.buffer, 0, buff_size);
	if (buff_size != 0) {
		material->args.buffer_dirty = true;

		// Construct a material parameters buffer on the GPU, and do it
		// threadsafe
		struct material_job_t {
			material_t material;
			uint32_t   buff_size;
		};
		material_job_t job_data = {material, buff_size};
		assets_execute_gpu([](void *data) {
			material_job_t *job_data = (material_job_t *)data;
			job_data->material->args.buffer_gpu = skg_buffer_create(nullptr, 1, job_data->buff_size, skg_buffer_type_constant, skg_use_dynamic);
			return (bool32_t)true;
		}, &job_data);
	}

	material->args.texture_count = meta->resource_count;
	material->args.textures      = meta->resource_count > 0 ? sk_malloc_t(shaderargs_tex_t, meta->resource_count) : nullptr;
	memset(material->args.textures, 0, sizeof(tex_t) * meta->resource_count);
	for (uint32_t i = 0; i < meta->resource_count; i++) {
		shaderargs_tex_t *tex_arg     = &material->args.textures[i];
		tex_t             default_tex = nullptr;

		if      (string_eq(meta->resources[i].extra, "white")) default_tex = tex_find(default_id_tex);
		else if (string_eq(meta->resources[i].extra, "black")) default_tex = tex_find(default_id_tex_black);
		else if (string_eq(meta->resources[i].extra, "gray" )) default_tex = tex_find(default_id_tex_gray);
		else if (string_eq(meta->resources[i].extra, "flat" )) default_tex = tex_find(default_id_tex_flat);
		else if (string_eq(meta->resources[i].extra, "rough")) default_tex = tex_find(default_id_tex_rough);
		else                                                   default_tex = tex_find(default_id_tex);
		tex_arg->tex       = default_tex;
		tex_arg->meta_hash = 0;
		tex_arg->bind      = meta->resources[i].bind;
	}
}

///////////////////////////////////////////

material_t material_create(shader_t shader) {
	shader_t material_shader = shader;
	if (material_shader == nullptr) {
		log_warn("material_create was provided a null shader, defaulting to an unlit shader.");
		material_shader = shader_find(default_id_shader_unlit);
	}
	material_t result = (material_t)assets_allocate(asset_type_material);
	shader_addref(material_shader);
	result->alpha_mode = transparency_none;
	result->shader     = material_shader;
	result->depth_test = depth_test_less;
	result->depth_write= true;
	result->pipeline   = skg_pipeline_create(&result->shader->shader);

	material_set_cull(result, cull_back);
	material_create_arg_defaults(result, material_shader);

	if (shader == nullptr) {
		shader_release(material_shader);
	}

	return result;
}

///////////////////////////////////////////

material_t material_copy(material_t material) {
	if (material == nullptr) return nullptr;

	// Make a new empty material
	material_t result = material_create(material->shader);
	// release any of the default textures for the material.
	for (int32_t i = 0; i < result->args.texture_count; i++) {
		if (result->args.textures[i].tex != nullptr)
			tex_release(result->args.textures[i].tex);
	}
	// Store allocated memory temporarily
	void             *tmp_buffer        = result->args.buffer;
	skg_buffer_t      tmp_buffer_gpu    = result->args.buffer_gpu;
	shaderargs_tex_t *tmp_textures      = result->args.textures;
	asset_header_t    tmp_header        = result->header;

	// Copy everything over from the old one, and then re-write with our own custom memory. Then copy that over too!
	memcpy(result, material, sizeof(_material_t));
	result->header             = tmp_header;
	result->args.buffer        = tmp_buffer;
	result->args.buffer_gpu    = tmp_buffer_gpu;
	result->args.textures      = tmp_textures;
	result->args.buffer_dirty  = true;
	memcpy(result->args.buffer,   material->args.buffer,   material->args.buffer_size);
	memcpy(result->args.textures, material->args.textures, sizeof(shaderargs_tex_t) * material->args.texture_count);

	// Add references to all the other material's assets
	for (int32_t i = 0; i < result->args.texture_count; i++) {
		if (result->args.textures[i].tex != nullptr)
			tex_addref(result->args.textures[i].tex);
	}
	if (result->chain != nullptr) material_addref(result->chain);

	// Copy over the material's pipeline
	result->pipeline = skg_pipeline_create(&material->shader->shader);
	material_copy_pipeline(result, material);

	return result;
}

///////////////////////////////////////////

void material_copy_pipeline(material_t dest, const material_t src) {
	material_set_cull        (dest, src->cull);
	material_set_depth_test  (dest, src->depth_test);
	material_set_depth_write (dest, src->depth_write);
	material_set_transparency(dest, src->alpha_mode);
	material_set_wireframe   (dest, src->wireframe);
}

///////////////////////////////////////////

material_t material_copy_id(const char *id) {
	material_t src    = material_find(id);
	material_t result = material_copy(src);
	material_release(src);
	return result;
}

///////////////////////////////////////////

void material_addref(material_t material) {
	assets_addref(&material->header);
}

///////////////////////////////////////////

void material_release(material_t material) {
	if (material == nullptr)
		return;
	assets_releaseref(&material->header);
}

///////////////////////////////////////////

void material_destroy(material_t material) {
	if (material->chain) material_release(material->chain);
	for (int32_t i = 0; i < material->args.texture_count; i++) {
		if (material->args.textures[i].tex != nullptr)
			tex_release(material->args.textures[i].tex);
	}
	shader_release(material->shader);
	skg_pipeline_destroy(&material->pipeline);
	sk_free(material->args.buffer);
	sk_free(material->args.textures);
	*material = {};
}

///////////////////////////////////////////

void material_set_shader(material_t material, shader_t shader) {
	// We can't really go without a shader, so unlit is our default fallback.
	if (shader == nullptr)
		shader = sk_default_shader_unlit;

	if (shader == material->shader)
		return;

	// Update references
	shader_addref(shader);

	// Copy over any relevant values that are attached to the old shader
	if (material->shader != nullptr) {
		shader_t          old_shader   = material->shader;
		void             *old_buffer   = material->args.buffer;
		shaderargs_tex_t *old_textures = material->args.textures;
		material_create_arg_defaults(material, shader);
		material->shader = shader;

		// Copy old param values
		int32_t count = skg_shader_get_var_count(&old_shader->shader);
		for (int32_t i = 0; i < count; i++) {
			const skg_shader_var_t *item     = skg_shader_get_var_info(&old_shader->shader, i);
			const skg_shader_var_t *new_slot = skg_shader_get_var_info(&material->shader->shader, skg_shader_get_var_index_h(&material->shader->shader, item->name_hash));
			if (new_slot != nullptr)
				memcpy( (uint8_t *)material->args.buffer + new_slot->offset, 
						(uint8_t *)old_buffer            + item->offset, new_slot->size);
		}

		// Do the same for textures
		for (uint32_t i = 0; i < old_shader->shader.meta->resource_count; i++) {
			material_set_texture(material, old_shader->shader.meta->resources[i].name, old_textures[i].tex);
			tex_release(old_textures[i].tex);
		}

		// And release the old shader content
		if (old_shader != nullptr)
			shader_release(old_shader);
		skg_pipeline_destroy(&material->pipeline);
		sk_free(old_buffer);
		sk_free(old_textures);
	}

	material->shader   = shader;
	material->pipeline = skg_pipeline_create(&material->shader->shader);
	material_copy_pipeline(material, material);
}

///////////////////////////////////////////

shader_t material_get_shader(material_t material) {
	shader_addref(material->shader);
	return material->shader;
}

///////////////////////////////////////////

void material_set_transparency(material_t material, transparency_ mode) {
	material->alpha_mode = mode;
	skg_pipeline_set_transparency(&material->pipeline, (skg_transparency_)mode);
	material_update_label(material);
}

///////////////////////////////////////////

void material_set_cull(material_t material, cull_ mode) {
	material->cull = mode;
	skg_pipeline_set_cull(&material->pipeline, (skg_cull_)mode);
	material_update_label(material);
}

///////////////////////////////////////////

void material_set_wireframe(material_t material, bool32_t wireframe) {
	material->wireframe = wireframe;
	skg_pipeline_set_wireframe(&material->pipeline, wireframe);
	material_update_label(material);
}

///////////////////////////////////////////

void material_set_depth_test(material_t material, depth_test_ depth_test_mode) {
	material->depth_test = depth_test_mode;
	skg_pipeline_set_depth_test(&material->pipeline, (skg_depth_test_)depth_test_mode);
	material_update_label(material);
}

///////////////////////////////////////////

void material_set_depth_write(material_t material, bool32_t write_enabled) {
	material->depth_write = write_enabled;
	skg_pipeline_set_depth_write(&material->pipeline, write_enabled);
	material_update_label(material);
}

///////////////////////////////////////////

void material_set_queue_offset(material_t material, int32_t offset) {
	material->queue_offset = offset;
}

///////////////////////////////////////////

void material_set_chain(material_t material, material_t chain_material) {
	if (material == chain_material) {
		log_warn("Chain material can't be recursive!");
		return;
	}

	if (chain_material ) material_addref (chain_material);
	if (material->chain) material_release(material->chain);
	material->chain = chain_material;
}

///////////////////////////////////////////

transparency_ material_get_transparency(material_t material) { 
	return material->alpha_mode;
}

///////////////////////////////////////////

cull_ material_get_cull(material_t material) {
	return material->cull;
}

///////////////////////////////////////////

bool32_t material_get_wireframe(material_t material) {
	return material->wireframe;
}

///////////////////////////////////////////

depth_test_ material_get_depth_test(material_t material) {
	return material->depth_test;
}

///////////////////////////////////////////

bool32_t material_get_depth_write(material_t material) {
	return material->depth_write;
}

///////////////////////////////////////////

int32_t material_get_queue_offset(material_t material) {
	return material->queue_offset;
}

///////////////////////////////////////////

material_t material_get_chain(material_t material) {
	if (material->chain) material_addref(material->chain);
	return material->chain;
}

///////////////////////////////////////////

void *_material_get_ptr(material_t material, const char *name, uint32_t size) {
	int32_t i = skg_shader_get_var_index(&material->shader->shader, name);
	if (i == -1) return nullptr;

	const skg_shader_var_t *info = skg_shader_get_var_info(&material->shader->shader, i);
	if (size == 0 || info->size != size) {
		log_errf("material_set_: '%s' mismatched type (for shader %s)",  info->name, material->shader->shader.meta->name);
		return nullptr;
	}
	return ((uint8_t*)material->args.buffer + info->offset);
}

///////////////////////////////////////////

void material_set_float(material_t material, const char *name, float value) {
	float *matparam = (float*)_material_get_ptr(material, name, sizeof(float));
	if (matparam != nullptr) {
		*matparam = value;
		material->args.buffer_dirty = true;
	}
}

///////////////////////////////////////////

void material_set_color32(material_t material, const char *name, color32 value) {
	color128 *matparam = (color128*)_material_get_ptr(material, name, sizeof(color128));
	if (matparam != nullptr) {
		*matparam = color_to_linear(color32_to_128(value));
		material->args.buffer_dirty = true;
	}
}

///////////////////////////////////////////

void material_set_color(material_t material, const char *name, color128 value) {
	color128 *matparam = (color128*)_material_get_ptr(material, name, sizeof(color128));
	if (matparam != nullptr) {
		*matparam = color_to_linear(value);
		material->args.buffer_dirty = true;
	}
}

///////////////////////////////////////////

void material_set_vector(material_t material, const char *name, vec4 value) {
	material_set_vector4(material, name, value);
}

///////////////////////////////////////////

void material_set_vector4(material_t material, const char *name, vec4 value) {
	vec4 *matparam = (vec4*)_material_get_ptr(material, name, sizeof(vec4));
	if (matparam != nullptr) {
		*matparam = value;
		material->args.buffer_dirty = true;
	}
}

///////////////////////////////////////////

void material_set_vector3(material_t material, const char *name, vec3 value) {
	vec3 *matparam = (vec3*)_material_get_ptr(material, name, sizeof(vec3));
	if (matparam != nullptr) {
		*matparam = value;
		material->args.buffer_dirty = true;
	}
}

///////////////////////////////////////////

void material_set_vector2(material_t material, const char *name, vec2 value) {
	vec2 *matparam = (vec2*)_material_get_ptr(material, name, sizeof(vec2));
	if (matparam != nullptr) {
		*matparam = value;
		material->args.buffer_dirty = true;
	}
}

///////////////////////////////////////////

void material_set_int(material_t material, const char *name, int32_t value) {
	int32_t *matparam = (int32_t*)_material_get_ptr(material, name, sizeof(int32_t));
	if (matparam != nullptr) {
		*matparam = value;
		material->args.buffer_dirty = true;
	}
}

///////////////////////////////////////////

void material_set_int2(material_t material, const char *name, int32_t  value1, int32_t value2) {
	int32_t *matparam = (int32_t*)_material_get_ptr(material, name, sizeof(int32_t)*2);
	if (matparam != nullptr) {
		matparam[0] = value1;
		matparam[1] = value2;
		material->args.buffer_dirty = true;
	}
}

///////////////////////////////////////////

void material_set_int3(material_t material, const char *name, int32_t  value1, int32_t value2, int32_t value3) {
	int32_t *matparam = (int32_t*)_material_get_ptr(material, name, sizeof(int32_t)*3);
	if (matparam != nullptr) {
		matparam[0] = value1;
		matparam[1] = value2;
		matparam[2] = value3;
		material->args.buffer_dirty = true;
	}
}

///////////////////////////////////////////

void material_set_int4(material_t material, const char *name, int32_t  value1, int32_t value2, int32_t value3, int32_t value4) {
	int32_t *matparam = (int32_t*)_material_get_ptr(material, name, sizeof(int32_t)*4);
	if (matparam != nullptr) {
		matparam[0] = value1;
		matparam[1] = value2;
		matparam[2] = value3;
		matparam[3] = value4;
		material->args.buffer_dirty = true;
	}
}

///////////////////////////////////////////

void material_set_bool(material_t material, const char *name, bool32_t value) {
	uint32_t *matparam = (uint32_t*)_material_get_ptr(material, name, sizeof(uint32_t));
	if (matparam != nullptr) {
		*matparam = value>0?1:0;
		material->args.buffer_dirty = true;
	}
}

///////////////////////////////////////////

void material_set_uint(material_t material, const char *name, uint32_t value) {
	uint32_t *matparam = (uint32_t*)_material_get_ptr(material, name, sizeof(uint32_t));
	if (matparam != nullptr) {
		*matparam = value;
		material->args.buffer_dirty = true;
	}
}

///////////////////////////////////////////

void material_set_uint2(material_t material, const char *name, uint32_t value1, uint32_t value2) {
	uint32_t *matparam = (uint32_t*)_material_get_ptr(material, name, sizeof(uint32_t)*2);
	if (matparam != nullptr) {
		matparam[0] = value1;
		matparam[1] = value2;
		material->args.buffer_dirty = true;
	}
}

///////////////////////////////////////////

void material_set_uint3(material_t material, const char *name, uint32_t value1, uint32_t value2, uint32_t value3) {
	uint32_t *matparam = (uint32_t*)_material_get_ptr(material, name, sizeof(uint32_t)*3);
	if (matparam != nullptr) {
		matparam[0] = value1;
		matparam[1] = value2;
		matparam[2] = value3;
		material->args.buffer_dirty = true;
	}
}

///////////////////////////////////////////

void material_set_uint4(material_t material, const char *name, uint32_t value1, uint32_t value2, uint32_t value3, uint32_t value4) {
	uint32_t *matparam = (uint32_t*)_material_get_ptr(material, name, sizeof(uint32_t)*4);
	if (matparam != nullptr) {
		matparam[0] = value1;
		matparam[1] = value2;
		matparam[2] = value3;
		matparam[3] = value4;
		material->args.buffer_dirty = true;
	}
}

///////////////////////////////////////////

void material_set_matrix(material_t material, const char *name, matrix value) {
	int32_t i = skg_shader_get_var_index(&material->shader->shader, name);
	if (i == -1) return;

	const skg_shader_var_t *info = skg_shader_get_var_info(&material->shader->shader, i);
	if (info->size != sizeof(matrix)) {
		log_errf("material_set_: '%s' mismatched type (for shader %s)",  info->name, material->shader->shader.meta->name);
		return;
	}
	*(matrix *)((uint8_t*)material->args.buffer + info->offset) = value;
	material->args.buffer_dirty = true;
}

///////////////////////////////////////////

bool32_t material_set_texture_id(material_t material, uint64_t id, tex_t value) {

	for (uint32_t i = 0; i < material->shader->shader.meta->resource_count; i++) {
		const skg_shader_resource_t *resource = &material->shader->shader.meta->resources[i];
		if (resource->name_hash == id) {

			// Assigning a null texture will crash the renderer, so we want to
			// instead find the default texture for the material parameter.
			if (value == nullptr) {
				if      (string_eq(resource->extra, "white")) value = sk_default_tex;
				else if (string_eq(resource->extra, "black")) value = sk_default_tex_black;
				else if (string_eq(resource->extra, "gray" )) value = sk_default_tex_gray;
				else if (string_eq(resource->extra, "flat" )) value = sk_default_tex_flat;
				else if (string_eq(resource->extra, "rough")) value = sk_default_tex_rough;
				else                                          value = sk_default_tex;
			}

			if (material->args.textures[i].tex != value) {
				// No need for safe swap, since we know these textures aren't
				// the same texture.
				if (material->args.textures[i].tex != nullptr)
					tex_release(material->args.textures[i].tex);
				material->args.textures[i].tex = value;
				tex_addref(value);

				// Information about the texture needs updated as well, but
				// this is done when checking the material before rendering,
				// since the texture's internal contents can change at any time
			}
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////

bool32_t material_set_texture(material_t material, const char *name, tex_t value) {
	uint64_t id = hash_fnv64_string(name);
	return material_set_texture_id(material, id, value);
}

///////////////////////////////////////////

float material_get_float(material_t material, const char* name) {
	float* matparam = (float*)_material_get_ptr(material, name, sizeof(float));
	return matparam != nullptr ? *matparam : 0.0f;
}

///////////////////////////////////////////

vec2 material_get_vector2(material_t material, const char* name) {
	vec2* matparam = (vec2*)_material_get_ptr(material, name, sizeof(vec2));
	return matparam != nullptr ? *matparam : vec2{};
}

///////////////////////////////////////////

vec3 material_get_vector3(material_t material, const char* name) {
	vec3* matparam = (vec3*)_material_get_ptr(material, name, sizeof(vec3));
	return matparam != nullptr ? *matparam : vec3{};
}

///////////////////////////////////////////

color128 material_get_color(material_t material, const char* name) {
	color128* matparam = (color128*)_material_get_ptr(material, name, sizeof(color128));
	return matparam != nullptr ? *matparam : color128{1,1,1,1};
}

///////////////////////////////////////////

vec4 material_get_vector4(material_t material, const char* name) {
	vec4* matparam = (vec4*)_material_get_ptr(material, name, sizeof(vec4));
	return matparam != nullptr ? *matparam : vec4{};
}

///////////////////////////////////////////

int32_t material_get_int(material_t material, const char* name) {
	int32_t* matparam = (int32_t*)_material_get_ptr(material, name, sizeof(int32_t));
	return matparam != nullptr ? *matparam : 0;
}

///////////////////////////////////////////

bool32_t material_get_bool(material_t material, const char* name) {
	uint32_t* matparam = (uint32_t*)_material_get_ptr(material, name, sizeof(uint32_t));
	return matparam != nullptr ? *matparam : false;
}

///////////////////////////////////////////

uint32_t material_get_uint(material_t material, const char* name) {
	uint32_t* matparam = (uint32_t*)_material_get_ptr(material, name, sizeof(uint32_t));
	return matparam != nullptr ? *matparam : 0;
}

///////////////////////////////////////////

matrix material_get_matrix(material_t material, const char* name) {
	matrix* matparam = (matrix*)_material_get_ptr(material, name, sizeof(matrix));
	return matparam != nullptr ? *matparam : matrix_identity;
}

///////////////////////////////////////////

tex_t material_get_texture(material_t material, const char* name) {
	uint64_t id = hash_fnv64_string(name);
	for (uint32_t i = 0; i < material->shader->shader.meta->resource_count; i++) {
		const skg_shader_resource_t* resource = &material->shader->shader.meta->resources[i];
		if (resource->name_hash == id) {
			tex_t result = material->args.textures[i].tex;
			tex_addref(result);
			return result;
		}
	}
	return nullptr;
}

///////////////////////////////////////////

bool32_t material_has_param(material_t material, const char *name, material_param_ type) {
	uint64_t id = hash_fnv64_string(name);

	if (type == material_param_texture) {
		for (uint32_t i = 0; i < material->shader->shader.meta->resource_count; i++) {
			if (material->shader->shader.meta->resources[i].name_hash == id)
				return true;
		}
	} else {
		if (skg_shader_get_var_index(&material->shader->shader, name) != -1)
			return true;
	}
	return false;
}

///////////////////////////////////////////

void material_set_param(material_t material, const char *name, material_param_ type, const void *value) {
	material_set_param_id(material, hash_fnv64_string(name), type, value);
}

///////////////////////////////////////////

void material_set_param_id(material_t material, uint64_t id, material_param_ type, const void *value) {
	if (type == material_param_texture) {
		material_set_texture_id(material, id, (tex_t)value);
	} else {
		int32_t i = skg_shader_get_var_index_h(&material->shader->shader, id);
		if (i != -1) {
			const skg_shader_var_t *info = skg_shader_get_var_info(&material->shader->shader, i);
			memcpy(((uint8_t *)material->args.buffer + info->offset), value, info->size);
			material->args.buffer_dirty = true;
		}
	}
}

///////////////////////////////////////////

bool32_t material_get_param(material_t material, const char *name, material_param_ type, void *out_value) {
	return material_get_param_id(material, hash_fnv64_string(name), type, out_value);
}

///////////////////////////////////////////

bool32_t material_get_param_id(material_t material, uint64_t id, material_param_ type, void *out_value) {
	if (type == material_param_texture) {
		for (uint32_t i = 0; i < material->shader->shader.meta->resource_count; i++) {
			if (material->shader->shader.meta->resources[i].name_hash == id) {
				memcpy(out_value, &material->args.textures[i], sizeof(tex_t));
				return true;
			}
		}
	} else {
		int32_t i = skg_shader_get_var_index_h(&material->shader->shader, id);
		if (i != -1) {
			const skg_shader_var_t *info = skg_shader_get_var_info(&material->shader->shader, i);
			memcpy(out_value, ((uint8_t *)material->args.buffer + info->offset), info->size);
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////

void material_get_param_info(material_t material, int32_t index, char **out_name, material_param_ *out_type) {
	const skg_shader_meta_t *meta = material->shader->shader.meta;

	int32_t buffer_id = meta->global_buffer_id;
	int32_t buffer_ct = buffer_id >= 0
		? meta->buffers[buffer_id].var_count
		: 0;

	if (index < buffer_ct) {
		skg_shader_var_t *info = &meta->buffers[buffer_id].vars[index];
		if (out_type != nullptr) {
			*out_type = material_param_unknown;
			if (info->type == skg_shader_var_float) {
				if      (info->type_count == 16) *out_type = material_param_matrix;
				else if (info->type_count == 1 ) *out_type = material_param_float;
				else if (info->type_count == 2 ) *out_type = material_param_vector2;
				else if (info->type_count == 3 ) *out_type = material_param_vector3;
				else if (info->type_count == 4 ) {
					if (string_eq_nocase(info->extra, "color"))
						*out_type = material_param_color128;
					else
						*out_type = material_param_vector4;
				}
			} else if (info->type == skg_shader_var_int) {
				if      (info->type_count == 1 ) *out_type = material_param_int;
				else if (info->type_count == 2 ) *out_type = material_param_int2;
				else if (info->type_count == 3 ) *out_type = material_param_int3;
				else if (info->type_count == 4 ) *out_type = material_param_int4;
			} else if (info->type == skg_shader_var_uint) {
				if      (info->type_count == 1 ) *out_type = material_param_uint;
				else if (info->type_count == 2 ) *out_type = material_param_uint2;
				else if (info->type_count == 3 ) *out_type = material_param_uint3;
				else if (info->type_count == 4 ) *out_type = material_param_uint4;
			}
		}
		if (out_name != nullptr) *out_name = info->name;
	} else {
		if (out_type != nullptr) *out_type = material_param_texture;
		if (out_name != nullptr) *out_name = meta->resources[index-buffer_ct].name;
	}
}

///////////////////////////////////////////

int material_get_param_count(material_t material) {
	int32_t buffer_id = material->shader->shader.meta->global_buffer_id;
	if (buffer_id == -1)
		return material->shader->shader.meta->resource_count;

	return
		material->shader->shader.meta->buffers[buffer_id].var_count +
		material->shader->shader.meta->resource_count;
}

///////////////////////////////////////////

material_buffer_t material_buffer_create(int32_t register_slot, int32_t size) {
	if (register_slot < 1 || register_slot == 2 || register_slot >= (sizeof(material_buffers)/sizeof(material_buffers[0]))) {
		log_errf("material_buffer_create: bad slot id '%d', use 3-%d.", register_slot, (sizeof(material_buffers)/sizeof(material_buffers[0])) - 1);
		return nullptr;
	}
	if (material_buffers[register_slot].size != 0) {
		log_errf("material_buffer_create: slot id '%d' is already in use.", register_slot);
		return nullptr;
	}
	material_buffers[register_slot].buffer = skg_buffer_create(nullptr, 1, size, skg_buffer_type_constant, skg_use_dynamic);
	material_buffers[register_slot].size   = size;
#if !defined(SKG_OPENGL) && (defined(_DEBUG) || defined(SK_GPU_LABELS))
	char name[64];
	snprintf(name, sizeof(name), "render/material_buffer/register_%d", register_slot);
	skg_buffer_name(&material_buffers[register_slot].buffer, name);
#endif
	return &material_buffers[register_slot];
}

///////////////////////////////////////////

void material_buffer_set_data(material_buffer_t buffer, const void *data) {
	skg_buffer_set_contents(&buffer->buffer, data, buffer->size);
}

///////////////////////////////////////////

void material_check_tex_changes(material_t material) {
	// Textures that progressively load or swap from fallbacks will change
	// their dimensions. SK provides dimensions via the texname_i variable, so
	// we're making sure that stays in sync here.
	for (int32_t i = 0; i < material->args.texture_count; i++) {
		shaderargs_tex_t *curr = &material->args.textures[i];
		if (curr->meta_hash != curr->tex->meta_hash) {
			tex_t physical_tex = curr->tex->fallback == nullptr
				? curr->tex
				: curr->tex->fallback;
			curr->meta_hash = curr->tex->meta_hash;

			uint64_t tex_info_hash = hash_fnv64_string("_i", material->shader->shader.meta->resources[i].name_hash);
			vec4     info = { (float)physical_tex->width, (float)physical_tex->height, (float)(uint32_t)log2(physical_tex->width), 0 };
			material_set_param_id(material, tex_info_hash, material_param_vector4, &info);
		}
	}
}

///////////////////////////////////////////

void material_check_dirty(material_t material) {
	material_check_tex_changes(material);

	if (material->args.buffer_dirty == false || material->args.buffer == nullptr) return;

	skg_buffer_set_contents(&material->args.buffer_gpu, material->args.buffer, (uint32_t)material->args.buffer_size);
	material->args.buffer_dirty = false;
}

///////////////////////////////////////////

void material_buffer_release(material_buffer_t buffer) {
	skg_buffer_destroy(&buffer->buffer);
	*buffer = {};
}

} // namespace sk