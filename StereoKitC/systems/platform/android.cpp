#ifdef __ANDROID__

#include "android.h"
#include "openxr.h"
#include "flatscreen_input.h"
#include "../render.h"
#include "../input.h"
#include "../../_stereokit.h"
#include "../../libraries/sk_gpu.h"

#include <android/native_activity.h>

namespace sk {

ANativeActivity* android_activity;
ANativeWindow*   android_window;
skr_swapchain_t  android_swapchain;

///////////////////////////////////////////

bool android_setup(void *from_window) {
	struct android_info_t {
		ANativeActivity *activity;
		ANativeWindow   *window;
	};
	android_info_t *info = (android_info_t *)from_window;
	android_window   = info->window;
	android_activity = info->activity;
	return true;
}

///////////////////////////////////////////

bool android_init() {
	skr_callback_log([](skr_log_ level, const char *text) {
		switch (level) {
		case skr_log_info:     log_diagf("sk_gpu: %s", text); break;
		case skr_log_warning:  log_warnf("sk_gpu: %s", text); break;
		case skr_log_critical: log_errf ("sk_gpu: %s", text); break;
		}
	});
	if (!skr_init(sk_app_name, android_window, nullptr))
		return false;
	log_diag("sk_gpu initialized!");

	android_swapchain = skr_swapchain_create(skr_tex_fmt_rgba32_linear, skr_tex_fmt_depth16, sk_info.display_width, sk_info.display_height);
	sk_info.display_width  = android_swapchain.width;
	sk_info.display_height = android_swapchain.height;
	log_diag("swapchain created!");

	flatscreen_input_init();
	return true;
}

///////////////////////////////////////////

void android_shutdown() {
	flatscreen_input_shutdown();
	skr_swapchain_destroy(&android_swapchain);
	skr_shutdown();
}

///////////////////////////////////////////

void android_step_begin() {
	flatscreen_input_update();
}

///////////////////////////////////////////

void android_step_end() {
	skr_draw_begin();

	color128   col    = render_get_clear_color();
	skr_tex_t *target = skr_swapchain_get_next(&android_swapchain);
	skr_tex_target_bind(target, true, &col.r);

	input_update_predicted();

	matrix view = render_get_cam_root  ();
	matrix proj = render_get_projection();
	matrix_inverse(view, view);
	render_draw_matrix(&view, &proj, 1);
	render_clear();
}

///////////////////////////////////////////

void android_vsync() {
	skr_swapchain_present(&android_swapchain);
}

} // namespace sk

#endif