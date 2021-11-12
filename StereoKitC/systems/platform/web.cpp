#include "win32.h"

#if defined(SK_OS_WEB)

#include <emscripten.h>
#include <emscripten/html5.h>

#include "../../stereokit.h"
#include "../../_stereokit.h"
#include "../../asset_types/texture.h"
#include "../../libraries/sokol_time.h"
#include "../system.h"
#include "../render.h"
#include "../input.h"
#include "../input_keyboard.h"
#include "../hand/input_hand.h"
#include "flatscreen_input.h"

namespace sk {

///////////////////////////////////////////

skg_swapchain_t web_swapchain  = {};
system_t       *web_render_sys = nullptr;

///////////////////////////////////////////

void web_resize(int width, int height) {
	if (width == sk_info.display_width && height == sk_info.display_height)
		return;
	sk_info.display_width  = width;
	sk_info.display_height = height;
	log_diagf("Resized to: %d<~BLK>x<~clr>%d", width, height);
	
	skg_swapchain_resize(&web_swapchain, sk_info.display_width, sk_info.display_height);
	render_update_projection();
}

///////////////////////////////////////////

bool web_start_pre_xr() {
	return true;
}

///////////////////////////////////////////

bool web_start_post_xr() {
	return true;
}

///////////////////////////////////////////

bool web_init() {
	web_render_sys = systems_find("FrameRender");
	return true;
}

///////////////////////////////////////////

void web_shutdown() {
}

///////////////////////////////////////////

bool web_start_flat() {
	sk_info.display_width  = sk_settings.flatscreen_width;
	sk_info.display_height = sk_settings.flatscreen_height;
	sk_info.display_type   = display_opaque;

	skg_tex_fmt_ color_fmt = skg_tex_fmt_rgba32_linear;
	skg_tex_fmt_ depth_fmt = render_preferred_depth_fmt(); // skg_tex_fmt_depthstencil

	web_swapchain = skg_swapchain_create(nullptr, color_fmt, skg_tex_fmt_none, width, height);
	sk_info.display_width  = web_swapchain.width;
	sk_info.display_height = web_swapchain.height;
	
	log_diagf("Created swapchain: %dx%d color:%s depth:%s", web_swapchain.width, web_swapchain.height, render_fmt_name((tex_format_)color_fmt), render_fmt_name((tex_format_)depth_fmt));

	flatscreen_input_init();

	//emscripten_request_animation_frame_loop(&main_step, 0);

	return true;
}

///////////////////////////////////////////

void web_stop_flat() {
	flatscreen_input_shutdown();
	skg_swapchain_destroy    (&web_swapchain);
}

///////////////////////////////////////////

void web_step_begin_xr() {
	//emscripten_request_animation_frame_loop(&main_step, 0);
}

///////////////////////////////////////////

void web_step_begin_flat() {
	flatscreen_input_update();
}

///////////////////////////////////////////

void web_step_end_flat() {
	skg_draw_begin();

	color128 col = render_get_clear_color();
	skg_swapchain_bind(&web_swapchain);
	skg_target_clear(true, &col.r);

	input_update_predicted();

	matrix view = render_get_cam_final ();
	matrix proj = render_get_projection();
	matrix_inverse(view, view);
	render_draw_matrix(&view, &proj, 1, render_get_filter());
	render_clear();

	web_render_sys->profile_frame_duration = stm_since(web_render_sys->profile_frame_start);
	skg_swapchain_present(&web_swapchain);
}

} // namespace sk

#endif // defined(SK_OS_WEB)