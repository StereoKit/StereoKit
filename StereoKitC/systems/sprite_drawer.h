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
bool sprite_drawer_init      ();
void sprite_drawer_step      ();
void sprite_drawer_shutdown  ();

} // namespace sk