#include "../stereokit.h"
#include "../sk_memory.h"
#include "../libraries/stref.h"
#include "compute.h"
#include "compute_buffer.h"
#include "material.h"
#include "shader.h"
#include "texture.h"
#include "assets.h"
#include "../systems/render.h"

#include <string.h>

namespace sk {

///////////////////////////////////////////

static skr_compute_info_t compute_build_info(compute_t compute) {
	skr_compute_info_t info = {};
	info.spec_constants      = compute->spec_overrides;
	info.spec_constant_count = compute->spec_override_count;
	return info;
}

///////////////////////////////////////////

static void compute_recreate_gpu(compute_t compute) {
	skr_compute_set_pipeline(&compute->gpu_compute, compute_build_info(compute));
}

///////////////////////////////////////////
// Spec constant routing. A scalar setter whose name matches a shader's
// [[vk::constant_id]] constant persists a name-keyed override and rebuilds the
// pipeline, instead of writing a cbuffer uniform. Unlike materials, skr_compute_t
// exposes no resolved value array, so we resolve override-or-default locally.

// Convert a value to the constant's declared 32-bit bit pattern, mirroring
// sk_renderer's _skr_shader_resolve_spec_constants (bools resolve as int).
static uint32_t spec_constant_to_bits(const sksc_shader_spec_constant_t *sc, double value) {
	uint32_t bits = 0;
	switch (sc->type) {
	case sksc_shader_var_uint:  { uint32_t v = (uint32_t)value; memcpy(&bits, &v, sizeof(v)); } break;
	case sksc_shader_var_float: { float    v = (float   )value; memcpy(&bits, &v, sizeof(v)); } break;
	default:                    { int32_t  v = (int32_t )value; memcpy(&bits, &v, sizeof(v)); } break;
	}
	return bits;
}

// Reinterpret a resolved bit pattern as a double, per the constant's type.
static double spec_constant_from_bits(const sksc_shader_spec_constant_t *sc, uint32_t bits) {
	switch (sc->type) {
	case sksc_shader_var_uint:  { uint32_t v; memcpy(&v, &bits, sizeof(v)); return (double)v; }
	case sksc_shader_var_float: { float    v; memcpy(&v, &bits, sizeof(v)); return (double)v; }
	default:                    { int32_t  v; memcpy(&v, &bits, sizeof(v)); return (double)v; }
	}
}

// Meta index of a spec constant matching `name`, or -1. Capped to the
// resolvable range (SKR_MAX_SPEC_CONSTANTS).
static int32_t compute_find_spec_constant(compute_t compute, const char *name) {
	const sksc_shader_meta_t *meta = &compute->shader->gpu_shader.meta;
	if (meta->spec_constant_count == 0) return -1;
	uint32_t count = meta->spec_constant_count < SKR_MAX_SPEC_CONSTANTS ? meta->spec_constant_count : SKR_MAX_SPEC_CONSTANTS;
	uint64_t hash  = skr_hash(name);
	for (uint32_t i = 0; i < count; i++)
		if (meta->spec_constants[i].name_hash == hash) return (int32_t)i;
	return -1;
}

// Effective (override or default) bit pattern of a spec constant.
static uint32_t compute_resolved_spec_bits(compute_t compute, int32_t meta_idx) {
	const sksc_shader_spec_constant_t *sc = &compute->shader->gpu_shader.meta.spec_constants[meta_idx];
	for (int32_t i = 0; i < compute->spec_override_count; i++)
		if (compute->spec_overrides[i].name == sc->name)
			return spec_constant_to_bits(sc, compute->spec_overrides[i].value);
	return sc->default_value;
}

// Persist a name-keyed override (append or update). The stored name points into
// the shader meta, which outlives the override store.
static void compute_store_spec_override(compute_t compute, int32_t meta_idx, double value) {
	const sksc_shader_spec_constant_t *sc = &compute->shader->gpu_shader.meta.spec_constants[meta_idx];
	for (int32_t i = 0; i < compute->spec_override_count; i++) {
		if (compute->spec_overrides[i].name == sc->name) {
			compute->spec_overrides[i].value = value;
			return;
		}
	}
	compute->spec_overrides[compute->spec_override_count].name  = sc->name;
	compute->spec_overrides[compute->spec_override_count].value = value;
	compute->spec_override_count++;
}

// Rebuild the pipeline only when the resolved value actually changes.
static void compute_set_spec_constant(compute_t compute, int32_t meta_idx, double value) {
	const sksc_shader_spec_constant_t *sc = &compute->shader->gpu_shader.meta.spec_constants[meta_idx];
	if (compute_resolved_spec_bits(compute, meta_idx) == spec_constant_to_bits(sc, value))
		return;
	compute_store_spec_override(compute, meta_idx, value);
	compute_recreate_gpu       (compute);
}

// Effective (override or default) value of a spec constant, as a double.
static double compute_get_spec_constant(compute_t compute, int32_t meta_idx) {
	const sksc_shader_spec_constant_t *sc = &compute->shader->gpu_shader.meta.spec_constants[meta_idx];
	return spec_constant_from_bits(sc, compute_resolved_spec_bits(compute, meta_idx));
}

///////////////////////////////////////////

compute_t compute_create(shader_t shader) {
	if (shader == nullptr) {
		log_err("compute_create: shader is null");
		return nullptr;
	}

	compute_t result = (compute_t)assets_allocate(asset_type_compute);
	shader_addref(shader);
	result->shader = shader;

	if (skr_compute_create(&shader->gpu_shader, compute_build_info(result), &result->gpu_compute) != skr_err_success) {
		log_err("compute_create: failed to create GPU compute object");
		shader_release(shader);
		result->shader = nullptr;
		assets_releaseref(&result->header);
		return nullptr;
	}

	// Allocate tracking arrays for bound resources
	const sksc_shader_meta_t *meta = &shader->gpu_shader.meta;

	result->texture_count = 0;
	result->buffer_count  = 0;
	for (uint32_t i = 0; i < meta->resource_count; i++) {
		skr_register_ reg = (skr_register_)meta->resources[i].bind.register_type;
		if      (reg == skr_register_texture   || reg == skr_register_readwrite_tex) result->texture_count++;
		else if (reg == skr_register_readwrite || reg == skr_register_read_buffer)   result->buffer_count++;
	}

	if (result->texture_count > 0) result->textures = sk_malloc_zero_t(tex_t, result->texture_count);
	if (result->buffer_count  > 0) result->buffers  = sk_malloc_zero_t(compute_buffer_t, result->buffer_count);

	return result;
}

///////////////////////////////////////////

compute_t compute_find(const char *id) {
	compute_t result = (compute_t)assets_find(id, asset_type_compute);
	if (result != nullptr) {
		compute_addref(result);
		return result;
	}
	return nullptr;
}

///////////////////////////////////////////

void compute_set_id(compute_t compute, const char *id) {
	assets_set_id(&compute->header, id);
}

///////////////////////////////////////////

const char* compute_get_id(const compute_t compute) {
	return compute->header.id_text;
}

///////////////////////////////////////////

shader_t compute_get_shader(const compute_t compute) {
	shader_addref(compute->shader);
	return compute->shader;
}

///////////////////////////////////////////

void compute_set_float(compute_t compute, const char *name, float value) {
	int32_t spec_idx = compute_find_spec_constant(compute, name);
	if (spec_idx >= 0) { compute_set_spec_constant(compute, spec_idx, (double)value); return; }
	skr_compute_set_param(&compute->gpu_compute, name, sksc_shader_var_float, 1, &value);
}

///////////////////////////////////////////

void compute_set_int(compute_t compute, const char *name, int32_t value) {
	int32_t spec_idx = compute_find_spec_constant(compute, name);
	if (spec_idx >= 0) { compute_set_spec_constant(compute, spec_idx, (double)value); return; }
	skr_compute_set_param(&compute->gpu_compute, name, sksc_shader_var_int, 1, &value);
}

///////////////////////////////////////////

void compute_set_uint(compute_t compute, const char *name, uint32_t value) {
	int32_t spec_idx = compute_find_spec_constant(compute, name);
	if (spec_idx >= 0) { compute_set_spec_constant(compute, spec_idx, (double)value); return; }
	skr_compute_set_param(&compute->gpu_compute, name, sksc_shader_var_uint, 1, &value);
}

///////////////////////////////////////////

void compute_set_vector2(compute_t compute, const char *name, vec2 value) {
	skr_compute_set_param(&compute->gpu_compute, name, sksc_shader_var_float, 2, &value);
}

///////////////////////////////////////////

void compute_set_vector3(compute_t compute, const char *name, vec3 value) {
	skr_compute_set_param(&compute->gpu_compute, name, sksc_shader_var_float, 3, &value);
}

///////////////////////////////////////////

void compute_set_vector4(compute_t compute, const char *name, vec4 value) {
	skr_compute_set_param(&compute->gpu_compute, name, sksc_shader_var_float, 4, &value);
}

///////////////////////////////////////////

void compute_set_color(compute_t compute, const char *name, color128 color_gamma) {
	color128 linear = color_to_linear(color_gamma);
	skr_compute_set_param(&compute->gpu_compute, name, sksc_shader_var_float, 4, &linear);
}

///////////////////////////////////////////

void compute_set_bool(compute_t compute, const char *name, bool32_t value) {
	int32_t spec_idx = compute_find_spec_constant(compute, name);
	if (spec_idx >= 0) { compute_set_spec_constant(compute, spec_idx, value ? 1.0 : 0.0); return; }
	uint32_t val = value ? 1 : 0;
	skr_compute_set_param(&compute->gpu_compute, name, sksc_shader_var_uint, 1, &val);
}

///////////////////////////////////////////

void compute_set_matrix(compute_t compute, const char *name, matrix value) {
	skr_compute_set_param(&compute->gpu_compute, name, sksc_shader_var_float, 16, &value);
}

///////////////////////////////////////////

float compute_get_float(compute_t compute, const char *name) {
	int32_t spec_idx = compute_find_spec_constant(compute, name);
	if (spec_idx >= 0) return (float)compute_get_spec_constant(compute, spec_idx);
	float result = 0.0f;
	skr_compute_get_param(&compute->gpu_compute, name, sksc_shader_var_float, 1, &result);
	return result;
}

///////////////////////////////////////////

int32_t compute_get_int(compute_t compute, const char *name) {
	int32_t spec_idx = compute_find_spec_constant(compute, name);
	if (spec_idx >= 0) return (int32_t)compute_get_spec_constant(compute, spec_idx);
	int32_t result = 0;
	skr_compute_get_param(&compute->gpu_compute, name, sksc_shader_var_int, 1, &result);
	return result;
}

///////////////////////////////////////////

uint32_t compute_get_uint(compute_t compute, const char *name) {
	int32_t spec_idx = compute_find_spec_constant(compute, name);
	if (spec_idx >= 0) return (uint32_t)compute_get_spec_constant(compute, spec_idx);
	uint32_t result = 0;
	skr_compute_get_param(&compute->gpu_compute, name, sksc_shader_var_uint, 1, &result);
	return result;
}

///////////////////////////////////////////

vec2 compute_get_vector2(compute_t compute, const char *name) {
	vec2 result = {};
	skr_compute_get_param(&compute->gpu_compute, name, sksc_shader_var_float, 2, &result);
	return result;
}

///////////////////////////////////////////

vec3 compute_get_vector3(compute_t compute, const char *name) {
	vec3 result = {};
	skr_compute_get_param(&compute->gpu_compute, name, sksc_shader_var_float, 3, &result);
	return result;
}

///////////////////////////////////////////

vec4 compute_get_vector4(compute_t compute, const char *name) {
	vec4 result = {};
	skr_compute_get_param(&compute->gpu_compute, name, sksc_shader_var_float, 4, &result);
	return result;
}

///////////////////////////////////////////

bool32_t compute_get_bool(compute_t compute, const char *name) {
	int32_t spec_idx = compute_find_spec_constant(compute, name);
	if (spec_idx >= 0) return compute_get_spec_constant(compute, spec_idx) != 0;
	uint32_t result = 0;
	skr_compute_get_param(&compute->gpu_compute, name, sksc_shader_var_uint, 1, &result);
	return result != 0;
}

///////////////////////////////////////////

color128 compute_get_color(compute_t compute, const char *name) {
	color128 result = {1,1,1,1};
	skr_compute_get_param(&compute->gpu_compute, name, sksc_shader_var_float, 4, &result);
	return result;
}

///////////////////////////////////////////

matrix compute_get_matrix(compute_t compute, const char *name) {
	matrix result = matrix_identity;
	skr_compute_get_param(&compute->gpu_compute, name, sksc_shader_var_float, 16, &result);
	return result;
}

///////////////////////////////////////////

bool32_t compute_set_texture(compute_t compute, const char *name, tex_t texture) {
	const sksc_shader_meta_t *meta = &compute->shader->gpu_shader.meta;

	// Find which texture tracking slot this resource maps to
	int32_t  tex_idx = 0;
	int32_t  found   = -1;
	uint64_t hash    = hash_string(name);
	for (uint32_t i = 0; i < meta->resource_count; i++) {
		skr_register_ reg = (skr_register_)meta->resources[i].bind.register_type;
		if (reg == skr_register_texture || reg == skr_register_readwrite_tex) {
			if (meta->resources[i].name_hash == hash) {
				found = tex_idx;
				break;
			}
			tex_idx++;
		}
	}

	if (found < 0 || found >= compute->texture_count)
		return false;

	if (texture) tex_addref(texture);
	tex_release(compute->textures[found]);
	compute->textures[found] = texture;
		
	return true;
}

///////////////////////////////////////////

bool32_t compute_set_storage(compute_t compute, const char *name, compute_buffer_t buffer) {
	const sksc_shader_meta_t *meta = &compute->shader->gpu_shader.meta;

	// Find which buffer tracking slot this resource maps to
	int32_t  buf_idx = 0;
	int32_t  found   = -1;
	uint64_t hash    = hash_string(name);
	for (uint32_t i = 0; i < meta->resource_count; i++) {
		skr_register_ reg = (skr_register_)meta->resources[i].bind.register_type;
		if (reg == skr_register_readwrite || reg == skr_register_read_buffer) {
			if (meta->resources[i].name_hash == hash) {
				found = buf_idx;
				break;
			}
			buf_idx++;
		}
	}

	if (found < 0 || found >= compute->buffer_count)
		return false;

	if (buffer) compute_buffer_addref(buffer);
	compute_buffer_release(compute->buffers[found]);
	compute->buffers[found] = buffer;

	// Always forward to sk_renderer
	skr_compute_set_buffer(&compute->gpu_compute, name, buffer ? &buffer->gpu_buffer : nullptr);
	return true;
}

///////////////////////////////////////////

bool32_t compute_set_constant(compute_t compute, const char *name, material_buffer_t buffer) {
	const sksc_shader_meta_t *meta = &compute->shader->gpu_shader.meta;
	id_hash_t hash = hash_string(name);

	for (uint32_t i = 0; i < meta->buffer_count; i++) {
		if (meta->buffers[i].name_hash != hash) continue;

		skr_register_ reg = (skr_register_)meta->buffers[i].bind.register_type;
		if (reg != skr_register_constant) {
			log_warnf("'%s' is not a constant buffer.", name);
			return false;
		}

		skr_compute_set_buffer(&compute->gpu_compute, name, buffer ? &buffer->buffer : nullptr);
		return true;
	}
	return false;
}

///////////////////////////////////////////

void compute_dispatch(compute_t compute, uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z) {
	render_queue_compute(compute, group_count_x, group_count_y, group_count_z);
}

///////////////////////////////////////////

void compute_dispatch_now(compute_t compute, uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z) {
	// Resolve texture fallbacks at dispatch time so that textures
	// that finish loading between set and dispatch use the correct
	// data instead of a stale fallback.
	const sksc_shader_meta_t *meta = &compute->shader->gpu_shader.meta;
	int32_t tex_idx = 0;
	for (uint32_t i = 0; i < meta->resource_count; i++) {
		skr_register_ reg = (skr_register_)meta->resources[i].bind.register_type;
		if (reg == skr_register_texture || reg == skr_register_readwrite_tex) {
			if (tex_idx < compute->texture_count) {
				tex_t tex = compute->textures[tex_idx];
				skr_compute_set_tex(&compute->gpu_compute, meta->resources[i].name, tex != nullptr
					? tex->fallback != nullptr
						? &tex->fallback->gpu_tex
						: &tex->gpu_tex
					: nullptr);
			}
			tex_idx++;
		}
	}

	skr_compute_execute(&compute->gpu_compute, group_count_x, group_count_y, group_count_z);
}

///////////////////////////////////////////

int32_t compute_get_param_count(compute_t compute) {
	int32_t buffer_id = compute->shader->gpu_shader.meta.global_buffer_id;
	if (buffer_id == -1)
		return compute->shader->gpu_shader.meta.resource_count;

	return
		compute->shader->gpu_shader.meta.buffers[buffer_id].var_count +
		compute->shader->gpu_shader.meta.resource_count;
}

///////////////////////////////////////////

void compute_get_param_info(compute_t compute, int32_t index, char **out_name, material_param_ *out_type) {
	const sksc_shader_meta_t *meta = &compute->shader->gpu_shader.meta;

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
				else if (info->type_count == 4 ) *out_type = string_eq_nocase(info->extra, "color") ? material_param_color128 : material_param_vector4;
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
		int32_t res_idx = index - buffer_ct;
		if (out_type != nullptr) {
			skr_register_ reg = (skr_register_)meta->resources[res_idx].bind.register_type;
			*out_type = (reg == skr_register_readwrite || reg == skr_register_read_buffer)
				? material_param_buffer
				: material_param_texture;
		}
		if (out_name != nullptr) *out_name = meta->resources[res_idx].name;
	}
}

///////////////////////////////////////////

void compute_addref(compute_t compute) {
	assets_addref(&compute->header);
}

///////////////////////////////////////////

void compute_release(compute_t compute) {
	if (compute == nullptr)
		return;
	assets_releaseref(&compute->header);
}

///////////////////////////////////////////

void compute_destroy(compute_t compute) {
	// Release tracked data
	for (int32_t i = 0; i < compute->texture_count; i++) tex_release(compute->textures[i]);
	for (int32_t i = 0; i < compute->buffer_count; i++) compute_buffer_release(compute->buffers[i]);
	sk_free(compute->textures);
	sk_free(compute->buffers);

	shader_release(compute->shader);
	skr_compute_destroy(&compute->gpu_compute);

	*compute = {};
}

} // namespace sk
