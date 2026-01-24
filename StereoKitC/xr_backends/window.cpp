// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

#include "window.h"
#include "ska_input.h"

#include <sk_app.h>
#include <sk_renderer.h>

#include "../_stereokit.h"
#include "../device.h"
#include "../log.h"
#include "../sk_math.h"
#include "../sk_memory.h"
#include "../platforms/_platform.h"
#include "../asset_types/texture.h"
#include "../systems/world.h"
#include "../systems/input.h"
#include "../systems/input_keyboard.h"
#include "../systems/render.h"
#include "../systems/render_pipeline.h"
#include "../libraries/stref.h"
#include "../libraries/sokol_time.h"

///////////////////////////////////////////

using namespace sk;

struct window_backend_state_t {
	ska_window_t*       ska_win;
	skr_surface_t       skr_surface;
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

	// Load window position from persistent storage
	ska_rect_t win_size = {};
	size_t loaded_size = 0;
	if (!ska_kvpstore_load("WindowLocation", &win_size, sizeof(win_size), &loaded_size) || loaded_size != sizeof(win_size)) {
		win_size.x = settings->flatscreen_pos_x;
		win_size.y = settings->flatscreen_pos_y;
		win_size.w = settings->flatscreen_width;
		win_size.h = settings->flatscreen_height;
	}
	win_size.w = maxi(1, win_size.w);
	win_size.h = maxi(1, win_size.h);

	// Create window with sk_app
	local->ska_win = ska_window_create(
		settings->app_name,
		win_size.x, win_size.y,
		win_size.w, win_size.h,
		ska_window_resizable
	);
	if (local->ska_win == nullptr) {
		log_errf("Failed to create window: %s", ska_error_get() ? ska_error_get() : "unknown error");
		log_fail_reason(90, log_error, "Window creation failed");
		sk_free(local);
		local = nullptr;
		return false;
	}

	// Create Vulkan surface for the window
	VkSurfaceKHR vk_surface = VK_NULL_HANDLE;
	if (!ska_vk_create_surface(local->ska_win, skr_get_vk_instance(), &vk_surface)) {
		log_errf("Failed to create Vulkan surface: %s", ska_error_get() ? ska_error_get() : "unknown error");
		log_fail_reason(90, log_error, "Vulkan surface creation failed");
		ska_window_destroy(local->ska_win);
		sk_free(local);
		local = nullptr;
		return false;
	}

	// Create skr_surface_t from the Vulkan surface
	if (skr_surface_create(vk_surface, &local->skr_surface) != skr_err_success) {
		log_fail_reason(90, log_error, "Failed to create renderer surface");
		vkDestroySurfaceKHR(skr_get_vk_instance(), vk_surface, nullptr);
		ska_window_destroy(local->ska_win);
		sk_free(local);
		local = nullptr;
		return false;
	}

	// Use BGRA to match typical swapchain format
	local->surface = render_pipeline_surface_create(pipeline_render_strategy_sequential, tex_format_bgra32, render_preferred_depth_fmt(), 1, 1, 1);
	if (local->skr_surface.size.x > 0 && local->skr_surface.size.y > 0)
		window_surface_resize(local->surface, local->skr_surface.size.x, local->skr_surface.size.y);

	return true;
}
///////////////////////////////////////////

void window_physical_key_interact() {
	// On desktop, we want to hide soft keyboards on physical presses
	input_set_last_physical_keypress_time(time_totalf_unscaled());
	ska_virtual_keyboard_show(false, ska_text_input_type_text);
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
	// Save window position to persistent storage
	if (local->ska_win) {
		ska_rect_t r;
		ska_window_get_frame_position(local->ska_win, &r.x, &r.y);
		ska_window_get_frame_size    (local->ska_win, &r.w, &r.h);
		ska_kvpstore_save("WindowLocation", &r, sizeof(r));
	}

	render_pipeline_shutdown();

	// Destroy the renderer surface before the window
	skr_surface_destroy(&local->skr_surface);
	ska_window_destroy (local->ska_win);

	*local = {};
	sk_free(local);
}

///////////////////////////////////////////

void window_step_begin() {
	// Process events from sk_app
	ska_event_t evt;
	while (ska_event_poll(&evt)) {
		switch (evt.type) {
		// Backend-specific: physical key events hide soft keyboard
		case ska_event_key_down:
		case ska_event_key_up:
			ska_handle_event(&evt);
			window_physical_key_interact();
			break;
		// Backend-specific: window resize updates our surface
		case ska_event_window_resized:
			window_surface_resize(local->surface, maxi(1, evt.window.data1), maxi(1, evt.window.data2));
			skr_surface_resize(&local->skr_surface);
			break;
		// All other events use common handling
		default:
			ska_handle_event(&evt);
			break;
		}
	}
	input_step();

	// Begin the render frame early so that any graphics operations the
	// application performs during step are captured.
	render_pipeline_begin_frame();
}

///////////////////////////////////////////

void window_step_end() {
	input_step_late();

	matrix view = matrix_invert(render_get_cam_final());
	matrix proj = render_get_projection_matrix();
	render_pipeline_surface_set_clear      (local->surface, render_get_clear_color_ln());
	render_pipeline_surface_set_layer      (local->surface, render_get_filter());
	render_pipeline_surface_set_perspective(local->surface, &view, &proj, 1);

	// Acquire swapchain image before rendering - it becomes the MSAA resolve target
	render_pipeline_surface_acquire_swapchain(local->surface, &local->skr_surface);

	render_pipeline_draw();

	render_pipeline_surface_present_swapchain(local->surface, &local->skr_surface);
}

}