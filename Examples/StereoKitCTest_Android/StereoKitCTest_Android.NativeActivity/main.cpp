#include <stereokit.h>
using namespace sk;
//#define SKR_IMPL
//#include "../../../StereoKitC/libraries/sk_gpu.h"

android_app* app;
bool app_visible = false;
bool app_run = true;

mesh_t mesh;
material_t mat;

static int32_t engine_handle_input(android_app*, AInputEvent* event) {
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		return 1;
	}
	return 0;
}

static void engine_handle_cmd(android_app*evt_app, int32_t cmd) {
	switch (cmd) {
	case APP_CMD_INIT_WINDOW:
		// The window is being shown, get it ready.
		if (evt_app->window != NULL) {
			__android_log_print(ANDROID_LOG_INFO, "StereoKit", "Initializing StereoKit...");
			//engine_init_display();
			//engine_draw_frame();
			log_set_filter(log_diagnostic);
			app_run = sk_init_from(evt_app->window, "StereoKitCTest_Android", runtime_mixedreality, true);
			app_visible = app_run;

			if (app_run) {
				mesh = mesh_gen_rounded_cube(vec3_one, 0.2f, 3);
				mat  = material_find(default_id_material);
			}
		}
		break;
	case APP_CMD_SAVE_STATE:   break;
	case APP_CMD_TERM_WINDOW:  app_run = false; break;
	case APP_CMD_GAINED_FOCUS: app_visible = true; break;
	case APP_CMD_LOST_FOCUS:   app_visible = false; break;
	}
}

void android_main(struct android_app* state) {
	app = state;
	app->onAppCmd     = engine_handle_cmd;
	app->onInputEvent = engine_handle_input;

	app_run = true;// sk_init_from(app->window, "StereoKitCTest_Android", runtime_mixedreality, true);

	while (app_run) {
		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue
		// to draw the next frame of animation.
		int events;
		struct android_poll_source* source;
		while (ALooper_pollAll(app_visible ? 0 : -1, nullptr, &events, (void**)&source) >= 0) {

			// Process this event.
			if (source != nullptr) source->process(state, source);

			// Check if we are exiting.
			if (state->destroyRequested != 0) {
				app_run = false;
				break;
			}
		}

		if (app_visible) sk_step([]() {
			render_add_mesh(mesh, mat, matrix_trs(vec3_zero));

			vec3 pos = { cosf(time_getf())*2, 1, sinf(time_getf())*2 };
			render_set_cam_root(matrix_trs(pos, quat_lookat(pos, vec3_zero)));
		});
	}

	sk_shutdown();
	//skr_shutdown();
}
