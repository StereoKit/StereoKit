#include "material.h"
#include "shader.h"
#include "../libraries/stref.h"
#include "../libraries/ferr_hash.h"
#include "../libraries/array.h"
#include "../sk_memory.h"

#include <stdio.h>
#include <string.h>

namespace sk {

_material_buffer_t material_buffers[14] = {};

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
	assets_set_id(material->header, id);
}

///////////////////////////////////////////

inline size_t material_param_size(material_param_ type) {
	switch (type) {
	case material_param_float:    return sizeof(float);
	case material_param_color128: return sizeof(color128);
	case material_param_vector:   return sizeof(vec4);
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

	if (buff_size != 0) {
		material->args.buffer       = sk_malloc(buff_size);
		material->args.buffer_size  = buff_size;
		material->args.buffer_bind  = buff_info->bind;
		material->args.buffer_dirty = true;
		material->args.buffer_gpu   = skg_buffer_create(nullptr, 1, buff_size, skg_buffer_type_constant, skg_use_dynamic);
		if (buff_info->defaults != nullptr)
			memcpy(material->args.buffer, buff_info->defaults, buff_size);
		else
			memset(material->args.buffer, 0, buff_size);
	}
	if (meta->resource_count > 0) {
		material->args.texture_count = meta->resource_count;
		material->args.textures      = sk_malloc_t(tex_t     , meta->resource_count);
		material->args.texture_binds = sk_malloc_t(skg_bind_t, meta->resource_count);
		memset(material->args.textures, 0, sizeof(tex_t) * meta->resource_count);
		for (size_t i = 0; i < meta->resource_count; i++) {
			material->args.texture_binds[i] = meta->resources[i].bind;

			if      (string_eq(meta->resources[i].extra, "white")) material->args.textures[i] = tex_find(default_id_tex);
			else if (string_eq(meta->resources[i].extra, "black")) material->args.textures[i] = tex_find(default_id_tex_black);
			else if (string_eq(meta->resources[i].extra, "gray" )) material->args.textures[i] = tex_find(default_id_tex_gray);
			else if (string_eq(meta->resources[i].extra, "flat" )) material->args.textures[i] = tex_find(default_id_tex_flat);
			else if (string_eq(meta->resources[i].extra, "rough")) material->args.textures[i] = tex_find(default_id_tex_rough);
			else                                                   material->args.textures[i] = tex_find(default_id_tex);
		}
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
	// Make a new empty material
	material_t result = material_create(material->shader);
	// release any of the default textures for the material.
	for (int32_t i = 0; i < result->args.texture_count; i++) {
		if (result->args.textures[i] != nullptr)
			tex_release(result->args.textures[i]);
	}
	// Store allocated memory temporarily
	void          *tmp_buffer        = result->args.buffer;
	skg_buffer_t   tmp_buffer_gpu    = result->args.buffer_gpu;
	tex_t         *tmp_textures      = result->args.textures;
	skg_bind_t    *tmp_texture_binds = result->args.texture_binds;
	asset_header_t tmp_header        = result->header;

	// Copy everything over from the old one, and then re-write with our own custom memory. Then copy that over too!
	memcpy(result, material, sizeof(_material_t));
	result->header             = tmp_header;
	result->args.buffer        = tmp_buffer;
	result->args.buffer_gpu    = tmp_buffer_gpu;
	result->args.textures      = tmp_textures;
	result->args.texture_binds = tmp_texture_binds;
	result->args.buffer_dirty  = true;
	memcpy(result->args.buffer,        material->args.buffer,        material->args.buffer_size);
	memcpy(result->args.textures,      material->args.textures,      sizeof(tex_t)      * material->args.texture_count);
	memcpy(result->args.texture_binds, material->args.texture_binds, sizeof(skg_bind_t) * material->args.texture_count);

	// Add references to all the other material's assets
	for (int32_t i = 0; i < result->args.texture_count; i++) {
		if (result->args.textures[i] != nullptr)
			tex_addref(result->args.textures[i]);
	}

	// Copy over the material's pipeline
	result->pipeline = skg_pipeline_create(&material->shader->shader);
	skg_pipeline_set_cull        (&result->pipeline, skg_pipeline_get_cull        (&material->pipeline));
	skg_pipeline_set_transparency(&result->pipeline, skg_pipeline_get_transparency(&material->pipeline));
	skg_pipeline_set_wireframe   (&result->pipeline, skg_pipeline_get_wireframe   (&material->pipeline));
	skg_pipeline_set_depth_test  (&result->pipeline, skg_pipeline_get_depth_test  (&material->pipeline));
	skg_pipeline_set_depth_write (&result->pipeline, skg_pipeline_get_depth_write (&material->pipeline));

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
	assets_addref(material->header);
}

///////////////////////////////////////////

void material_release(material_t material) {
	if (material == nullptr)
		return;
	assets_releaseref(material->header);
}

///////////////////////////////////////////

void material_destroy(material_t material) {
	for (int32_t i = 0; i < material->args.texture_count; i++) {
		if (material->args.textures[i] != nullptr)
			tex_release(material->args.textures[i]);
	}
	shader_release(material->shader);
	skg_pipeline_destroy(&material->pipeline);
	free(material->args.buffer);
	free(material->args.textures);
	free(material->args.texture_binds);
	*material = {};
}

///////////////////////////////////////////

void material_set_shader(material_t material, shader_t shader) {
	if (shader == material->shader)
		return;

	// Update references
	if (shader != nullptr)
		shader_addref(shader);

	// Copy over any relevant values that are attached to the old shader
	if (material->shader != nullptr && shader != nullptr) {
		shader_t old_shader   = material->shader;
		void    *old_buffer   = material->args.buffer;
		tex_t   *old_textures = material->args.textures;
		skg_bind_t *old_binds = material->args.texture_binds;
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
			material_set_texture(material, old_shader->shader.meta->resources[i].name, old_textures[i]);
			tex_release(old_textures[i]);
		}

		// And release the old shader content
		if (old_shader != nullptr)
			shader_release(old_shader);
		skg_pipeline_destroy(&material->pipeline);
		free(old_buffer);
		free(old_textures);
		free(old_binds);
	}

	material->shader   = shader;
	material->pipeline = skg_pipeline_create(&material->shader->shader);
	material_set_cull        (material, material->cull);
	material_set_depth_test  (material, material->depth_test);
	material_set_depth_write (material, material->depth_write);
	material_set_transparency(material, material->alpha_mode);
	material_set_wireframe   (material, material->wireframe);
}

///////////////////////////////////////////

shader_t material_get_shader(material_t material) {
	return material->shader;
}

///////////////////////////////////////////

void material_set_transparency(material_t material, transparency_ mode) {
	material->alpha_mode = mode;
	skg_pipeline_set_transparency(&material->pipeline, (skg_transparency_)mode);
}

///////////////////////////////////////////

void material_set_cull(material_t material, cull_ mode) {
	material->cull = mode;
	skg_pipeline_set_cull(&material->pipeline, (skg_cull_)mode);
}

///////////////////////////////////////////

void material_set_wireframe(material_t material, bool32_t wireframe) {
	material->wireframe = wireframe;
	skg_pipeline_set_wireframe(&material->pipeline, wireframe);
}

///////////////////////////////////////////

void material_set_depth_test(material_t material, depth_test_ depth_test_mode) {
	material->depth_test = depth_test_mode;
	skg_pipeline_set_depth_test(&material->pipeline, (skg_depth_test_)depth_test_mode);
}

///////////////////////////////////////////

void material_set_depth_write(material_t material, bool32_t write_enabled) {
	material->depth_write = write_enabled;
	skg_pipeline_set_depth_write(&material->pipeline, write_enabled);
}

///////////////////////////////////////////

void material_set_queue_offset(material_t material, int32_t offset) {
	material->queue_offset = offset;
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

void material_set_float(material_t material, const char *name, float value) {
	int32_t i = skg_shader_get_var_index(&material->shader->shader, name);
	if (i == -1) return;
	
	const skg_shader_var_t *info = skg_shader_get_var_info(&material->shader->shader, i);
	if (info->size != sizeof(float)) {
		log_errf("material_set_float: '%s' is not a float (in shader %s)",  info->name, material->shader->shader.meta->name);
		return;
	}
	*(float *)((uint8_t*)material->args.buffer + info->offset) = value;
	material->args.buffer_dirty = true;
}

///////////////////////////////////////////

void material_set_color32(material_t material, const char *name, color32 value) {
	int32_t i = skg_shader_get_var_index(&material->shader->shader, name);
	if (i == -1) return;

	const skg_shader_var_t *info = skg_shader_get_var_info(&material->shader->shader, i);
	if (info->size != sizeof(color128)) {
		log_errf("material_set_color32: '%s' is not a float4 (in shader %s)",  info->name, material->shader->shader.meta->name);
		return;
	}
	*(color128 *)((uint8_t *)material->args.buffer + info->offset) = color_to_linear( { value.r / 255.f, value.g / 255.f, value.b / 255.f, value.a / 255.f } );
	material->args.buffer_dirty = true;
}

///////////////////////////////////////////

void material_set_color(material_t material, const char *name, color128 value) {
	int32_t i = skg_shader_get_var_index(&material->shader->shader, name);
	if (i == -1) return;

	const skg_shader_var_t *info = skg_shader_get_var_info(&material->shader->shader, i);
	if (info->size != sizeof(color128)) {
		log_errf("material_set_color: '%s' is not a float4 (in shader %s)",  info->name, material->shader->shader.meta->name);
		return;
	}
	*(color128 *)((uint8_t*)material->args.buffer + info->offset) = color_to_linear( value );
	material->args.buffer_dirty = true;
}

///////////////////////////////////////////

void material_set_vector(material_t material, const char *name, vec4 value) {
	material_set_vector4(material, name, value);
}

///////////////////////////////////////////

void material_set_vector4(material_t material, const char *name, vec4 value) {
	int32_t i = skg_shader_get_var_index(&material->shader->shader, name);
	if (i == -1) return;

	const skg_shader_var_t *info = skg_shader_get_var_info(&material->shader->shader, i);
	if (info->size != sizeof(vec4)) {
		log_errf("material_set_vector4: '%s' is not a float4 (in shader %s)",  info->name, material->shader->shader.meta->name);
		return;
	}
	*(vec4 *)((uint8_t*)material->args.buffer + info->offset) = value;
	material->args.buffer_dirty = true;
}

///////////////////////////////////////////

void material_set_vector3(material_t material, const char *name, vec3 value) {
	int32_t i = skg_shader_get_var_index(&material->shader->shader, name);
	if (i == -1) return;

	const skg_shader_var_t *info = skg_shader_get_var_info(&material->shader->shader, i);
	if (info->size != sizeof(vec3)) {
		log_errf("material_set_vector3: '%s' is not a float3 (in shader %s)",  info->name, material->shader->shader.meta->name);
		return;
	}
	*(vec3 *)((uint8_t*)material->args.buffer + info->offset) = value;
	material->args.buffer_dirty = true;
}

///////////////////////////////////////////

void material_set_vector2(material_t material, const char *name, vec2 value) {
	int32_t i = skg_shader_get_var_index(&material->shader->shader, name);
	if (i == -1) return;

	const skg_shader_var_t *info = skg_shader_get_var_info(&material->shader->shader, i);
	if (info->size != sizeof(vec2)) {
		log_errf("material_set_vector2: '%s' is not a float2 (in shader %s)",  info->name, material->shader->shader.meta->name);
		return;
	}
	*(vec2 *)((uint8_t*)material->args.buffer + info->offset) = value;
	material->args.buffer_dirty = true;
}

///////////////////////////////////////////

void material_set_matrix(material_t material, const char *name, matrix value) {
	int32_t i = skg_shader_get_var_index(&material->shader->shader, name);
	if (i == -1) return;

	const skg_shader_var_t *info = skg_shader_get_var_info(&material->shader->shader, i);
	if (info->size != sizeof(matrix)) {
		log_errf("material_set_matrix: '%s' is not a float4x4 (in shader %s)",  info->name, material->shader->shader.meta->name);
		return;
	}
	*(matrix *)((uint8_t*)material->args.buffer + info->offset) = value;
	material->args.buffer_dirty = true;
}

///////////////////////////////////////////

bool32_t material_set_texture_id(material_t material, uint64_t id, tex_t value) {
	for (uint32_t i = 0; i < material->shader->shader.meta->resource_count; i++) {
		if (material->shader->shader.meta->resources[i].name_hash == id) {
			if (material->args.textures[i] != value) {
				if (material->args.textures[i] != nullptr)
					tex_release(material->args.textures[i]);
				material->args.textures[i] = value;
				if (value != nullptr) {
					tex_addref(value);

					// Tell the shader about the texture dimensions, if it has
					// a parameter for it. Texture info will get put into any
					// float4 param with the name [texname]_i
					uint64_t tex_info_hash = hash_fnv64_string("_i", id);
					vec4     info = {(float)tex_get_width(value), (float)tex_get_height(value), (float)(uint32_t)log2(tex_get_width(value)), 0};
					material_set_param_id(material, tex_info_hash, material_param_vector, &info);
				}
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

bool32_t material_has_param(material_t material, const char *name, material_param_ type) {
	uint64_t id = hash_fnv64_string(name);

	if (type == material_param_texture) {
		for (size_t i = 0; i < material->shader->shader.meta->resource_count; i++) {
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
		for (size_t i = 0; i < material->shader->shader.meta->resource_count; i++) {
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
	skg_shader_var_t *info = &material->shader->shader.meta->buffers[material->shader->shader.meta->global_buffer_id].vars[index];
	if (out_type != nullptr) *out_type = (material_param_)0; //TODO: implement this // info->type;
	if (out_name != nullptr) *out_name = info->name;
	log_warn("material_get_param_info doesn't implement type yet.");
}

///////////////////////////////////////////

int material_get_param_count(material_t material) {
	if (material->shader->shader.meta->global_buffer_id == -1)
		return 0;

	return material->shader->shader.meta->buffers[material->shader->shader.meta->global_buffer_id].var_count;
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
	return &material_buffers[register_slot];
}

///////////////////////////////////////////

void material_buffer_set_data(material_buffer_t buffer, const void *data) {
	skg_buffer_set_contents(&buffer->buffer, data, buffer->size);
}

///////////////////////////////////////////

void material_buffer_release(material_buffer_t buffer) {
	skg_buffer_destroy(&buffer->buffer);
	*buffer = {};
}

} // namespace sk