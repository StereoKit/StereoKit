#include "demo_sprites.h"

#include <stdlib.h>

#include <stereokit.h>

using namespace sk;

///////////////////////////////////////////

sprite_t     sprite;
text_style_t sprite_text_style;

///////////////////////////////////////////

void demo_sprites_init() {
	font_t font       = font_find(default_id_font);
	sprite_text_style = text_make_style(font, 0.1f, { 1,1,1,1 });
	font_release(font);

	sprite = sprite_create_file("power.png");
}

///////////////////////////////////////////

void demo_sprites_update() {
	for (int32_t x = -2; x <= 2; x++) {
		for (int32_t y = -2; y <= 2; y++) {
			sprite_draw(sprite, matrix_trs({x*0.1f,y*0.1f,-0.5f}, quat_lookat(vec3_zero, -vec3_forward), vec3_one * 0.1f), text_align_center);
		}
	}
}

///////////////////////////////////////////

void demo_sprites_shutdown() {
	sprite_release(sprite);
}