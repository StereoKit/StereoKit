// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2025 Nick Klingensmith
// Copyright (c) 2023-2025 Qualcomm Technologies, Inc.

#include "simulator.h"
#include "ska_input.h"

#include <sk_app.h>
#include <sk_renderer.h>

#include "../_stereokit.h"
#include "../stereokit_ui.h" // for ui_has_keyboard_focus
#include "../device.h"
#include "../log.h"
#include "../sk_math.h"
#include "../libraries/stref.h"
#include "../libraries/sokol_time.h"
#include "../platforms/platform.h"
#include "../platforms/_platform.h"
#include "../systems/input.h"
#include "../systems/input_keyboard.h"
#include "../systems/render.h"
#include "../systems/render_pipeline.h"
#include "../systems/world.h"
#include "../asset_types/anchor.h"

namespace sk {

///////////////////////////////////////////

vec3           sim_head_rot;
vec3           sim_head_pos;
pose_t         sim_bounds_pose;
bool           sim_mouse_look;

ska_window_t*       ska_win;
skr_surface_t       sim_skr_surface;
pipeline_surface_id sim_surface;

const float    sim_move_speed = 1.4f; // average human walk speed, see: https://en.wikipedia.org/wiki/Preferred_walking_speed;
const vec2     sim_rot_speed  = { 10.f, 5.f }; // converting mouse pixel movement to rotation;

///////////////////////////////////////////

void sim_physical_key_interact();
void sim_surface_resize        (pipeline_surface_id surface, int32_t width, int32_t height);

///////////////////////////////////////////

bool simulator_init() {
	const sk_settings_t* settings = sk_get_settings_ref();

	device_data.has_hand_tracking = true;
	device_data.has_eye_gaze      = true;
	device_data.tracking          = device_tracking_6dof;
	device_data.display_blend     = display_blend_opaque;
	device_data.display_type      = display_type_flatscreen;
	device_data.name              = string_copy("Simulator");
	device_data.runtime           = string_copy("Simulator");

	sim_head_rot     = { -21, 0.0001f, 0 };
	sim_head_pos     = { 0, 0.2f, 0.0f };
	sim_mouse_look   = false;
	ska_win          = nullptr;
	sim_skr_surface  = {};

	quat initial_rot = quat_from_angles(0, sim_head_rot.y, 0);
	switch (sk_get_settings_ref()->origin) {
	case origin_mode_local: world_origin_mode = origin_mode_local; world_origin_offset = { sim_head_pos,                  initial_rot }; sim_bounds_pose = { 0,-1.5f,0, quat_inverse(initial_rot) }; break;
	case origin_mode_floor: world_origin_mode = origin_mode_local; world_origin_offset = { sim_head_pos - vec3{0,1.5f,0}, initial_rot }; sim_bounds_pose = { world_origin_offset.position, quat_inverse(world_origin_offset.orientation) }; break;
	case origin_mode_stage: world_origin_mode = origin_mode_local; world_origin_offset = { sim_head_pos - vec3{0,1.5f,0}, initial_rot }; sim_bounds_pose = { world_origin_offset.position, quat_inverse(world_origin_offset.orientation) }; break;
	}

	// Load window position from persistent storage
	ska_rect_t win_size = {};
	size_t loaded_size = 0;
	if (sk_use_manual_pos() || !ska_kvpstore_load("WindowLocation", &win_size, sizeof(win_size), &loaded_size) || loaded_size != sizeof(win_size)) {
		win_size.x = settings->flatscreen_pos_x;
		win_size.y = settings->flatscreen_pos_y;
		win_size.w = settings->flatscreen_width;
		win_size.h = settings->flatscreen_height;
	}
	win_size.w = maxi(1, win_size.w);
	win_size.h = maxi(1, win_size.h);

	// Create window with sk_app
	ska_win = ska_window_create(
		settings->app_name,
		win_size.x, win_size.y,
		win_size.w, win_size.h,
		ska_window_resizable
	);
	if (ska_win == nullptr) {
		log_errf("Failed to create window: %s", ska_error_get() ? ska_error_get() : "unknown error");
		log_fail_reason(90, log_error, "Failed to create window");
		return false;
	}

	// Create Vulkan surface for the window
	VkSurfaceKHR vk_surface = VK_NULL_HANDLE;
	if (!ska_vk_create_surface(ska_win, skr_get_vk_instance(), &vk_surface)) {
		log_errf("Failed to create Vulkan surface: %s", ska_error_get() ? ska_error_get() : "unknown error");
		log_fail_reason(90, log_error, "Failed to create Vulkan surface");
		ska_window_destroy(ska_win);
		ska_win = nullptr;
		return false;
	}

	// Create skr_surface_t from the Vulkan surface
	if (skr_surface_create(vk_surface, &sim_skr_surface) != skr_err_success) {
		log_fail_reason(90, log_error, "Failed to create renderer surface");
		vkDestroySurfaceKHR(skr_get_vk_instance(), vk_surface, nullptr);
		ska_window_destroy(ska_win);
		ska_win = nullptr;
		return false;
	}

	// Use BGRA to match typical swapchain format
	sim_surface = render_pipeline_surface_create(pipeline_render_strategy_sequential, tex_format_bgra32, render_preferred_depth_fmt(), 1, 1, 1);
	if (sim_skr_surface.size.x > 0 && sim_skr_surface.size.y > 0)
		sim_surface_resize(sim_surface, sim_skr_surface.size.x, sim_skr_surface.size.y);

	anchors_init();
	return true;
}

///////////////////////////////////////////

void sim_surface_resize(pipeline_surface_id surface, int32_t width, int32_t height) {
	device_data.display_width  = width;
	device_data.display_height = height;

	if (render_pipeline_surface_resize(surface, width, height, 8))
		render_update_projection();
}

///////////////////////////////////////////

void simulator_shutdown() {
	// Save window position to persistent storage
	if (ska_win) {
		ska_rect_t r;
		ska_window_get_frame_position(ska_win, &r.x, &r.y);
		ska_window_get_frame_size(ska_win, &r.w, &r.h);
		ska_kvpstore_save("WindowLocation", &r, sizeof(r));
	}

	render_pipeline_shutdown();
	sim_surface = -1;

	// Destroy the renderer surface before the window
	skr_surface_destroy(&sim_skr_surface);
	sim_skr_surface = {};

	ska_window_destroy(ska_win);
	ska_win = nullptr;
	anchors_shutdown(NULL);
}

///////////////////////////////////////////

void simulator_step_begin() {
	// Process events from sk_app
	ska_event_t evt;
	while (ska_event_poll(&evt)) {
		switch (evt.type) {
		// Backend-specific: physical key events hide soft keyboard
		case ska_event_key_down:
		case ska_event_key_up:
			ska_handle_event(&evt);
			sim_physical_key_interact();
			break;
		// Backend-specific: window resize updates our surface
		case ska_event_window_resized:
			sim_surface_resize(sim_surface, maxi(1, evt.window.data1), maxi(1, evt.window.data2));
			skr_surface_resize(&sim_skr_surface);
			break;
		// All other events use common handling
		default:
			ska_handle_event(&evt);
			break;
		}
	}

	input_step();

	if (simulator_is_simulating_movement()) {
		// Get key based movement
		vec3 movement = {};
		if (!ui_has_keyboard_focus()) {
			// Don't do keyboard movement if the UI is using the keyboard!
			if (input_key(key_w) & button_state_active) movement += vec3_forward;
			if (input_key(key_s) & button_state_active) movement -= vec3_forward;
			if (input_key(key_d) & button_state_active) movement += vec3_right;
			if (input_key(key_a) & button_state_active) movement -= vec3_right;
			if (input_key(key_e) & button_state_active) movement += vec3_up;
			if (input_key(key_q) & button_state_active) movement -= vec3_up;
		}
		if (vec3_magnitude_sq( movement ) != 0)
			movement = vec3_normalize(movement);

		// head rotation
		quat orientation;
		if (input_key(key_mouse_right) & button_state_just_active) {
			sim_mouse_look = true;
		}
		if (sim_mouse_look) {
			const mouse_t *mouse = input_mouse();
			sim_head_rot.y -= mouse->pos_change.x * sim_rot_speed.x * time_stepf_unscaled();
			sim_head_rot.x -= mouse->pos_change.y * sim_rot_speed.y * time_stepf_unscaled();
			sim_head_rot.x = fmaxf(-89.9f, fminf(sim_head_rot.x, 89.9f));
			orientation = quat_from_angles(sim_head_rot.x, sim_head_rot.y, sim_head_rot.z);

			vec2 prev_pt = mouse->pos - mouse->pos_change;

			ska_mouse_warp(ska_win, (int32_t)prev_pt.x, (int32_t)prev_pt.y);
			input_mouse_override_pos(prev_pt);

		} else {
			orientation = quat_from_angles(sim_head_rot.x, sim_head_rot.y, sim_head_rot.z);
		}
		// Apply movement to the camera
		sim_head_pos += orientation * movement * time_stepf_unscaled() * sim_move_speed;
	} else {
		sim_mouse_look = false;
	}
	if (input_key(key_mouse_right) & button_state_just_inactive) {
		sim_mouse_look = false;
	}

	bool sim_tracked = (input_key(key_alt) & button_state_active) > 0 ? true : false;
	input_eyes_tracked_set(button_make_state(input_eyes_tracked() & button_state_active, sim_tracked));
	ray_t ray = {};
	if (sim_tracked && ray_from_mouse(input_mouse()->pos, ray)) {
		input_pose_inject(input_pose_eyes, pose_t{
			matrix_transform_pt(render_get_cam_final_inv(), ray.pos),
			quat_lookat(vec3_zero, matrix_transform_dir(render_get_cam_final_inv(), ray.dir)) }, 
			track_state_known, track_state_known);
	}

	render_set_sim_origin(world_origin_offset);
	render_set_sim_head  (pose_t{ sim_head_pos, quat_from_angles(sim_head_rot.x, sim_head_rot.y, sim_head_rot.z) });
	anchors_step_begin(NULL);

	// Begin the render frame early so that any graphics operations the
	// application performs during step are captured.
	render_pipeline_begin_frame();
}

///////////////////////////////////////////

void simulator_step_end() {
	anchors_step_end(NULL);
	input_step_late();

	matrix view = matrix_invert(render_get_cam_final());
	matrix proj = render_get_projection_matrix();
	render_pipeline_surface_set_clear      (sim_surface, render_get_clear_color_ln());
	render_pipeline_surface_set_layer      (sim_surface, render_get_filter());
	render_pipeline_surface_set_perspective(sim_surface, &view, &proj, 1);

	// Acquire swapchain image before rendering - it becomes the MSAA resolve target
	render_pipeline_surface_acquire_swapchain(sim_surface, &sim_skr_surface);

	render_pipeline_draw();

	render_pipeline_surface_present_swapchain(sim_surface, &sim_skr_surface);
}

///////////////////////////////////////////

void simulator_set_origin_offset(pose_t offset) {
	render_set_sim_origin(offset);
}

///////////////////////////////////////////

bool simulator_is_simulating_movement() {
	return backend_xr_get_type() == backend_xr_type_simulator &&
		(input_key(key_caps_lock) & button_state_active ||
		 input_key(key_shift)     & button_state_active);
}

///////////////////////////////////////////

vec2 simulator_bounds_size() {
	return vec2{ 3,2 };
}

///////////////////////////////////////////

pose_t simulator_bounds_pose() {
	return matrix_transform_pose(render_get_cam_root(), pose_t{
		sim_bounds_pose.position - world_origin_offset.position,
		quat_inverse(sim_bounds_pose.orientation * world_origin_offset.orientation)
	});
}

///////////////////////////////////////////

void sim_physical_key_interact() {
	// On desktop, we want to hide soft keyboards on physical presses
	input_set_last_physical_keypress_time(time_totalf_unscaled());
	ska_virtual_keyboard_show(false, ska_text_input_type_text);
}

}