// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

#include "window.h"

#include "../_stereokit.h"
#include "../device.h"
#include "../log.h"
#include "../sk_memory.h"
#include "../platforms/_platform.h"
#include "../asset_types/texture.h"
#include "../systems/world.h"
#include "../systems/input.h"
#include "../systems/input_keyboard.h"
#include "../systems/system.h"
#include "../systems/render.h"
#include "../systems/render_pipeline.h"
#include "../libraries/stref.h"
#include "../libraries/sokol_time.h"

///////////////////////////////////////////

using namespace sk;

struct window_backend_state_t {
	system_t*           render_sys;
	platform_win_t      window;
	pipeline_surface_id surface;
};
static window_backend_state_t* local = {};

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

void window_physical_key_interact();
void window_surface_resize       (pipeline_surface_id surface, int32_t width, int32_t height);

///////////////////////////////////////////

bool window_init() {
	const sk_settings_t* settings = sk_get_settings_ref();

	device_data.has_hand_tracking = false;
	device_data.has_eye_gaze      = false;
	device_data.tracking          = device_tracking_none;
	device_data.display_blend     = display_blend_opaque;
	device_data.display_type      = display_type_flatscreen;
	device_data.name              = string_copy("Window");
	device_data.runtime           = string_copy("None");

	world_origin_mode   = origin_mode_local;
	world_origin_offset = { {0,0,0}, quat_identity };

	local = sk_malloc_zero_t(window_backend_state_t, 1);
	local->render_sys = systems_find("FrameRender");

	recti_t win_size = {};
	if (!platform_key_load_bytes("WindowLocation", &win_size, sizeof(win_size))) {
		win_size.x = settings->flatscreen_pos_x;
		win_size.y = settings->flatscreen_pos_y;
		win_size.w = settings->flatscreen_width;
		win_size.h = settings->flatscreen_height;
	}

	switch (platform_win_type()) {
	case platform_win_type_creatable: {
		local->window = platform_win_make(settings->app_name, win_size, platform_surface_swapchain);
		if (local->window == -1) {
			log_fail_reason(90, log_error, "Failed to create platform window.");
			return false;
		}
	} break;
	case platform_win_type_existing: {
		local->window = platform_win_get_existing(platform_surface_swapchain);
	} break;
	default: return false;
	}

	local->surface = render_pipeline_surface_create(tex_format_rgba32, render_preferred_depth_fmt(), 1, 1, 1);
	skg_swapchain_t* swapchain = platform_win_get_swapchain(local->window);
	if (swapchain)
		window_surface_resize(local->surface, swapchain->width, swapchain->height);

	return true;
}
///////////////////////////////////////////

void window_physical_key_interact() {
	// On desktop, we want to hide soft keyboards on physical presses
	input_set_last_physical_keypress_time(time_totalf_unscaled());
	platform_keyboard_show(false, text_context_text);
}

///////////////////////////////////////////

void window_surface_resize(pipeline_surface_id surface, int32_t width, int32_t height) {
	device_data.display_width  = width;
	device_data.display_height = height;

	if (render_pipeline_surface_resize(surface, width, height, 8))
		render_update_projection();
}

///////////////////////////////////////////

void window_shutdown() {
	recti_t r = platform_win_rect(local->window);
	platform_key_save_bytes("WindowLocation", &r, sizeof(r));

	render_pipeline_shutdown();
	platform_win_destroy(local->window);

	local = {};
	sk_free(local);
}

///////////////////////////////////////////

void window_step_begin() {
	// Process events from the simulator window
	platform_evt_       evt  = {};
	platform_evt_data_t data = {};
	while (platform_win_next_event(local->window, &evt, &data)) {
		switch (evt) {
		case platform_evt_app_focus:    sk_set_app_focus (data.app_focus); break;
		case platform_evt_key_press:    input_key_inject_press  (data.press_release); window_physical_key_interact(); break;
		case platform_evt_key_release:  input_key_inject_release(data.press_release); window_physical_key_interact(); break;
		case platform_evt_character:    input_text_inject_char  (data.character);                                  break;
		case platform_evt_mouse_press:  if (sk_app_focus() == app_focus_active) input_key_inject_press  (data.press_release); break;
		case platform_evt_mouse_release:if (sk_app_focus() == app_focus_active) input_key_inject_release(data.press_release); break;
		case platform_evt_close:        sk_quit(quit_reason_system_close); break;
		case platform_evt_resize:       window_surface_resize(local->surface, data.resize.width, data.resize.height); break;
		case platform_evt_none: break;
		default: break;
		}
	}
	input_step();
}

///////////////////////////////////////////

void window_step_end() {
	input_step_late();

	matrix view = matrix_invert(render_get_cam_final());
	matrix proj = render_get_projection_matrix();
	render_pipeline_surface_set_clear      (local->surface, render_get_clear_color_ln());
	render_pipeline_surface_set_layer      (local->surface, render_get_filter());
	render_pipeline_surface_set_perspective(local->surface, &view, &proj, 1);

	render_pipeline_draw();

	skg_swapchain_t* swapchain = platform_win_get_swapchain(local->window);
	local->render_sys->profile_frame_duration = stm_since(local->render_sys->profile_frame_start);
	render_pipeline_surface_to_swapchain(local->surface, swapchain);
}

}