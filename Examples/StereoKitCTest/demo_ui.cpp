#include "demo_ui.h"

#include "../../StereoKitC/stereokit.h"
#include "../../StereoKitC/stereokit_ui.h"
using namespace sk;

///////////////////////////////////////////

sprite_t ui_sprite;

///////////////////////////////////////////

void demo_ui_init() {
	ui_sprite = sprite_create_file("../../Documentation/img/StereoKitLogo.png", sprite_type_single);
	ui_init();
}

///////////////////////////////////////////

void demo_ui_update() {
	static pose_t window_pose = //pose_t{ {0,0.5f,0},{0,0,0,1} };
		pose_t{ {0.85f,1,0.9f}, quat_lookat(vec3_zero, -vec3_one) };
	// input_hand(handed_right).root;
	ui_window_begin("Main", window_pose, vec2{ 24 }*cm2m);

	ui_button("Testing!"); ui_sameline();
	static char buffer[128] = {};
	ui_input("text", buffer, 128);
	
	static float val = 0.5f;
	static float val2 = 0.5f;
	ui_hslider("slider", val, 0, 1, 0.2f, 72*mm2m); ui_sameline();
	ui_hslider("slider2", val2, 0, 1, 0, 72*mm2m);
	if (input_key(key_mouse_left) & button_state_down)
		ui_image(ui_sprite, vec2{ 6,0 }*cm2m);
	if (ui_button("Press me!")) {
		ui_button("DYANMIC BUTTON!!");
	}

	ui_window_end();

	line_add(vec3_zero, input_hand(handed_right).root.position, {255,0,0,255}, 1*cm2m);
}

///////////////////////////////////////////

void demo_ui_shutdown() {
	// Release everything
	sprite_release(ui_sprite);
}