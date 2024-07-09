#include "sprite.h"
#include "assets.h"
#include "../libraries/ferr_hash.h"
#include "../libraries/array.h"
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

int32_t                 sprite_index   = 0;
array_t<sprite_atlas_t> sprite_atlases = {};

bool                    sprite_warning = false;

///////////////////////////////////////////

void sprite_atlas_place(sprite_atlas_t *atlas, sprite_t sprite);

///////////////////////////////////////////

sprite_t sprite_find(const char* id) {
	sprite_t result = (sprite_t)assets_find(id, asset_type_sprite);
	if (result != nullptr) {
		sprite_addref(result);
		return result;
	}
	return nullptr;
}

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
	material_set_depth_test  (result, depth_test_less_or_eq);
	shader_release(shader);

	return result;
}

///////////////////////////////////////////

sprite_t sprite_create(tex_t image, sprite_type_ type, const char *atlas_id) {
	if (type == sprite_type_atlased) {
		if (sprite_warning == false) {
			sprite_warning = true;
			log_diag("sprite_create: Atlased sprites not implemented yet! All atlased sprites will be switched to single.");
		}
		type = sprite_type_single;
	}

	// Make an id for the sprite
	const char* image_id = tex_get_id(image);
	char sprite_id[256];
	if (type == sprite_type_single) {
		snprintf(sprite_id, sizeof(sprite_id), "%s/spr", image_id);
	} else {
		snprintf(sprite_id, sizeof(sprite_id), "atlas_spr/%s/%s", atlas_id, image_id);
	}
	// Check if the id already exists
	sprite_t result = sprite_find(sprite_id);
	if (result != nullptr)
		return result;

	tex_addref(image);
	result = (_sprite_t*)assets_allocate(asset_type_sprite);

	assets_block_until((asset_header_t*)image, asset_state_loaded);

	result->texture = image;
	result->uvs[0]  = vec2{ 0,0 };
	result->uvs[1]  = vec2{ 1,1 };
	result->aspect  = tex_get_width(image) / (float)tex_get_height(image);
	result->size    = 1;
	result->dimensions_normalized = result->aspect > 1
		? vec2{ 1, 1.f/result->aspect } // Width is larger than height
		: vec2{ result->aspect, 1 };    // Height is larger than, or equal to width

	if (type == sprite_type_single) {
		result->buffer_index = -1;
		result->material     = sprite_create_material(sprite_index);
		material_set_texture(result->material, "diffuse", image);
	} else {
		// Find the atlas for this id
		uint64_t map_id = hash_fnv64_string(atlas_id);
		int64_t  index  = sprite_atlases.index_where(&sprite_atlas_t::id, map_id);
		
		// No atlas yet? Make one!
		if (index == -1) {
			sprite_atlas_t new_atlas = {};
			new_atlas.id         = map_id;
			new_atlas.material   = sprite_create_material(sprite_index);
			new_atlas.rects      = rect_atlas_create(0, 0); // This will get upscaled in sprite_atlas_place
			new_atlas.texture    = tex_create(tex_type_rendertarget);
			new_atlas.dirty_full = true;
			tex_set_address(new_atlas.texture, tex_address_clamp);
			sprite_drawer_add_buffer(new_atlas.material);
			material_set_texture(new_atlas.material, "diffuse", new_atlas.texture);

			index = sprite_atlases.add(new_atlas);
		}

		sprite_atlas_place(&sprite_atlases[index], result);
		result->buffer_index = index;
	}

	sprite_index += 1;
	sprite_set_id(result, sprite_id);
	return result;
}

///////////////////////////////////////////

int64_t sprite_area_inv(sprite_t sprite) {
	return -(int64_t)tex_get_width(sprite->texture) * (int64_t)tex_get_height(sprite->texture);
}

///////////////////////////////////////////

