#include "demo_ui.h"

#include "../../StereoKitC/stereokit.h"
#include "../../StereoKitC/stereokit_ui.h"

///////////////////////////////////////////

sprite_t ui_sprite;

///////////////////////////////////////////

void demo_ui_init() {
	ui_sprite = sprite_create_file("../../Documentation/img/StereoKitLogo.png", sprite_type_single);
	sk_ui_init();
}

///////////////////////////////////////////

void demo_ui_update() {
	static pose_t window_pose = //pose_t{ {0,0.5f,0},{0,0,0,1} };
		pose_t{ {0.85f,1,0.9f}, quat_lookat(vec3_zero, -vec3_one) };
	// input_hand(handed_right).root;
	sk_ui_window_begin("Main", window_pose, vec2{ 24 }*cm2m);

	sk_ui_button("Testing!\nok"); sk_ui_sameline();
	sk_ui_button("Another");
	
	static float val = 0.5f;
	static float val2 = 0.5f;
	sk_ui_hslider("slider", val, 0, 1, 0.2f, 72*mm2m); sk_ui_sameline();
	sk_ui_hslider("slider2", val2, 0, 1, 0, 72*mm2m);
	sk_ui_image(ui_sprite, vec2{ 6,0 }*cm2m);
	if (sk_ui_button("Press me!")) {
		sk_ui_button("DYANMIC BUTTON!!");
	}

	sk_ui_window_end();
}

///////////////////////////////////////////

void demo_ui_shutdown() {
	// Release everything
	sprite_release(ui_sprite);
}