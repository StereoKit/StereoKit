#include "demo_ui.h"

#include <stereokit.h>
#include <stereokit_ui.h>
using namespace sk;

///////////////////////////////////////////

sprite_t ui_sprite;
sprite_t ui_search_sprite;

///////////////////////////////////////////

void demo_ui_init() {
	ui_sprite        = sprite_create_file("StereoKitWide.png", sprite_type_single);
	ui_search_sprite = sprite_create_file("search.png",        sprite_type_single);
}

///////////////////////////////////////////

void demo_ui_update() {
	static pose_t window_pose = //pose_t{ {0,0.5f,0},{0,0,0,1} };
		pose_t{ {0.25,0.25,-0.25f}, quat_lookat({0.25,0.25,-0.25f}, {0,0.25,0}) };
	// input_hand(handed_right).root;
	ui_window_begin("Main", window_pose, vec2{ 24 }*cm2m);

	ui_image(ui_sprite, vec2{ 6,0 }*cm2m);
	
	ui_button("Testing!"); ui_sameline();
	static char buffer[128] = {};
	ui_input("text", buffer, 128, {16*cm2m,ui_line_height()});
	
	static ui_btn_layout_ layout = ui_btn_layout_left;
	if (ui_button_img("Find", ui_search_sprite, layout))
		layout = (ui_btn_layout_)((layout + 1) % 3);
	
	static float val = 0.5f;
	static float val2 = 0.5f;
	ui_hslider("slider", val, 0, 1, 0.2f, 72*mm2m, ui_confirm_pinch); ui_sameline();
	ui_hslider("slider2", val2, 0, 1, 0, 72*mm2m, ui_confirm_push);
	
	ui_panel_begin();
	ui_label("Last Element Focus");
	static float val3 = 0.5f;
	ui_hslider("slider3", val3, 0, 1, 0.2f, 0, ui_confirm_pinch);
	if (ui_last_element_hand_used(handed_left ) & button_state_active) ui_label("Left!");
	if (ui_last_element_hand_used(handed_right) & button_state_active) ui_label("Right!");
	if (ui_last_element_focused  (            ) & button_state_active) ui_label("Focused!");
	if (ui_last_element_active   (            ) & button_state_active) ui_label("Active!");
	ui_panel_end();
	
	if (ui_button("Press me!")) {
		ui_button("DYNAMIC BUTTON!!");
	}

	ui_progress_bar(sinf(time_getf())*0.5f+0.5f);

	ui_text(u8"古池や\n蛙飛び込む\n水の音\n- Matsuo Basho");
	ui_text(u8"Съешь же ещё этих мягких французских булок да выпей чаю. Широкая электрификация южных губерний даст мощный толчок подъёму сельского хозяйства. В чащах юга жил бы цитрус? Да, но фальшивый экземпляр!");

	ui_window_end();
}

///////////////////////////////////////////

void demo_ui_shutdown() {
	// Release everything
	sprite_release(ui_sprite);
	sprite_release(ui_search_sprite);
}