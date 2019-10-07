#pragma once

#include "../stereokit.h"
#include "assets.h"

namespace sk {

struct _sprite_t {
	asset_header_t header;

	vec2       uvs[2];
	float      size;
	float      aspect;
	tex2d_t    texture;
	material_t material;
	int32_t    buffer_index;
};

void sprite_destroy(sprite_t sprite);

} // namespace sk