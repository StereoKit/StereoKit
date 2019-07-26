#include "material.h"
#include "stref.h"

#include <stdio.h>

material_t material_create(const char *name, shader_t shader) {
	material_t result = (material_t)assets_find(name);
	if (result != nullptr) {
		assets_addref(result->header);
		return result;
	}

	result = (material_t)assets_allocate(asset_type_material, name);
	assets_addref(shader->header);
	result->shader  = shader;
	result->args.buffer   = malloc(shader->args.buffer_size);
	result->args.textures = (tex2d_t*)malloc(sizeof(tex2d_t)*shader->tex_slots.tex_count);
	memset(result->args.buffer,   0, shader->args.buffer_size);
	memset(result->args.textures, 0, sizeof(tex2d_t) * shader->tex_slots.tex_count); // maybe default texture?
	return result;
}
void material_release(material_t material) {
	assets_releaseref(material->header);
}
void material_destroy(material_t material) {
	for (size_t i = 0; i < material->shader->tex_slots.tex_count; i++) {
		if (material->args.textures[i] != nullptr)
			tex2d_release(material->args.textures[i]);
	}
	shader_release(material->shader);
	if (material->args.buffer   != nullptr) free(material->args.buffer);
	if (material->args.textures != nullptr) free(material->args.textures);
	*material = {};
}

inline shaderargs_desc_item_t *find_desc(material_t material, const char *name) {
	uint64_t id = string_hash(name);
	for (size_t i = 0; i < material->shader->args_desc.item_count; i++) {
		if (material->shader->args_desc.item[i].id == id) {
			return &material->shader->args_desc.item[i];
		}
	}
	printf("Can't find material parameter %s!\n", name);
	return nullptr;
}

void material_set_float(material_t material, const char *name, float value) {
	shaderargs_desc_item_t *desc = find_desc(material, name);
	if (desc != nullptr)
		*(float *)((uint8_t*)material->args.buffer + desc->offset) = value;
}
void material_set_color32(material_t material, const char *name, color32 value) {
	shaderargs_desc_item_t *desc = find_desc(material, name);
	if (desc != nullptr)
		*(color128 *)((uint8_t *)material->args.buffer + desc->offset) = { value.r / 255.f, value.g / 255.f, value.b / 255.f, value.a / 255.f };
}
void material_set_color(material_t material, const char *name, color128 value) {
	shaderargs_desc_item_t *desc = find_desc(material, name);
	if (desc != nullptr)
		*(color128 *)((uint8_t*)material->args.buffer + desc->offset) = value;
}
void material_set_vector(material_t material, const char *name, vec4 value) {
	shaderargs_desc_item_t *desc = find_desc(material, name);
	if (desc != nullptr)
		*(vec4 *)((uint8_t*)material->args.buffer + desc->offset) = value;
}
void material_set_matrix(material_t material, const char *name, matrix value) {
	shaderargs_desc_item_t *desc = find_desc(material, name);
	if (desc != nullptr)
		*(matrix *)((uint8_t*)material->args.buffer + desc->offset) = value;
}
void material_set_texture(material_t material, const char *name, tex2d_t value) {
	uint64_t id = string_hash(name);
	for (size_t i = 0; i < material->shader->tex_slots.tex_count; i++) {
		if (material->shader->tex_slots.tex[i].id == id) {
			int slot = material->shader->tex_slots.tex[i].slot;
			if (material->args.textures[slot] != value) {
				if (material->args.textures[slot] != nullptr)
					tex2d_release(material->args.textures[slot]);
				material->args.textures[slot] = value;
				assets_addref(value->header);
			}
			return;
		}
	}
	printf("Can't find material texture %s!\n", name);
}

void material_set_active(material_t material) {
	shader_set_active    (material->shader);
	shaderargs_set_data  (material->shader->args, material->args.buffer);
	shaderargs_set_active(material->shader->args);
	for (size_t i = 0; i < material->shader->tex_slots.tex_count; i++) {
		tex2d_set_active(material->args.textures[i], i);
	}
}