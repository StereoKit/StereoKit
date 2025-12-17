#include "material.h"
#include "shader.h"
#include "texture.h"
#include "../libraries/stref.h"
#include "../libraries/array.h"
#include "../libraries/atomic_util.h"
#include "../sk_memory.h"
#include "../systems/defaults.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

namespace sk {


///////////////////////////////////////////

skr_material_info_t material_build_info(material_t material) {
	skr_material_info_t info = {};
	info.shader = &material->shader->gpu_shader;

	switch (material->cull) {
	case cull_back:  info.cull = skr_cull_back;  break;
	case cull_front: info.cull = skr_cull_front; break;
	case cull_none:  info.cull = skr_cull_none;  break;
	default:         info.cull = skr_cull_back;  break;
	}

	switch (material->depth_test) {
	case depth_test_less:         info.depth_test = skr_compare_less;          break;
	case depth_test_less_or_eq:   info.depth_test = skr_compare_less_or_eq;    break;
	case depth_test_greater:      info.depth_test = skr_compare_greater;       break;
	case depth_test_greater_or_eq:info.depth_test = skr_compare_greater_or_eq; break;
	case depth_test_equal:        info.depth_test = skr_compare_equal;         break;
	case depth_test_not_equal:    info.depth_test = skr_compare_not_equal;     break;
	case depth_test_always:       info.depth_test = skr_compare_always;        break;
	case depth_test_never:        info.depth_test = skr_compare_never;         break;
	default:                      info.depth_test = skr_compare_less;          break;
	}

	switch (material->alpha_mode) {
	case transparency_none:  info.blend_state = skr_blend_off;      break;
	case transparency_blend: info.blend_state = skr_blend_alpha;    break;
	case transparency_add:   info.blend_state = skr_blend_additive; break;
	default:                 info.blend_state = skr_blend_off;      break;
	}

	info.write_mask        = material->depth_write ? skr_write_default : skr_write_rgba;
	info.alpha_to_coverage = material->alpha_mode == transparency_msaa;
	info.queue_offset      = material->queue_offset;
	return info;
}

///////////////////////////////////////////

void material_recreate_gpu(material_t material) {
	// Create new material first (allows pipeline reuse if identical)
	skr_material_t      new_mat = {};
	skr_material_info_t info    = material_build_info(material);
	if (skr_material_create(info, &new_mat) != skr_err_success) {
		log_err("Failed to recreate GPU material");
		return;
	}

	// Destroy old material
	if (skr_material_is_valid(&material->gpu_mat)) {
		skr_material_destroy(&material->gpu_mat);
	}

	material->gpu_mat = new_mat;
}

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
}

///////////////////////////////////////////

