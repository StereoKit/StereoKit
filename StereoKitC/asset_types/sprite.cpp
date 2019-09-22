#include "sprite.h"
#include "assets.h"
#include "texture.h"
#include "../stref.h"
#include "../systems/sprite_drawer.h"

#include <vector>
using namespace std;

struct sprite_inst_t {
	vec2    uvs[2];
	color32 color;
	matrix  transform;
};

struct spritemap_t {
	tex2d_t texture;
};

int32_t sprite_index = 0;

///////////////////////////////////////////

sprite_t sprite_create(tex2d_t image, sprite_type_ type, const char *atlas_id) {
	assets_addref(image->header);
	sprite_t result = (_sprite_t*)assets_allocate(asset_type_sprite, string_hash("_sprite", image->header.id));

	result->texture = image;
	result->uvs[0] = vec2{ 0,0 };
	result->uvs[1] = vec2{ 1,1 };
	

	if (type == sprite_type_single) {
		char id[64];
		sprintf_s(id, 64, "render/sprite_mat_%d", sprite_index);
		result->material     = material_create(id, shader_find("default/shader_unlit"));
		material_set_texture   (result->material, "diffuse", image);
		material_set_alpha_mode(result->material, material_alpha_blend);
		material_set_cull      (result->material, material_cull_none);
		result->size         = 1;
		result->buffer_index = -1;
	}
	sprite_index += 1;
	return result;
}

///////////////////////////////////////////

sprite_t sprite_create_file(const char *filename, sprite_type_ type, const char *atlas_id) {
	tex2d_t  image  = tex2d_create_file(filename);
	sprite_t result = sprite_create(image, type, atlas_id);
	tex2d_release(image);
	return result;
}

///////////////////////////////////////////

void sprite_release(sprite_t sprite) {
	if (sprite == nullptr)
		return;
	assets_releaseref(sprite->header);
}

///////////////////////////////////////////

void sprite_destroy(sprite_t sprite) {
	tex2d_release   (sprite->texture);
	material_release(sprite->material);
	*sprite = {};
}

///////////////////////////////////////////

void sprite_draw(sprite_t sprite, matrix transform, color32 color) {
	sprite_drawer_add(sprite, transform, color);
}