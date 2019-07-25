#include "material.h"

material_t material_create(const char *name, shader_t shader, tex2d_t texture) {
	material_t result = (material_t)assets_find(name);
	if (result != nullptr) {
		assets_addref(result->header);
		return result;
	}

	result = (material_t)assets_allocate(asset_type_material, name);
	assets_addref(shader->header);
	assets_addref(texture->header);
	result->shader  = shader;
	result->texture = texture;
	return result;
}
void material_release(material_t material) {
	assets_releaseref(material->header);
}
void material_destroy(material_t material) {
	shader_release(material->shader);
	tex2d_release(material->texture);
	*material = {};
}