const char* material_get_id(const material_t material) {
	return material->header.id_text;
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

void material_alloc_textures(material_t material) {
	const sksc_shader_meta_t *meta = material->shader->gpu_shader.meta;
	material->texture_count       = (int32_t)meta->resource_count;
	material->textures            = material->texture_count > 0 ? sk_malloc_t(tex_t,    material->texture_count) : nullptr;
	material->texture_meta_hashes = material->texture_count > 0 ? sk_malloc_t(uint64_t, material->texture_count) : nullptr;
	memset(material->textures,            0, sizeof(tex_t)    * material->texture_count);
	memset(material->texture_meta_hashes, 0, sizeof(uint64_t) * material->texture_count);
}

///////////////////////////////////////////

void material_set_default_textures(material_t material) {
	const sksc_shader_meta_t *meta = material->shader->gpu_shader.meta;

	// Set default textures for all resources declared in the shader
	for (uint32_t i = 0; i < meta->resource_count; i++) {
		tex_t default_tex = nullptr;

		if      (string_eq(meta->resources[i].value, "white")) default_tex = tex_find(default_id_tex);
		else if (string_eq(meta->resources[i].value, "black")) default_tex = tex_find(default_id_tex_black);
		else if (string_eq(meta->resources[i].value, "gray" )) default_tex = tex_find(default_id_tex_gray);
		else if (string_eq(meta->resources[i].value, "flat" )) default_tex = tex_find(default_id_tex_flat);
		else if (string_eq(meta->resources[i].value, "rough")) default_tex = tex_find(default_id_tex_rough);
		else                                                   default_tex = tex_find(default_id_tex);

		// Release old texture if present, store new one
		if (material->textures[i] != nullptr)
			tex_release(material->textures[i]);
		material->textures[i]            = default_tex; // tex_find already adds a ref
		material->texture_meta_hashes[i] = default_tex->meta_hash;

		// Update sk_renderer binding
		tex_t physical_tex = default_tex->fallback ? default_tex->fallback : default_tex;
		skr_material_set_tex(&material->gpu_mat, meta->resources[i].name, &physical_tex->gpu_tex);
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
	result->alpha_mode   = transparency_none;
	result->shader       = material_shader;
	result->depth_test   = depth_test_less;
	result->depth_write  = true;
	result->depth_clip   = false;
	result->cull         = cull_back;
	result->queue_offset = 0;

	// Create the sk_renderer material
	if (skr_material_create(material_build_info(result), &result->gpu_mat) != skr_err_success) {
		log_err("Failed to create GPU material");
	}

	material_alloc_textures      (result);
	material_set_default_textures(result);

	if (shader == nullptr) {
		shader_release(material_shader);
	}

	return result;
}

///////////////////////////////////////////

material_t material_copy(material_t material) {
	if (material == nullptr) return nullptr;

	// Allocate new material
	material_t result = (material_t)assets_allocate(asset_type_material);

	// Copy cached state
	result->alpha_mode   = material->alpha_mode;
	result->cull         = material->cull;
	result->depth_test   = material->depth_test;
	result->depth_write  = material->depth_write;
	result->depth_clip   = material->depth_clip;
	result->queue_offset = material->queue_offset;
	result->chain        = material->chain;
	memcpy(result->variants, material->variants, sizeof(result->variants));

	// Add references to shared assets
	shader_addref(material->shader);
	result->shader = material->shader;
	if (result->chain != nullptr) material_addref(result->chain);
	for (int32_t i = 0; i < (int32_t)_countof(result->variants); i++) {
		if (result->variants[i] != nullptr)
			material_addref(result->variants[i]);
	}

	// Create the sk_renderer material with copied state
	skr_material_info_t info = material_build_info(result);
	if (skr_material_create(info, &result->gpu_mat) != skr_err_success) {
		log_err("Failed to create GPU material during copy");
	}

	// Allocate texture array for result
	material_alloc_textures(result);

	// Copy parameter values from source material
	const sksc_shader_meta_t   *meta      = material->shader->gpu_shader.meta;
	const sksc_shader_buffer_t *buff_info = meta->global_buffer_id != -1
		? &meta->buffers[meta->global_buffer_id]
		: nullptr;
	if (buff_info && buff_info->size > 0) {
		// Get param values from source and set on dest
		for (int32_t i = 0; i < buff_info->var_count; i++) {
			const sksc_shader_var_t *var = &buff_info->vars[i];
			void *tmp = sk_malloc(var->size);
			skr_material_get_param(&material->gpu_mat, var->name, (sksc_shader_var_)var->type, (uint32_t)var->type_count, tmp);
			skr_material_set_param(&result->gpu_mat, var->name, (sksc_shader_var_)var->type, (uint32_t)var->type_count, tmp);
			sk_free(tmp);
		}
	}

	// Copy texture bindings from source material
	for (int32_t i = 0; i < material->texture_count; i++) {
		if (material->textures[i] != nullptr) {
			tex_addref(material->textures[i]);
			result->textures[i]            = material->textures[i];
			result->texture_meta_hashes[i] = material->texture_meta_hashes[i];

			tex_t physical_tex = material->textures[i]->fallback
				? material->textures[i]->fallback
				: material->textures[i];
			skr_material_set_tex(&result->gpu_mat, meta->resources[i].name, &physical_tex->gpu_tex);
		}
	}

	return result;
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
	for (size_t i = 0; i < _countof(material->variants); i++) {
		if (material->variants[i])
			material_release(material->variants[i]);
	}
	if (material->chain) material_release(material->chain);
	shader_release(material->shader);

	// Release all texture references
	for (int32_t i = 0; i < material->texture_count; i++) {
		if (material->textures[i] != nullptr)
			tex_release(material->textures[i]);
	}
	sk_free(material->textures);
	sk_free(material->texture_meta_hashes);

	skr_material_destroy(&material->gpu_mat);
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
	shader_t old_shader = material->shader;

	// Copy param values from old shader to temp storage
	const sksc_shader_meta_t*   old_meta = old_shader                                   ? old_shader->gpu_shader.meta                    : nullptr;
	const sksc_shader_buffer_t* old_buff = old_meta && old_meta->global_buffer_id != -1 ? &old_meta->buffers[old_meta->global_buffer_id] : nullptr;

	// Store old param values temporarily
	struct param_copy_t { char name[64]; void *data; uint32_t size; sksc_shader_var_ type; uint32_t count; };
	param_copy_t *old_params      = nullptr;
	int32_t       old_param_count = 0;
	if (old_buff && old_buff->var_count > 0) {
		old_params      = sk_malloc_t(param_copy_t, old_buff->var_count);
		old_param_count = old_buff->var_count;

		for (int32_t i = 0; i < old_buff->var_count; i++) {
			const sksc_shader_var_t *var = &old_buff->vars[i];
			strncpy(old_params[i].name, var->name, sizeof(old_params[i].name) - 1);
			old_params[i].name[sizeof(old_params[i].name) - 1] = '\0';
			old_params[i].size  = var->size;
			old_params[i].type  = (sksc_shader_var_)var->type;
			old_params[i].count = var->type_count;
			old_params[i].data  = sk_malloc(var->size);
			skr_material_get_param(&material->gpu_mat, var->name, (sksc_shader_var_)var->type, var->type_count, old_params[i].data);
		}
	}

	// Store old textures with their name hashes (keep refs for now)
	struct tex_copy_t { id_hash_t name_hash; tex_t tex; };
	tex_copy_t *old_textures  = nullptr;
	int32_t     old_tex_count = material->texture_count;
	if (old_meta && old_tex_count > 0) {
		old_textures = sk_malloc_t(tex_copy_t, old_tex_count);
		for (int32_t i = 0; i < old_tex_count; i++) {
			old_textures[i].name_hash = old_meta->resources[i].name_hash;
			old_textures[i].tex       = material->textures[i]; // Transfer ref ownership
		}
	}

	// Free arrays (refs transferred to old_textures)
	sk_free(material->textures);
	sk_free(material->texture_meta_hashes);
	material->textures            = nullptr;
	material->texture_meta_hashes = nullptr;
	material->texture_count       = 0;

	// Update shader and recreate gpu material
	material->shader = shader;
	material_recreate_gpu(material);

	// Allocate new texture array and set defaults
	material_alloc_textures      (material);
	material_set_default_textures(material);

	// Re-bind matching textures from old shader
	if (old_textures) {
		for (int32_t i = 0; i < old_tex_count; i++) {
			if (old_textures[i].tex != nullptr) {
				// Try to find matching texture slot in new shader by name hash
				material_set_texture_id(material, old_textures[i].name_hash, old_textures[i].tex);
				tex_release(old_textures[i].tex);
			}
		}
		sk_free(old_textures);
	}

	// Copy matching params to new shader
	if (old_params) {
		for (int32_t i = 0; i < old_param_count; i++) {
			// Try to set the param - sk_renderer will ignore if name doesn't exist
			skr_material_set_param(&material->gpu_mat, old_params[i].name, old_params[i].type, old_params[i].count, old_params[i].data);
			sk_free(old_params[i].data);
		}
		sk_free(old_params);
	}

	// Release old shader
	if (old_shader != nullptr)
		shader_release(old_shader);
}

///////////////////////////////////////////

shader_t material_get_shader(material_t material) {
	shader_addref(material->shader);
	return material->shader;
}

///////////////////////////////////////////

void material_set_transparency(material_t material, transparency_ mode) {
	if (material->alpha_mode == mode) return;
	material->alpha_mode = mode;
	material_recreate_gpu(material);
}

///////////////////////////////////////////

void material_set_cull(material_t material, cull_ mode) {
	if (material->cull == mode) return;
	material->cull = mode;
	material_recreate_gpu(material);
}

///////////////////////////////////////////

void material_set_wireframe(material_t material, bool32_t wireframe) {
	// sk_renderer doesn't support wireframe, so we just ignore this
	// Keep the API for compatibility but don't store the value
	(void)material;
	(void)wireframe;
}

///////////////////////////////////////////

void material_set_depth_test(material_t material, depth_test_ depth_test_mode) {
	if (material->depth_test == depth_test_mode) return;
	material->depth_test = depth_test_mode;
	material_recreate_gpu(material);
}

///////////////////////////////////////////

void material_set_depth_write(material_t material, bool32_t write_enabled) {
	if (material->depth_write == write_enabled) return;
	material->depth_write = write_enabled;
	material_recreate_gpu(material);
}

///////////////////////////////////////////

void material_set_depth_clip(material_t material, bool32_t clip_enabled) {
	if (material->depth_clip == clip_enabled) return;
	material->depth_clip = clip_enabled;
	// TODO: verify sk_renderer supports depth clip
	material_recreate_gpu(material);
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

void material_set_variant(material_t material, int32_t variant_idx, material_t variant_material) {
	if (variant_idx < 1) {
		log_err("Variant index must be 1 or greater!");
		return;
	} else if (variant_idx > _countof(material->variants)) {
		log_errf("Variant index must be less than %d!", _countof(material->variants)+1);
		return;
	}

	int idx = variant_idx - 1;
	if (variant_material       ) material_addref (variant_material);
	if (material->variants[idx]) material_release(material->variants[idx]);
	material->variants[idx] = variant_material;
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
	// sk_renderer doesn't support wireframe
	(void)material;
	return false;
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

bool32_t material_get_depth_clip(material_t material) {
	return material->depth_clip;
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

material_t material_get_variant(material_t material, int32_t variant_idx) {
	if (variant_idx < 1) {
		log_err("Variant index must be 1 or greater!");
		return nullptr;
	} else if (variant_idx > _countof(material->variants)) {
		log_errf("Variant index must be less than %d!", _countof(material->variants)+1);
		return nullptr;
	}
	int idx = variant_idx - 1;
	if (material->variants[idx]) material_addref(material->variants[idx]);
	return material->variants[idx];
}

///////////////////////////////////////////

void material_set_float(material_t material, const char *name, float value) {
	skr_material_set_param(&material->gpu_mat, name, sksc_shader_var_float, 1, &value);
}

///////////////////////////////////////////

void material_set_color32(material_t material, const char *name, color32 value) {
	color128 linear = color_to_linear(color32_to_128(value));
	skr_material_set_param(&material->gpu_mat, name, sksc_shader_var_float, 4, &linear);
}

///////////////////////////////////////////

void material_set_color(material_t material, const char *name, color128 value) {
	color128 linear = color_to_linear(value);
	skr_material_set_param(&material->gpu_mat, name, sksc_shader_var_float, 4, &linear);
}

///////////////////////////////////////////

void material_set_vector4(material_t material, const char *name, vec4 value) {
	skr_material_set_param(&material->gpu_mat, name, sksc_shader_var_float, 4, &value);
}

///////////////////////////////////////////

void material_set_vector3(material_t material, const char *name, vec3 value) {
	skr_material_set_param(&material->gpu_mat, name, sksc_shader_var_float, 3, &value);
}

///////////////////////////////////////////

void material_set_vector2(material_t material, const char *name, vec2 value) {
	skr_material_set_param(&material->gpu_mat, name, sksc_shader_var_float, 2, &value);
}

///////////////////////////////////////////

void material_set_int(material_t material, const char *name, int32_t value) {
	skr_material_set_param(&material->gpu_mat, name, sksc_shader_var_int, 1, &value);
}

///////////////////////////////////////////

void material_set_int2(material_t material, const char *name, int32_t value1, int32_t value2) {
	int32_t values[2] = { value1, value2 };
	skr_material_set_param(&material->gpu_mat, name, sksc_shader_var_int, 2, values);
}

///////////////////////////////////////////

void material_set_int3(material_t material, const char *name, int32_t value1, int32_t value2, int32_t value3) {
	int32_t values[3] = { value1, value2, value3 };
	skr_material_set_param(&material->gpu_mat, name, sksc_shader_var_int, 3, values);
}

///////////////////////////////////////////

void material_set_int4(material_t material, const char *name, int32_t value1, int32_t value2, int32_t value3, int32_t value4) {
	int32_t values[4] = { value1, value2, value3, value4 };
	skr_material_set_param(&material->gpu_mat, name, sksc_shader_var_int, 4, values);
}

///////////////////////////////////////////

void material_set_bool(material_t material, const char *name, bool32_t value) {
	uint32_t v = value > 0 ? 1 : 0;
	skr_material_set_param(&material->gpu_mat, name, sksc_shader_var_uint, 1, &v);
}

///////////////////////////////////////////

void material_set_uint(material_t material, const char *name, uint32_t value) {
	skr_material_set_param(&material->gpu_mat, name, sksc_shader_var_uint, 1, &value);
}

///////////////////////////////////////////

void material_set_uint2(material_t material, const char *name, uint32_t value1, uint32_t value2) {
	uint32_t values[2] = { value1, value2 };
	skr_material_set_param(&material->gpu_mat, name, sksc_shader_var_uint, 2, values);
}

///////////////////////////////////////////

void material_set_uint3(material_t material, const char *name, uint32_t value1, uint32_t value2, uint32_t value3) {
	uint32_t values[3] = { value1, value2, value3 };
	skr_material_set_param(&material->gpu_mat, name, sksc_shader_var_uint, 3, values);
}

///////////////////////////////////////////

void material_set_uint4(material_t material, const char *name, uint32_t value1, uint32_t value2, uint32_t value3, uint32_t value4) {
	uint32_t values[4] = { value1, value2, value3, value4 };
	skr_material_set_param(&material->gpu_mat, name, sksc_shader_var_uint, 4, values);
}

///////////////////////////////////////////

void material_set_matrix(material_t material, const char *name, matrix value) {
	skr_material_set_param(&material->gpu_mat, name, sksc_shader_var_float, 16, &value);
}

///////////////////////////////////////////

bool32_t material_set_texture_id(material_t material, id_hash_t id, tex_t value) {
	const sksc_shader_meta_t *meta = material->shader->gpu_shader.meta;

	for (uint32_t i = 0; i < meta->resource_count; i++) {
		const sksc_shader_resource_t *resource = &meta->resources[i];
		if (resource->name_hash == id) {

			// Assigning a null texture will crash the renderer, so we want to
			// instead find the default texture for the material parameter.
			tex_t tex_to_set = value;
			if (tex_to_set == nullptr) {
				if      (string_eq(resource->value, "white")) tex_to_set = sk_default_tex;
				else if (string_eq(resource->value, "black")) tex_to_set = sk_default_tex_black;
				else if (string_eq(resource->value, "gray" )) tex_to_set = sk_default_tex_gray;
				else if (string_eq(resource->value, "flat" )) tex_to_set = sk_default_tex_flat;
				else if (string_eq(resource->value, "rough")) tex_to_set = sk_default_tex_rough;
				else                                          tex_to_set = sk_default_tex;
			}

			// Update texture reference tracking
			if (material->textures[i] != tex_to_set) {
				tex_addref(tex_to_set);
				if (material->textures[i] != nullptr)
					tex_release(material->textures[i]);
				material->textures[i]            = tex_to_set;
				material->texture_meta_hashes[i] = tex_to_set->meta_hash;
			}

			// Set the GPU texture binding (use fallback if present)
			tex_t physical_tex = tex_to_set->fallback ? tex_to_set->fallback : tex_to_set;
			skr_material_set_tex(&material->gpu_mat, resource->name, &physical_tex->gpu_tex);
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////

bool32_t material_set_texture(material_t material, const char *name, tex_t value) {
	return material_set_texture_id(material, hash_string(name), value);
}

///////////////////////////////////////////

float material_get_float(material_t material, const char* name) {
	float result = 0.0f;
	skr_material_get_param(&material->gpu_mat, name, sksc_shader_var_float, 1, &result);
	return result;
}

///////////////////////////////////////////

vec2 material_get_vector2(material_t material, const char* name) {
	vec2 result = {};
	skr_material_get_param(&material->gpu_mat, name, sksc_shader_var_float, 2, &result);
	return result;
}

///////////////////////////////////////////

vec3 material_get_vector3(material_t material, const char* name) {
	vec3 result = {};
	skr_material_get_param(&material->gpu_mat, name, sksc_shader_var_float, 3, &result);
	return result;
}

///////////////////////////////////////////

color128 material_get_color(material_t material, const char* name) {
	color128 result = {1,1,1,1};
	skr_material_get_param(&material->gpu_mat, name, sksc_shader_var_float, 4, &result);
	return result;
}

///////////////////////////////////////////

vec4 material_get_vector4(material_t material, const char* name) {
	vec4 result = {};
	skr_material_get_param(&material->gpu_mat, name, sksc_shader_var_float, 4, &result);
	return result;
}

///////////////////////////////////////////

int32_t material_get_int(material_t material, const char* name) {
	int32_t result = 0;
	skr_material_get_param(&material->gpu_mat, name, sksc_shader_var_int, 1, &result);
	return result;
}

///////////////////////////////////////////

bool32_t material_get_bool(material_t material, const char* name) {
	uint32_t result = 0;
	skr_material_get_param(&material->gpu_mat, name, sksc_shader_var_uint, 1, &result);
	return result != 0;
}

///////////////////////////////////////////

uint32_t material_get_uint(material_t material, const char* name) {
	uint32_t result = 0;
	skr_material_get_param(&material->gpu_mat, name, sksc_shader_var_uint, 1, &result);
	return result;
}

///////////////////////////////////////////

matrix material_get_matrix(material_t material, const char* name) {
	matrix result = matrix_identity;
	skr_material_get_param(&material->gpu_mat, name, sksc_shader_var_float, 16, &result);
	return result;
}

///////////////////////////////////////////

tex_t material_get_texture(material_t material, const char* name) {
	const sksc_shader_meta_t *meta = material->shader->gpu_shader.meta;
	id_hash_t id = hash_string(name);

	for (uint32_t i = 0; i < meta->resource_count; i++) {
		if (meta->resources[i].name_hash == id) {
			tex_t result = material->textures[i];
			if (result != nullptr)
				tex_addref(result);
			return result;
		}
	}
	return nullptr;
}

///////////////////////////////////////////

bool32_t material_has_param(material_t material, const char *name, material_param_ type) {
	id_hash_t id = hash_string(name);

	if (type == material_param_texture) {
		for (uint32_t i = 0; i < material->shader->gpu_shader.meta->resource_count; i++) {
			if (material->shader->gpu_shader.meta->resources[i].name_hash == id)
				return true;
		}
	} else {
		if (sksc_shader_meta_get_var_index(material->shader->gpu_shader.meta, name) != -1)
			return true;
	}
	return false;
}

///////////////////////////////////////////

void material_set_param(material_t material, const char *name, material_param_ type, const void *value) {
	material_set_param_id(material, hash_string(name), type, value);
}

///////////////////////////////////////////

void material_set_param_id(material_t material, id_hash_t id, material_param_ type, const void *value) {
	if (type == material_param_texture) {
		material_set_texture_id(material, id, (tex_t)value);
	} else {
		int32_t i = sksc_shader_meta_get_var_index_h(material->shader->gpu_shader.meta, id);
		if (i != -1) {
			const sksc_shader_var_t *info = sksc_shader_meta_get_var_info(material->shader->gpu_shader.meta, i);
			skr_material_set_param(&material->gpu_mat, info->name, (sksc_shader_var_)info->type, info->type_count, value);
		}
	}
}

///////////////////////////////////////////

bool32_t material_get_param(material_t material, const char *name, material_param_ type, void *out_value) {
	return material_get_param_id(material, hash_string(name), type, out_value);
}

///////////////////////////////////////////

bool32_t material_get_param_id(material_t material, id_hash_t id, material_param_ type, void *out_value) {
	if (type == material_param_texture) {
		const sksc_shader_meta_t *meta = material->shader->gpu_shader.meta;
		for (uint32_t i = 0; i < meta->resource_count; i++) {
			if (meta->resources[i].name_hash == id) {
				tex_t result = material->textures[i];
				if (result != nullptr)
					tex_addref(result);
				*(tex_t*)out_value = result;
				return true;
			}
		}
		*(tex_t*)out_value = nullptr;
		return false;
	} else {
		int32_t i = sksc_shader_meta_get_var_index_h(material->shader->gpu_shader.meta, id);
		if (i != -1) {
			const sksc_shader_var_t *info = sksc_shader_meta_get_var_info(material->shader->gpu_shader.meta, i);
			skr_material_get_param(&material->gpu_mat, info->name, (sksc_shader_var_)info->type, info->type_count, out_value);
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////

void material_get_param_info(material_t material, int32_t index, char **out_name, material_param_ *out_type) {
	const sksc_shader_meta_t *meta = material->shader->gpu_shader.meta;

	int32_t buffer_id = meta->global_buffer_id;
	int32_t buffer_ct = buffer_id >= 0
		? meta->buffers[buffer_id].var_count
		: 0;

	if (index < buffer_ct) {
		sksc_shader_var_t *info = &meta->buffers[buffer_id].vars[index];
		if (out_type != nullptr) {
			*out_type = material_param_unknown;
			if (info->type == sksc_shader_var_float) {
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
			} else if (info->type == sksc_shader_var_int) {
				if      (info->type_count == 1 ) *out_type = material_param_int;
				else if (info->type_count == 2 ) *out_type = material_param_int2;
				else if (info->type_count == 3 ) *out_type = material_param_int3;
				else if (info->type_count == 4 ) *out_type = material_param_int4;
			} else if (info->type == sksc_shader_var_uint) {
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
	int32_t buffer_id = material->shader->gpu_shader.meta->global_buffer_id;
	if (buffer_id == -1)
		return material->shader->gpu_shader.meta->resource_count;

	return
		material->shader->gpu_shader.meta->buffers[buffer_id].var_count +
		material->shader->gpu_shader.meta->resource_count;
}

///////////////////////////////////////////

material_buffer_t material_buffer_create(int32_t size) {
	_material_buffer_t* buffer = sk_malloc_t(_material_buffer_t, 1);
	buffer->size = size;
	buffer->refs = 1;
	void* data = sk_calloc(size);
	if (skr_buffer_create(data, (uint32_t)size, 1, skr_buffer_type_constant, skr_use_dynamic, &buffer->buffer) != skr_err_success) {
		log_err("Failed to create material buffer");
	}
	sk_free(data);
	return buffer;
}

///////////////////////////////////////////

void material_buffer_addref(material_buffer_t buffer) {
	atomic_increment(&buffer->refs);
}

///////////////////////////////////////////

void material_buffer_destroy(material_buffer_t buffer) {
	skr_buffer_destroy(&buffer->buffer);
	*buffer = {};
	sk_free(buffer);
}

///////////////////////////////////////////

void material_buffer_release(material_buffer_t buffer) {
	if (buffer && atomic_decrement(&buffer->refs) == 0) {
		material_buffer_destroy(buffer);
	}
}

///////////////////////////////////////////

void material_buffer_set_data(material_buffer_t buffer, const void *data) {
	skr_buffer_set(&buffer->buffer, data, (uint32_t)buffer->size);
}

///////////////////////////////////////////

void material_check_dirty(material_t material) {
	// Textures that progressively load or swap from fallbacks will change
	// their dimensions. SK provides dimensions via the texname_i variable, so
	// we're making sure that stays in sync here.
	const sksc_shader_meta_t *meta = material->shader->gpu_shader.meta;

	for (int32_t i = 0; i < material->texture_count; i++) {
		tex_t tex = material->textures[i];
		if (tex == nullptr) continue;

		// Compare cached hash against current texture's hash
		if (material->texture_meta_hashes[i] != tex->meta_hash) {
			material->texture_meta_hashes[i] = tex->meta_hash;

			tex_t physical_tex = tex->fallback != nullptr
				? tex->fallback
				: tex;

			// Update the _i info param with texture dimensions
			id_hash_t tex_info_hash = hash_string_with("_i", meta->resources[i].name_hash);
			vec4      info          = { (float)physical_tex->width, (float)physical_tex->height, (float)(uint32_t)log2(physical_tex->width), 0 };
			material_set_param_id(material, tex_info_hash, material_param_vector4, &info);
		}
	}
}

} // namespace sk