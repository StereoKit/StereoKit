#pragma once

#include "stereokit.h"
#include "assets.h"

struct _font_t {
	asset_header_t header;
	tex2d_t font_tex;
};

void font_destroy(font_t font);