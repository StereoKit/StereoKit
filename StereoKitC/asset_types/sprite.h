#pragma once

#include "../stereokit.h"
#include "../rect_atlas.h"
#include "assets.h"

namespace sk {

struct _sprite_t {
	asset_header_t header;

	vec2       uvs[2];
	vec2       dimensions_normalized;
	float      size;
	float      aspect;
	tex_t      texture;
	material_t material;
	int32_t    buffer_index;
};

struct sprite_atlas_t {
	uint64_t          id;
	tex_t             texture;
	material_t        material;
	array_t<sprite_t> sprites;
	rect_atlas_t      rects;

	array_t<sprite_t> dirty_queue;
	bool32_t          dirty_full;
};

extern array_t<sprite_atlas_t> sprite_atlases;

void sprite_destroy(sprite_t sprite);

} // namespace sk