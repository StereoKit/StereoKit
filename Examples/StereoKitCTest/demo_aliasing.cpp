#include "demo_aliasing.h"

#include <stereokit.h>
#include <stereokit_ui.h>
#include <stdio.h>
using namespace sk;

///////////////////////////////////////////

void demo_aliasing_init() {
}

///////////////////////////////////////////

void demo_aliasing_update() {
	static pose_t window_pose =
		pose_t{ {0.25,0.25,-0.25f}, quat_lookat({0.25,0.25,-0.25f}, {0,0.25,0}) };

	static float render_scale       =        render_get_scaling();
	static float render_multisample = (float)render_get_multisample();

	ui_window_begin("Aliasing settings", &window_pose);
	ui_label("These settings only work in XR!");

	ui_hseparator();
	ui_label("MSAA");
	
	char txt[64];
	snprintf(txt, 64, "%d", render_get_multisample());
	ui_label_sz(txt, { .04f,0 });
	ui_sameline();
	if (ui_hslider("msaa", render_multisample, 1, 8, 1))
		render_set_multisample((int32_t)render_multisample);

	ui_hseparator();
	ui_label("Render scale");

	snprintf(txt, 64, "%.2f", render_get_scaling());
	ui_label_sz(txt, { .04f,0 });
	ui_sameline();
	if (ui_hslider("scaling", render_scale, 0.2f, 2))
		render_set_scaling(render_scale);

	ui_window_end();

	if (input_key(key_left ) & button_state_just_active) { render_scale -= 0.1f; render_set_scaling(render_scale); }
	if (input_key(key_right) & button_state_just_active) { render_scale += 0.1f; render_set_scaling(render_scale); }
	if (input_key(key_up   ) & button_state_just_active) { render_multisample += 1; render_set_multisample((int32_t)render_multisample); }
	if (input_key(key_down ) & button_state_just_active) { render_multisample -= 1; render_set_multisample((int32_t)render_multisample); }
}

///////////////////////////////////////////

void demo_aliasing_shutdown() {
}