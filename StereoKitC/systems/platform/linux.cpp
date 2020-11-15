#ifdef __linux__

#include "../../log.h"
#include "../../stereokit.h"
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

bool linux_init() {
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

bool linux_start() {
	sk_info.display_width  = sk_settings.flatscreen_width;
	sk_info.display_height = sk_settings.flatscreen_height;
	sk_info.display_type   = display_opaque;
	skg_tex_fmt_ color_fmt = skg_tex_fmt_rgba32_linear;
	skg_tex_fmt_ depth_fmt = skg_tex_fmt_depth16;
	linux_swapchain = skg_swapchain_create(&win, color_fmt, depth_fmt, sk_info.display_width, sk_info.display_height);
	sk_info.display_width  = linux_swapchain.width;
	sk_info.display_height = linux_swapchain.height;

	flatscreen_input_init();

	return true;
}

///////////////////////////////////////////

bool linux_get_cursor(vec2 &out_pos) {
	Window root_window, child_window;
	int root_cursor_pos[2], win_cursor_pos[2];
	uint32_t mask_return;

	bool result = XQueryPointer(
		dpy, win,
		&root_window, &child_window, //Root window, child window
		&root_cursor_pos[0], &root_cursor_pos[1], //Root relative X and Y coordinates
		&win_cursor_pos[0], &win_cursor_pos[1], //Window relative X and Y coordinates
		&mask_return //Key mask
	);

	if(result) {
		out_pos.x = (float) win_cursor_pos[0];
		out_pos.y = (float) win_cursor_pos[1];
	}

	return result;
}

///////////////////////////////////////////

void linux_stop() {
	flatscreen_input_shutdown();
	skg_swapchain_destroy(&linux_swapchain);
}

///////////////////////////////////////////

void linux_shutdown() {
	glXMakeCurrent(dpy, None, nullptr);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

///////////////////////////////////////////

void linux_step_begin() {
	flatscreen_input_update();
}

///////////////////////////////////////////

void linux_step_end() {
	skg_draw_begin();

	color128 col = render_get_clear_color();
	skg_swapchain_bind(&linux_swapchain, true, &col.r);

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
