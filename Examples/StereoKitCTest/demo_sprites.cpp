#include "demo_sprites.h"

#include "../../StereoKitC/stereokit.h"

///////////////////////////////////////////

sprite_t    sprite;

///////////////////////////////////////////

void demo_sprites_init() {
	sprite = sprite_create_file("../../Examples/Assets/icon.png", sprite_type_single);
}

///////////////////////////////////////////

void demo_sprites_update() {
	sprite_draw(sprite, matrix_trs(vec3_up * 0.1f, quat_lookat(vec3_zero, -vec3_up)));
}

///////////////////////////////////////////

void demo_sprites_shutdown() {
	sprite_release(sprite);
}