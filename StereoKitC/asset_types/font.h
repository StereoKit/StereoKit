#pragma once

#include "../stereokit.h"
#include "../libraries/stb_truetype.h"
#include "../libraries/array.h"
#include "../rect_atlas.h"
#include "assets.h"

namespace sk {

struct font_char_t {
	float x0,y0,x1,y1;
	float u0,v0,u1,v1;
	float xadvance;
};
struct _font_t {
	asset_header_t header;
	tex_t       font_tex;
	font_char_t characters[128];
	float       character_height;
	int32_t     character_resolution;
	int32_t     character_ascend;
	int32_t     character_descend;
	int32_t     line_gap;
	float       space_width;

	hashmap_t<int32_t,  font_char_t> glyph_map;
	hashmap_t<char32_t, font_char_t> character_map;
	array_t  <char32_t>              update_queue;

	float          font_scale;
	stbtt_fontinfo font_info;
	void          *font_file;
	rect_atlas_t   atlas;
	uint8_t       *atlas_data;
};

void font_destroy     (font_t font);
void font_update_fonts();

const font_char_t *font_get_glyph(font_t font, char32_t character);

} // namespace sk