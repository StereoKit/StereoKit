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
	uint64_t   id;
	tex2d_t    texture;
	material_t material;
	sprite_t  *sprites;
	int32_t    sprite_count;
	int32_t    sprite_cap;
};

int32_t      sprite_index     = 0;
spritemap_t *sprite_maps      = nullptr;
int32_t      sprite_map_count = 0;

///////////////////////////////////////////

material_t sprite_create_material(int index_id) {
	char id[64];
	sprintf_s(id, 64, "render/sprite_mat_%d", index_id);
	material_t result = material_create(id, shader_find("default/shader_unlit"));
	material_set_alpha_mode(result, material_alpha_blend);
	material_set_cull      (result, material_cull_none);

	return result;
}

///////////////////////////////////////////

sprite_t sprite_create(tex2d_t image, sprite_type_ type, const char *atlas_id) {
	assets_addref(image->header);
	sprite_t result = (_sprite_t*)assets_allocate(asset_type_sprite, string_hash("_sprite", image->header.id));

	result->texture = image;
	result->uvs[0] = vec2{ 0,0 };
	result->uvs[1] = vec2{ 1,1 };
	result->aspect = image->width / (float)image->height;
	

	if (type == sprite_type_single) {
		result->size         = 1;
		result->buffer_index = -1;
		result->material     = sprite_create_material(sprite_index);
		material_set_texture(result->material, "diffuse", image);
	} else {
		// Find the atlas for this id
		uint64_t map_id = string_hash(atlas_id);
		int32_t  index  = -1;
		for (size_t i = 0; i < sprite_map_count; i++) {
			if (sprite_maps[i].id == map_id) {
				index = i;
				break;
			}
		}
		// No atlas yet? Make one!
		if (index == -1) {
			index             = sprite_map_count;
			sprite_map_count += 1;
			sprite_maps       = (spritemap_t*)realloc(sprite_maps, sizeof(spritemap_t) * sprite_map_count);

			sprite_maps[index].id       = map_id;
			sprite_maps[index].material = sprite_create_material(sprite_index);
		}

		// Add a sprite to the list

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