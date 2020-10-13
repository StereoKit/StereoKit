#include <stereokit.h>
#include <stereokit_ui.h>
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

			sk_android_info_t info = {};
			info.java_vm       = evt_app->activity->vm;
			info.jni_env       = evt_app->activity->env;
			info.asset_manager = evt_app->activity->assetManager;
			info.activity      = evt_app->activity->clazz;
			info.window        = evt_app->window;
			app_run = sk_init_from(&info, "StereoKitCTest_Android", runtime_mixedreality, true);
			app_visible = app_run;

			if (app_run) {
				mesh = mesh_gen_rounded_cube(vec3_one*.4f, 0.05f, 4);
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

pose_t pose = { vec3{0,0.5f,0}, quat_identity };
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

			if (sk_active_runtime() == runtime_flatscreen) {
				vec3 pos = { cosf(time_getf())*.6f, 0.5f, sinf(time_getf())*.6f };
				render_set_cam_root(matrix_trs(pos, quat_lookat(pos, vec3{0,0.25f,0})));
			}

			ui_window_begin("Hello!", pose);
			ui_button("play");
			ui_window_end();
		});
	}

	sk_shutdown();
	//skr_shutdown();
}
