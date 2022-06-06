#include "demo_ui.h"

#include <stereokit.h>
#include <stereokit_ui.h>
using namespace sk;

///////////////////////////////////////////

void demo_ui_layout_init() {
}

///////////////////////////////////////////

void demo_ui_layout_update() {
	static pose_t window_pose = //pose_t{ {0,0.5f,0},{0,0,0,1} };
		pose_t{ {0.25,0.25,-0.25f}, quat_lookat({0.25,0.25,-0.25f}, {0,0.25,0}) };

	ui_window_begin("Main", window_pose);

	static ui_pad_ pad_mode = ui_pad_outside;
	ui_panel_begin(pad_mode);
	{
		ui_label("UI Panel Padding");

		bool32_t mode = pad_mode == ui_pad_none;
		if (ui_toggle("None", mode) && mode) pad_mode = ui_pad_none;
		ui_sameline();
		mode = pad_mode == ui_pad_outside;
		if (ui_toggle("Outside", mode) && mode) pad_mode = ui_pad_outside;
		ui_sameline();
		mode = pad_mode == ui_pad_inside;
		if (ui_toggle("Inside", mode) && mode) pad_mode = ui_pad_inside;
	}
	ui_panel_end();

	ui_button("A"); ui_sameline();
	ui_panel_begin(pad_mode);
	{
		ui_button("B"); ui_sameline();
		ui_button("C");
	}
	ui_panel_end();
	ui_sameline();
	ui_button("D");

	ui_button("A"); ui_sameline();
	ui_panel_begin(pad_mode);
	{
		ui_button("B"); ui_sameline();
		ui_button("C"); ui_sameline();
	}
	ui_panel_end();
	ui_sameline();
	ui_button("D");

	ui_button("A"); ui_sameline();
	ui_button("B"); ui_sameline();
	ui_button("C"); ui_sameline();
	ui_button("D");

	ui_window_end();
}

///////////////////////////////////////////

void demo_ui_layout_shutdown() {
}