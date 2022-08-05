#include "demo_sprites.h"

#include <stdlib.h>

#include <stereokit.h>

using namespace sk;

///////////////////////////////////////////

const int32_t sprite_count = 3;
sprite_t      sprites[sprite_count] = {};
text_style_t  sprite_text_style;

///////////////////////////////////////////

void demo_sprites_init() {
	font_t font       = font_find(default_id_font);
	sprite_text_style = text_make_style(font, 0.1f, { 1,1,1,1 });
	font_release(font);

	sprites[0] = sprite_create_file("power.png");
	sprites[1] = sprite_create_file("search.png");
	sprites[2] = sprite_create_file("mic_icon.png");
}

///////////////////////////////////////////

void demo_sprites_update() {
	for (int32_t x = -2; x <= 2; x++) {
		for (int32_t y = -2; y <= 2; y++) {
			sprite_draw_at(sprites[((x+2)+(y+2)*5) % sprite_count], matrix_trs({x * 0.1f,y * 0.1f,-0.5f}, quat_lookat(vec3_zero, -vec3_forward), vec3_one * 0.1f), text_align_center);
		}
	}
}

///////////////////////////////////////////

void demo_sprites_shutdown() {
	for (int32_t i = 0; i < sprite_count; i++)
		sprite_release(sprites[i]);
}