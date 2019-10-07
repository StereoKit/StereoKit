#include "demo_sprites.h"

#include <stdlib.h>

#include "../../StereoKitC/stereokit.h"
using namespace sk;

///////////////////////////////////////////

sprite_t     sprite;
text_style_t sprite_text_style;

///////////////////////////////////////////

void demo_sprites_init() {
	material_t font_mat = material_create(shader_find("default/shader_font"));
	font_t font         = font_create("C:/Windows/Fonts/segoeui.ttf");
	sprite_text_style   = text_make_style(font, 0.1f, font_mat, text_align_x_right | text_align_y_center);
	material_release(font_mat);
	font_release    (font);

	sprite = sprite_create_file("../../Examples/Assets/test.png");
}

///////////////////////////////////////////

void demo_sprites_update() {
	static int frame = 0;
	frame++;
	int ct = (frame/30) % 10;
	for (size_t i = 0; i < ct; i++) {
		sprite_draw(sprite, matrix_trs(vec3_up * 0.1f + vec3{rand() / (float)RAND_MAX, 0, rand() / (float)RAND_MAX}, quat_lookat(vec3_zero, -vec3_up), vec3_one*0.1f));
	}
}

///////////////////////////////////////////

void demo_sprites_shutdown() {
	sprite_release(sprite);
}