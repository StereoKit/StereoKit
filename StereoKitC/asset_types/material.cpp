#include "material.h"
#include "texture.h"
#include "../systems/d3d.h"
#include "../stref.h"

#include <stdio.h>

material_t material_find(const char *id) {
	material_t result = (material_t)assets_find(id);
	if (result != nullptr) {
		assets_addref(result->header);
		return result;
	}
	return result;
}

///////////////////////////////////////////

material_t material_create(const char *id, shader_t shader) {
	material_t result = material_find(id);
	if (result != nullptr)
		return result;

	result = (material_t)assets_allocate(asset_type_material, id);
	assets_addref(shader->header);
	result->cull          = material_cull_ccw;
	result->mode          = material_alpha_none;
	result->shader        = shader;
	result->args.buffer   = malloc(shader->args.buffer_size);
	result->args.textures = (tex2d_t*)malloc(sizeof(tex2d_t)*shader->tex_slots.tex_count);
	memset(result->args.buffer,   0, shader->args.buffer_size);
	memset(result->args.textures, 0, sizeof(tex2d_t) * shader->tex_slots.tex_count);

	for (size_t i = 0; i < shader->args_desc.item_count; i++) {
		shaderargs_desc_item_t &item = shader->args_desc.item[i];
		if (item.default_value != nullptr)
			memcpy((uint8_t*)result->args.buffer + item.offset, item.default_value, item.size);
	}
	return result;
}

///////////////////////////////////////////

material_t material_copy(const char *id, material_t material) {
	// Make a new empty material
	material_t result = material_create(id, material->shader);
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

inline shaderargs_desc_item_t *find_desc(material_t material, const char *name) {
	uint64_t id = string_hash(name);
	for (size_t i = 0; i < material->shader->args_desc.item_count; i++) {
		if (material->shader->args_desc.item[i].id == id) {
			return &material->shader->args_desc.item[i];
		}
	}
	log_writef(log_warning, "Can't find material parameter %s!", name);
	return nullptr;
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
	shaderargs_desc_item_t *desc = find_desc(material, name);
	if (desc != nullptr)
		*(float *)((uint8_t*)material->args.buffer + desc->offset) = value;
}

///////////////////////////////////////////

void material_set_color32(material_t material, const char *name, color32 value) {
	shaderargs_desc_item_t *desc = find_desc(material, name);
	if (desc != nullptr)
		*(color128 *)((uint8_t *)material->args.buffer + desc->offset) = { value.r / 255.f, value.g / 255.f, value.b / 255.f, value.a / 255.f };
}

///////////////////////////////////////////

void material_set_color(material_t material, const char *name, color128 value) {
	shaderargs_desc_item_t *desc = find_desc(material, name);
	if (desc != nullptr)
		*(color128 *)((uint8_t*)material->args.buffer + desc->offset) = value;
}

///////////////////////////////////////////

void material_set_vector(material_t material, const char *name, vec4 value) {
	shaderargs_desc_item_t *desc = find_desc(material, name);
	if (desc != nullptr)
		*(vec4 *)((uint8_t*)material->args.buffer + desc->offset) = value;
}

///////////////////////////////////////////

void material_set_matrix(material_t material, const char *name, matrix value) {
	shaderargs_desc_item_t *desc = find_desc(material, name);
	if (desc != nullptr)
		*(matrix *)((uint8_t*)material->args.buffer + desc->offset) = value;
}

///////////////////////////////////////////

void material_set_texture(material_t material, const char *name, tex2d_t value) {
	uint64_t id = string_hash(name);
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
			return;
		}
	}
	log_writef(log_warning, "Can't find material texture %s!", name);
}