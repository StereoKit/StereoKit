#pragma once

#include "stereokit.h"
#include "font.h"

struct _text_style_t {
	font_t font;
	material_t material;
};
struct text_buffer_t {
	mesh_t  mesh;
	vert_t *verts;
	int     vert_count;
	int     vert_cap;
};