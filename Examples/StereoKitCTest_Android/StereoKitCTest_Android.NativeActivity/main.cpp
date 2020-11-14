#include <stereokit.h>
#include <stereokit_ui.h>
using namespace sk;

android_app* app;
bool app_visible = false;
bool app_run = true;

mesh_t mesh;
material_t mat;

static void engine_handle_cmd(android_app*evt_app, int32_t cmd) {
	switch (cmd) {
	case APP_CMD_INIT_WINDOW:    app_visible = app_run = sk_set_window(evt_app->window, sk_active_runtime()); break;
	case APP_CMD_WINDOW_RESIZED:
	case APP_CMD_CONFIG_CHANGED: app_run = sk_set_window(evt_app->window, sk_active_runtime()); break;
	case APP_CMD_TERM_WINDOW:    app_run = sk_set_window(nullptr,         sk_active_runtime()); break;
	case APP_CMD_GAINED_FOCUS:   app_visible = true; break;
	case APP_CMD_LOST_FOCUS:     app_visible = false; break;
	}
}

pose_t pose = { vec3{0,0.5f,0}, quat_identity };
void android_main(struct android_app* state) {
	app = state;
	app->onAppCmd = engine_handle_cmd;

	log_set_filter(log_diagnostic);

	settings_t settings = sk_get_settings();
	settings.android_activity = state->activity->clazz;
	settings.android_java_vm  = state->activity->vm;
	sk_set_settings(settings);
	if (!sk_init("StereoKitCTest_Android", runtime_flatscreen, true))
		return;

	mesh = mesh_gen_rounded_cube(vec3_one*.4f, 0.05f, 4);
	mat  = material_find(default_id_material);
	render_enable_skytex(false);

	while (app_run) {
		int events;
		struct android_poll_source* source;
		while (ALooper_pollAll(app_visible ? 0 : -1, nullptr, &events, (void**)&source) >= 0) {
			if (source                  != nullptr) source->process(state, source);
			if (state->destroyRequested != 0      ) app_run = false;
		}

		if (app_visible) sk_step([]() {
			render_add_mesh(mesh, mat, matrix_trs(vec3_zero));

			float f = (cosf(time_getf()) + 1) / 2;
			render_set_clear_color({f,f,f,1});

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
}
