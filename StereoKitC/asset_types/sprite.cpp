#include "sprite.h"
#include "assets.h"
#include "../libraries/ferr_hash.h"
#include "../systems/sprite_drawer.h"
#include "../sk_math.h"
#include "../sk_memory.h"

#include <stdio.h>

namespace sk {

struct sprite_inst_t {
	vec2    uvs[2];
	color32 color;
	matrix  transform;
};

struct spritemap_t {
	uint64_t   id;
	tex_t      texture;
	material_t material;
	sprite_t  *sprites;
	int32_t    sprite_count;
	int32_t    sprite_cap;
};

int32_t      sprite_index     = 0;
spritemap_t *sprite_maps      = nullptr;
int32_t      sprite_map_count = 0;

///////////////////////////////////////////

void sprite_set_id(sprite_t sprite, const char *id) {
	assets_set_id(&sprite->header, id);
}

///////////////////////////////////////////

const char* sprite_get_id(const sprite_t sprite) {
	return sprite->header.id_text;
}

///////////////////////////////////////////

material_t sprite_create_material(int index_id) {
	char id[64];
	snprintf(id, sizeof(id), "render/sprite_mat_%d", index_id);
	shader_t   shader = shader_find(default_id_shader_unlit_clip);
	material_t result = material_create(shader);
	material_set_id          (result, id);
	material_set_transparency(result, transparency_blend);
	material_set_cull        (result, cull_none);
	shader_release(shader);

	return result;
}

///////////////////////////////////////////

sprite_t sprite_create(tex_t image, sprite_type_ type, const char *atlas_id) {
	tex_addref(image);
	sprite_t result = (_sprite_t*)assets_allocate(asset_type_sprite);

	assets_block_until((asset_header_t*)image, asset_state_loaded_meta);

	result->texture = image;
	result->uvs[0] = vec2{ 0,0 };
	result->uvs[1] = vec2{ 1,1 };
	result->aspect = tex_get_width(image) / (float)tex_get_height(image);
	if (result->aspect > 1) // Width is larger than height
		result->dimensions_normalized = { 1, 1.f / result->aspect };
	else                    // Height is larger than, or equal to width
		result->dimensions_normalized = { result->aspect, 1 };

	if (type == sprite_type_atlased) {
		log_diag("sprite_create: Atlased sprites not implemented yet! Switching to single.");
		type = sprite_type_single;
	}
	
	if (type == sprite_type_single) {
		result->size         = 1;
		result->buffer_index = -1;
		result->material     = sprite_create_material(sprite_index);
		material_set_texture(result->material, "diffuse", image);
	} else {
		// Find the atlas for this id
		uint64_t     map_id = hash_fnv64_string(atlas_id);
		spritemap_t *map    = nullptr;
		int32_t      index  = -1;
		for (int32_t i = 0; i < sprite_map_count; i++) {
			if (sprite_maps[i].id == map_id) {
				map   = &sprite_maps[i];
				index = i;
				break;
			}
		}
		// No atlas yet? Make one!
		if (map == nullptr) {
			index             = sprite_map_count;
			sprite_map_count += 1;
			sprite_maps       = sk_realloc_t(spritemap_t, sprite_maps, sprite_map_count);
			map               = &sprite_maps[sprite_map_count-1];

			*map = {};
			map->id       = map_id;
			map->material = sprite_create_material(sprite_index);
			sprite_drawer_add_buffer(map->material);
		}

		// Add a sprite to the list
		if (map->sprite_count + 1 > map->sprite_cap) {
			map->sprite_cap = maxi(1, map->sprite_cap * 2);
			map->sprites    = sk_realloc_t(sprite_t, map->sprites, map->sprite_cap);
		}
		map->sprites[map->sprite_count] = result;
		map->sprite_count += 1;

		result->buffer_index = index;
	}

	sprite_index += 1;
	return result;
}

///////////////////////////////////////////

sprite_t sprite_create_file(const char *filename, sprite_type_ type, const char *atlas_id) {
	tex_t image = tex_create_file(filename);
	if (image == nullptr) return nullptr;

	tex_set_address(image, tex_address_clamp);
	sprite_t result = sprite_create(image, type, atlas_id);
	tex_release(image);
	return result;
}

///////////////////////////////////////////

void sprite_addref(sprite_t sprite) {
	assets_addref(&sprite->header);
}

///////////////////////////////////////////

void sprite_release(sprite_t sprite) {
	if (sprite == nullptr)
		return;
	assets_releaseref(&sprite->header);
}

///////////////////////////////////////////

float sprite_get_aspect(sprite_t sprite) {
	return sprite->aspect;
}

///////////////////////////////////////////

int32_t sprite_get_width(sprite_t sprite) {
	return tex_get_width(sprite->texture);
}

///////////////////////////////////////////

int32_t sprite_get_height(sprite_t sprite) {
	return tex_get_height(sprite->texture);
}

///////////////////////////////////////////

vec2 sprite_get_dimensions_normalized(sprite_t sprite) {
	return sprite->dimensions_normalized;
}

///////////////////////////////////////////

void sprite_destroy(sprite_t sprite) {
	tex_release     (sprite->texture);
	material_release(sprite->material);
	*sprite = {};
}

///////////////////////////////////////////

void sprite_draw(sprite_t sprite, const matrix &transform, color32 color) {
	sprite_drawer_add(sprite, transform, color);
}

///////////////////////////////////////////

void sprite_draw_at(sprite_t sprite, matrix transform, text_align_ anchor_position, color32 color) {
	sprite_drawer_add_at(sprite, transform, anchor_position, color);
}


} // namespace sk