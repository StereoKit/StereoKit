#include "material.h"

material_t material_create(shader_t shader, tex2d_t tex) {
	material_t result = (material_t)assets_allocate(asset_type_material);
	result->shader  = shader;
	result->texture = tex;
	return result;
}
void material_destroy(material_t material) {
	*material = {};
}