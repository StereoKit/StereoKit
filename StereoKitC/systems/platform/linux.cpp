#ifdef __linux__

#include "../../log.h"
#include "linux.h"
#include "openxr.h"
#include "flatscreen_input.h"
#include "../render.h"
#include "../input.h"
#include "../../_stereokit.h"
#include "../../libraries/sk_gpu.h"

namespace sk {

skr_swapchain_t   linux_swapchain;

///////////////////////////////////////////

bool linux_setup(void *from_window) {
	return true;
}

///////////////////////////////////////////

	return true;
}

///////////////////////////////////////////

bool linux_init() {
	skr_callback_log([](skr_log_ level, const char *text) {
		switch (level) {
		case skr_log_info:     log_diagf("sk_gpu: %s", text); break;
		case skr_log_warning:  log_warnf("sk_gpu: %s", text); break;
		case skr_log_critical: log_errf ("sk_gpu: %s", text); break;
		}
	});
	if (!skr_init(sk_app_name, linux_window, nullptr))
		return false;
	log_diag("sk_gpu initialized!");

	skr_tex_fmt_ color_fmt = skr_tex_fmt_rgba32_linear;
	skr_tex_fmt_ depth_fmt = skr_tex_fmt_depth16;
	linux_swapchain = skr_swapchain_create(color_fmt, depth_fmt, sk_info.display_width, sk_info.display_height);
	sk_info.display_width  = linux_swapchain.width;
	sk_info.display_height = linux_swapchain.height;
	log_diagf("Created swapchain: %dx%d color:%s depth:%s", linux_swapchain.width, linux_swapchain.height, render_fmt_name((tex_format_)color_fmt), render_fmt_name((tex_format_)depth_fmt));

	flatscreen_input_init();
	return true;
}

///////////////////////////////////////////

void linux_shutdown() {
	flatscreen_input_shutdown();
	skr_swapchain_destroy(&linux_swapchain);
	skr_shutdown();

	if (linux_vm)
		linux_vm->DetachCurrentThread();
}

///////////////////////////////////////////

void linux_step_begin() {
	flatscreen_input_update();
}

///////////////////////////////////////////

void linux_step_end() {
	skr_draw_begin();

	color128   col    = render_get_clear_color();
	skr_tex_t *target = skr_swapchain_get_next(&linux_swapchain);
	skr_tex_target_bind(target, true, &col.r);

	input_update_predicted();

	matrix view = render_get_cam_root  ();
	matrix proj = render_get_projection();
	matrix_inverse(view, view);
	render_draw_matrix(&view, &proj, 1);
	render_clear();
}

///////////////////////////////////////////

void linux_vsync() {
	skr_swapchain_present(&linux_swapchain);
}

} // namespace sk

#endif
