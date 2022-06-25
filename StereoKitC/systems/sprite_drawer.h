#pragma once

#include "../stereokit.h"

namespace sk {

struct sprite_buffer_t {
	material_t material;
	mesh_t     mesh;
	vert_t    *verts;
	uint32_t   id;
	int32_t    vert_count;
	int32_t    vert_cap;
};

void sprite_drawer_add_buffer(material_t material);
void sprite_drawer_add       (sprite_t sprite, const matrix &at, color32 color = {255,255,255,255});
void sprite_drawer_add_at    (sprite_t sprite, matrix at, text_align_ anchor_position, color32 color);
bool sprite_drawer_init      ();
void sprite_drawer_update    ();
void sprite_drawer_shutdown  ();

} // namespace sk