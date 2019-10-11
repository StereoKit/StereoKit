#include "material.h"
#include "texture.h"
#include "../systems/d3d.h"
#include "../libraries/stref.h"

#include <stdio.h>

namespace sk {

///////////////////////////////////////////

material_t material_find(const char *id) {
	material_t result = (material_t)assets_find(id);
	if (result != nullptr) {
		assets_addref(result->header);
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
	case material_param_texture:  return sizeof(tex2d_t);
	default: log_err("Bad material param type"); return 0;
	}
}

///////////////////////////////////////////

void material_create_arg_defaults(material_t material, shader_t shader) {
	material->args.buffer   = malloc(shader->args.buffer_size);
	material->args.textures = (tex2d_t*)malloc(sizeof(tex2d_t)*shader->tex_slots.tex_count);
	memset(material->args.buffer,   0, shader->args.buffer_size);
	memset(material->args.textures, 0, sizeof(tex2d_t) * shader->tex_slots.tex_count);

	for (size_t i = 0; i < shader->args_desc.item_count; i++) {
		shaderargs_desc_item_t &item = shader->args_desc.item[i];
		if (item.default_value != nullptr)
			memcpy((uint8_t*)material->args.buffer + item.offset, item.default_value, item.size);
	}
}

///////////////////////////////////////////

material_t material_create(shader_t shader) {

	material_t result = (material_t)assets_allocate(asset_type_material);
	assets_addref(shader->header);
	result->cull          = material_cull_ccw;
	result->mode          = material_alpha_none;
	result->shader        = shader;
	
	material_create_arg_defaults(result, shader);

	return result;
}

///////////////////////////////////////////

material_t material_copy(material_t material) {
	// Make a new empty material
	material_t result = material_create(material->shader);
	// Store allocated memory temporarily
	void          *tmp_buffer   = result->args.buffer;
	tex2d_t       *tmp_textures = result->args.textures;
	asset_header_t tmp_header   = result->header;

	// Copy everything over from the old one, and then re-write with our own custom memory. Then copy that over too!
	memcpy(result, material, sizeof(_material_t));
	result->header        = tmp_header;
	result->args.buffer   = tmp_buffer;
	result->args.textures = tmp_textures;
	memcpy(result->args.buffer,   material->args.buffer,   material->shader->args.buffer_size);
	memcpy(result->args.textures, material->args.textures, sizeof(tex2d_t) * material->shader->tex_slots.tex_count);

	// Add references to all the other material's assets
	assets_addref(result->shader->header);
	for (size_t i = 0; i < result->shader->tex_slots.tex_count; i++) {
		if (result->args.textures[i] != nullptr)
			assets_addref(result->args.textures[i]->header);
	}
	if (result->rasterizer_state != nullptr) result->rasterizer_state->AddRef();
	if (result->blend_state      != nullptr) result->blend_state->AddRef();

	return result;
}

///////////////////////////////////////////

void material_release(material_t material) {
	if (material == nullptr)
		return;
	assets_releaseref(material->header);
}

///////////////////////////////////////////

void material_destroy(material_t material) {
	for (size_t i = 0; i < material->shader->tex_slots.tex_count; i++) {
		if (material->args.textures[i] != nullptr)
			tex2d_release(material->args.textures[i]);
	}
	shader_release(material->shader);
	if (material->blend_state != nullptr) material->blend_state->Release();
	free(material->args.buffer);
	free(material->args.textures);
	*material = {};
}

///////////////////////////////////////////

shaderargs_desc_item_t *find_desc(shader_t shader, uint64_t id) {
	for (size_t i = 0; i < shader->args_desc.item_count; i++) {
		if (shader->args_desc.item[i].id == id) {
			return &shader->args_desc.item[i];
		}
	}
	return nullptr;
}

///////////////////////////////////////////

shaderargs_desc_item_t *find_desc(shader_t shader, const char *name) {
	uint64_t                id     = string_hash(name);
	shaderargs_desc_item_t *result = find_desc(shader, id);
	if (result == nullptr)
		log_warnf("Can't find shader parameter %s!", name);
	return result;
}

///////////////////////////////////////////

void material_set_shader(material_t material, shader_t shader) {
	if (shader == material->shader)
		return;

	// Copy over any relevant values that are attached to the old shader
	if (material->shader != nullptr && shader != nullptr) {
		shader_t old_shader   = material->shader;
		void    *old_buffer   = material->args.buffer;
		tex2d_t *old_textures = material->args.textures;
		material_create_arg_defaults(material, shader);

		// Copy old param values
		for (size_t i = 0; i < old_shader->args_desc.item_count; i++) {
			shaderargs_desc_item_t &item     = old_shader->args_desc.item[i];
			shaderargs_desc_item_t *new_slot = find_desc(shader, item.id);
			if (new_slot != nullptr)
				memcpy( (uint8_t *)material->args.buffer + new_slot->offset, 
						(uint8_t *)old_buffer            + item.offset, new_slot->size);
		}

		// Do the same for textures
		for (size_t i = 0; i < old_shader->tex_slots.tex_count; i++) {
			material_set_texture_id(material, old_shader->tex_slots.tex[i].id, old_textures[i]);
			tex2d_release(old_textures[i]);
		}

		free(old_buffer);
		free(old_textures);
	}

	// Update references
	if (shader != nullptr)
		assets_addref(shader->header);
	if (material->shader != nullptr)
		shader_release(material->shader);

	material->shader = shader;
}

///////////////////////////////////////////

shader_t material_get_shader(material_t material) {
	return material->shader;
}

///////////////////////////////////////////

void material_set_alpha_mode(material_t material, material_alpha_ mode) {
	if (material->blend_state != nullptr)
		material->blend_state->Release();
	D3D11_BLEND_DESC desc_blend = {};
	desc_blend.AlphaToCoverageEnable  = false;
	desc_blend.IndependentBlendEnable = false;
	desc_blend.RenderTarget[0].BlendEnable = mode == material_alpha_blend;
	desc_blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	desc_blend.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	desc_blend.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	desc_blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	desc_blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	desc_blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	desc_blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	d3d_device->CreateBlendState(&desc_blend, &material->blend_state);
	material->mode = mode;
}

///////////////////////////////////////////

void material_set_cull(material_t material, material_cull_ mode) {
	if (material->rasterizer_state != nullptr)
		material->rasterizer_state->Release();
	D3D11_RASTERIZER_DESC desc_rasterizer = {};
	desc_rasterizer.FillMode = D3D11_FILL_SOLID;
	desc_rasterizer.CullMode = mode == material_cull_none ? D3D11_CULL_NONE : D3D11_CULL_BACK;
	desc_rasterizer.FrontCounterClockwise = mode == material_cull_ccw;

	d3d_device->CreateRasterizerState(&desc_rasterizer, &material->rasterizer_state);
	material->cull = mode;
}

///////////////////////////////////////////

void material_set_queue_offset(material_t material, int32_t offset) {
	material->queue_offset = offset;
}

///////////////////////////////////////////

void material_set_float(material_t material, const char *name, float value) {
	shaderargs_desc_item_t *desc = find_desc(material->shader, name);
	if (desc != nullptr)
		*(float *)((uint8_t*)material->args.buffer + desc->offset) = value;
}

///////////////////////////////////////////

void material_set_color32(material_t material, const char *name, color32 value) {
	shaderargs_desc_item_t *desc = find_desc(material->shader, name);
	if (desc != nullptr)
		*(color128 *)((uint8_t *)material->args.buffer + desc->offset) = { value.r / 255.f, value.g / 255.f, value.b / 255.f, value.a / 255.f };
}

///////////////////////////////////////////

void material_set_color(material_t material, const char *name, color128 value) {
	shaderargs_desc_item_t *desc = find_desc(material->shader, name);
	if (desc != nullptr)
		*(color128 *)((uint8_t*)material->args.buffer + desc->offset) = value;
}

///////////////////////////////////////////

void material_set_vector(material_t material, const char *name, vec4 value) {
	shaderargs_desc_item_t *desc = find_desc(material->shader, name);
	if (desc != nullptr)
		*(vec4 *)((uint8_t*)material->args.buffer + desc->offset) = value;
}

///////////////////////////////////////////

void material_set_matrix(material_t material, const char *name, matrix value) {
	shaderargs_desc_item_t *desc = find_desc(material->shader, name);
	if (desc != nullptr)
		*(matrix *)((uint8_t*)material->args.buffer + desc->offset) = value;
}

///////////////////////////////////////////

bool32_t material_set_texture_id(material_t material, uint64_t id, tex2d_t value) {
	for (size_t i = 0; i < material->shader->tex_slots.tex_count; i++) {
		if (material->shader->tex_slots.tex[i].id == id) {
			int slot = material->shader->tex_slots.tex[i].slot;
			if (material->args.textures[slot] != value) {
				if (material->args.textures[slot] != nullptr)
					tex2d_release(material->args.textures[slot]);
				material->args.textures[slot] = value;
				if (value != nullptr)
					assets_addref(value->header);
			}
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////

bool32_t material_set_texture(material_t material, const char *name, tex2d_t value) {
	uint64_t id = string_hash(name);
	return material_set_texture_id(material, id, value);
}

///////////////////////////////////////////

void material_set_param(material_t material, const char *name, material_param_ type, const void *value) {
	material_set_param_id(material, string_hash(name), type, value);
}

///////////////////////////////////////////

void material_set_param_id(material_t material, uint64_t id, material_param_ type, const void *value) {
	if (type == material_param_texture) {
		material_set_texture_id(material, id, (tex2d_t)value);
	} else {
		shaderargs_desc_item_t *desc = find_desc(material->shader, id);
		if (desc != nullptr) {
			size_t size = material_param_size(type);
			memcpy(((uint8_t *)material->args.buffer + desc->offset), value, size);
		}
	}
}

///////////////////////////////////////////

bool32_t material_get_param(material_t material, const char *name, material_param_ type, void *out_value) {
	return material_get_param_id(material, string_hash(name), type, out_value);
}

///////////////////////////////////////////

bool32_t material_get_param_id(material_t material, uint64_t id, material_param_ type, void *out_value) {
	if (type == material_param_texture) {
		for (size_t i = 0; i < material->shader->tex_slots.tex_count; i++) {
			if (material->shader->tex_slots.tex[i].id == id) {
				memcpy(out_value, material->args.textures[material->shader->tex_slots.tex[i].slot], sizeof(tex2d_t));
				return true;
			}
		}
	} else {
		shaderargs_desc_item_t *desc = find_desc(material->shader, id);
		if (desc != nullptr) {
			size_t size = material_param_size(type);
			memcpy(out_value, ((uint8_t *)material->args.buffer + desc->offset), size);
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////

void material_get_param_info(material_t material, int index, char **out_name, material_param_ *out_type) {
	shaderargs_desc_item_t &item = material->shader->args_desc.item[index];
	if (out_type != nullptr) *out_type = item.type;
	if (out_name != nullptr) *out_name = item.name;
}

///////////////////////////////////////////

int material_get_param_count(material_t material) {
	return material->shader->args_desc.item_count;
}

} // namespace sk