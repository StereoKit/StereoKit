#pragma once

#include "../stereokit.h"
#include "../asset_types/font.h"

namespace sk {

struct _text_style_t {
	font_t      font;
	uint32_t    buffer_index;
	color32     color;
	float       size;
	float       char_height;
	float       line_spacing;
};
struct text_buffer_t {
	font_t     font;
	material_t material;
	mesh_t     mesh;
	vert_t    *verts;
	uint32_t   id;
	int        vert_count;
	int        vert_cap;
};
struct text_stepper_t {
	_text_style_t *style;
	text_align_ align;
	int32_t line_remaining;
	bool32_t wrap;
	vec2 start;
	vec2 bounds;
	vec2 pos;
};

vec2 text_line_size(text_style_t style, const char *text);

void text_update();
void text_shutdown();

} // namespace sk