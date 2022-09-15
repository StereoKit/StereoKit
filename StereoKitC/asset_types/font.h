#pragma once

#include "../stereokit.h"
#include "../libraries/array.h"
#include "../rect_atlas.h"
#include "assets.h"

namespace sk {

struct font_char_t {
	float x0,y0,x1,y1;
	float u0,v0,u1,v1;
	float xadvance;
};

struct font_glyph_t {
	int32_t idx;
	int32_t font;
};

struct _font_t {
	asset_header_t header;
	tex_t       font_tex;
	font_char_t characters[128];
	float       character_ascend;
	float       character_descend;
	float       line_gap;
	float       space_width;

	hashmap_t<font_glyph_t, font_char_t> glyph_map;
	hashmap_t<char32_t,     font_char_t> character_map;
	array_t  <font_glyph_t>              update_queue;
	array_t  <int32_t>                   font_ids;

	rect_atlas_t   atlas;
	uint8_t       *atlas_data;
};

font_t font_create_default();
void   font_destroy       (font_t font);
void   font_update_fonts  ();

const font_char_t *font_get_glyph(font_t font, char32_t character);

} // namespace sk