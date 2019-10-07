#pragma once

#include "../stereokit.h"
#include "../libraries/stb_truetype.h"
#include "assets.h"

namespace sk {

struct font_char_t {
	float x0,y0,x1,y1;
	float u0,v0,u1,v1;
	float xadvance;
};
struct _font_t {
	asset_header_t header;
	tex2d_t font_tex;
	font_char_t characters[128];
};

void font_destroy(font_t font);

} // namespace sk