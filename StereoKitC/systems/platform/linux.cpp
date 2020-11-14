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

skg_swapchain_t linux_swapchain;

///////////////////////////////////////////

Display                 *dpy;
Window                  root;

GLint                   att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_DOUBLEBUFFER, None };
XVisualInfo             *vi;
Colormap                cmap;
XSetWindowAttributes    swa;
Window                  win;

bool linux_setup() {
	dpy = XOpenDisplay(0);
	if(dpy == nullptr) {
		printf("\n\tcannot connect to X server\n\n");
		exit(0);
	}

	root = DefaultRootWindow(dpy);
	vi = glXChooseVisual(dpy, 0, att);

	if(vi == nullptr) {
		printf("\n\tno appropriate visual found\n\n");
    	exit(0);
    }

	cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);

	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask;

	win = XCreateWindow(dpy, root, 0, 0, 1280, 720, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);

	XMapWindow(dpy, win);
	XStoreName(dpy, win, sk_app_name);

	skg_setup_xlib(dpy, vi, &win);

	return true;
}

///////////////////////////////////////////

bool linux_init() {
	linux_setup();

	skg_callback_log([](skg_log_ level, const char *text) {
		switch (level) {
		case skg_log_info:     log_diagf("sk_gpu: %s", text); break;
		case skg_log_warning:  log_warnf("sk_gpu: %s", text); break;
		case skg_log_critical: log_errf ("sk_gpu: %s", text); break;
		}
	});

	skg_tex_fmt_ color_fmt = skg_tex_fmt_rgba32_linear;
	skg_tex_fmt_ depth_fmt = skg_tex_fmt_depth16;
	linux_swapchain = skg_swapchain_create(nullptr, color_fmt, depth_fmt, sk_info.display_width, sk_info.display_height);
	sk_info.display_width  = linux_swapchain.width;
	sk_info.display_height = linux_swapchain.height;
	// log_diagf("Created swapchain: %dx%d color:%s depth:%s", linux_swapchain.width, linux_swapchain.height, render_fmt_name((tex_format_)color_fmt), render_fmt_name((tex_format_)depth_fmt));

	flatscreen_input_init();


	return true;
}

///////////////////////////////////////////

void linux_shutdown() {
	flatscreen_input_shutdown();
	skg_swapchain_destroy(&linux_swapchain);

	glXMakeCurrent(dpy, None, nullptr);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);

	skg_shutdown();
}

///////////////////////////////////////////

void linux_step_begin() {
	flatscreen_input_update();
}

///////////////////////////////////////////

void linux_step_end() {
	skg_draw_begin();

	color128 col = render_get_clear_color();
	skg_tex_t *target;
	skg_tex_target_bind(target, true, &col.r);

	input_update_predicted();

	matrix view = render_get_cam_root  ();
	matrix proj = render_get_projection();
	matrix_inverse(view, view);
	render_draw_matrix(&view, &proj, 1);
	render_clear();
}

///////////////////////////////////////////

void linux_vsync() {
	skg_swapchain_present(&linux_swapchain);
}

} // namespace sk

#endif
