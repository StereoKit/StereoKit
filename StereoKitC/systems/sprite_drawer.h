#pragma once

#include "../stereokit.h"
#include "../asset_types/sprite.h"

namespace sk {

struct sprite_buffer_t {
	material_t material;
	mesh_t     mesh;
	vert_t    *verts;
	uint32_t   id;
	int        vert_count;
	int        vert_cap;
};

void sprite_drawer_add_buffer(material_t material);
void sprite_drawer_add     (sprite_t sprite, const matrix &at, color32 color = {255,255,255,255});
bool sprite_drawer_init    ();
void sprite_drawer_update  ();
void sprite_drawer_shutdown();

} // namespace sk