void sprite_atlas_place(sprite_atlas_t *atlas, sprite_t sprite) {
	// Binary insert the sprite so largest items are first in the list!
	int32_t at = atlas->sprites.binary_search(sprite_area_inv, sprite_area_inv(sprite));
	if (at < 0) at = ~at;
	atlas->sprites.insert(at, sprite);

	// Add the sprite to the atlas and queue it for rendering
	int32_t rect_id = rect_atlas_add(&atlas->rects, tex_get_width(sprite->texture), tex_get_height(sprite->texture));
	if (rect_id != -1) {
		rect_area_t rect = atlas->rects.packed[rect_id];
		sprite->uvs[0] =                  vec2{ rect.x / (float)atlas->rects.w, rect.y / (float)atlas->rects.h };
		sprite->uvs[1] = sprite->uvs[0] + vec2{ rect.w / (float)atlas->rects.w, rect.h / (float)atlas->rects.h };

		if (atlas->dirty_full == false) {
			at = atlas->dirty_queue.binary_search(sprite_area_inv, sprite_area_inv(sprite));
			if (at < 0) at = ~at;
			atlas->dirty_queue.insert(at, sprite);
		}
		return;
	}

	// The atlas was full, so we need to rebuild the atlas completely!
	atlas->dirty_full = true;
	bool full = rect_id == -1;
	while (full) {
		// Resize it to be a bit bigger than it was before, and at least bigger
		// than the sprite we're trying to add!
		int32_t new_w = atlas->rects.w;
		int32_t new_h = atlas->rects.h;
		if (new_w < tex_get_width (sprite->texture)) { new_w = 1 << (int)ceilf(log2f(tex_get_width (sprite->texture))); }
		if (new_h < tex_get_height(sprite->texture)) { new_h = 1 << (int)ceilf(log2f(tex_get_height(sprite->texture))); }
		if (new_w == atlas->rects.w && new_h == atlas->rects.h) {
			if (new_w == new_h) { new_w = new_w * 2; }
			else                { new_h = new_h * 2; }
		}

		// Clear out the atlas rects
		atlas->rects.packed    .clear();
		atlas->rects.free_space.clear();
		atlas->rects.used_area = 0;
		atlas->rects.w         = new_w;
		atlas->rects.h         = new_h;
		atlas->rects.free_space.add({ 0,0,new_w,new_h });

		// Add all the sprites back
		full = false;
		for (size_t i = 0; i < atlas->sprites.count; i++)
		{
			sprite_t curr      = atlas->sprites[i];
			int32_t  curr_rect = rect_atlas_add(&atlas->rects, tex_get_width(curr->texture), tex_get_height(curr->texture));
			if (curr_rect == -1) { full = true; break; }

			rect_area_t rect = atlas->rects.packed[curr_rect];
			curr->uvs[0] =                vec2{ rect.x/(float)new_w, rect.y/(float)new_h };
			curr->uvs[1] = curr->uvs[0] + vec2{ rect.w/(float)new_w, rect.h/(float)new_h };
		}
	}
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
	// Remove the sprite from the atlas!
	if (sprite->buffer_index != -1) {
		sprite_atlas_t *atlas = &sprite_atlases[sprite->buffer_index];

		int32_t x   = sprite->uvs[0].x * atlas->rects.w;
		int32_t y   = sprite->uvs[0].y * atlas->rects.h;
		int32_t idx = -1;
		for (size_t i = 0; i < atlas->rects.packed.count; i++)
		{
			if (atlas->rects.packed[i].x == x && atlas->rects.packed[i].y == y) {
				idx = i;
				break;
			}
		}
		if (idx != -1)
			rect_atlas_remove(&atlas->rects, idx);
		int32_t index = atlas->dirty_queue.index_of(sprite);
		if (index != -1) atlas->dirty_queue.remove(index);
		index = atlas->sprites.index_of(sprite);
		if (index != -1) atlas->sprites.remove(index);
	}

	tex_release     (sprite->texture);
	material_release(sprite->material);
	*sprite = {};
}

///////////////////////////////////////////

void sprite_draw(sprite_t sprite, matrix transform, text_align_ anchor_position, color32 color) {
	sprite_drawer_add_at(sprite, transform, anchor_position, color);
}


} // namespace sk
