// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

#include "simulator.h"

#include "../_stereokit.h"
#include "../stereokit_ui.h" // for ui_has_keyboard_focus
#include "../device.h"
#include "../log.h"
#include "../libraries/stref.h"
#include "../libraries/sokol_time.h"
#include "../platforms/platform.h"
#include "../platforms/_platform.h"
#include "../asset_types/texture.h"
#include "../systems/system.h"
#include "../systems/input.h"
#include "../systems/input_keyboard.h"
#include "../systems/render.h"
#include "../systems/world.h"

namespace sk {

///////////////////////////////////////////

int32_t        sim_gaze_pointer;
vec3           sim_head_rot;
vec3           sim_head_pos;
pose_t         sim_bounds_pose;
bool           sim_mouse_look;

platform_win_t sim_window;
tex_t          sim_target;
system_t*      sim_render_sys;

const float    sim_move_speed = 1.4f; // average human walk speed, see: https://en.wikipedia.org/wiki/Preferred_walking_speed;
const vec2     sim_rot_speed  = { 10.f, 5.f }; // converting mouse pixel movement to rotation;

///////////////////////////////////////////

void sim_physical_key_interact();
void sim_target_resize        (tex_t* target, int32_t width, int32_t height);

///////////////////////////////////////////

bool simulator_init() {
	const sk_settings_t* settings = sk_get_settings_ref();

	device_data.has_hand_tracking = true;
	device_data.has_eye_gaze      = true;
	device_data.tracking          = device_tracking_6dof;
	device_data.display_blend     = display_blend_opaque;
	device_data.display_type      = display_type_flatscreen;
	device_data.name              = string_copy("Simulator");

	sim_head_rot     = { -21, 0.0001f, 0 };
	sim_head_pos     = { 0, 0.2f, 0.0f };
	sim_mouse_look   = false;
	sim_gaze_pointer = input_add_pointer(input_source_gaze | input_source_gaze_head);
	sim_window       = -1;
	sim_target       = {};
	sim_render_sys   = systems_find("FrameRender");

	quat initial_rot = quat_from_angles(0, sim_head_rot.y, 0);
	switch (sk_get_settings_ref()->origin) {
	case origin_mode_local: world_origin_mode = origin_mode_local; world_origin_offset = { sim_head_pos,                  initial_rot }; sim_bounds_pose = { 0,-1.5f,0, quat_inverse(initial_rot) }; break;
	case origin_mode_floor: world_origin_mode = origin_mode_local; world_origin_offset = { sim_head_pos - vec3{0,1.5f,0}, initial_rot }; sim_bounds_pose = { world_origin_offset.position, quat_inverse(world_origin_offset.orientation) }; break;
	case origin_mode_stage: world_origin_mode = origin_mode_local; world_origin_offset = { sim_head_pos - vec3{0,1.5f,0}, initial_rot }; sim_bounds_pose = { world_origin_offset.position, quat_inverse(world_origin_offset.orientation) }; break;
	}

	recti_t win_size = {};
	if (!platform_key_load_bytes("WindowLocation", &win_size, sizeof(win_size))) {
		win_size.x = settings->flatscreen_pos_x;
		win_size.y = settings->flatscreen_pos_y;
		win_size.w = settings->flatscreen_width;
		win_size.h = settings->flatscreen_height;
	}

	switch (platform_win_type()) {
	case platform_win_type_creatable: {
		sim_window = platform_win_make(settings->app_name, win_size, platform_surface_swapchain);
		if (sim_window == -1) {
			log_fail_reason(90, log_error, "Failed to create platform window.");
			return false;
		}
	} break;
	case platform_win_type_existing: {
		sim_window = platform_win_get_existing(platform_surface_swapchain);
	} break;
	}

	skg_swapchain_t* swapchain = platform_win_get_swapchain(sim_window);
	if (swapchain)
		sim_target_resize(&sim_target, swapchain->width, swapchain->height);

	return true;
}

///////////////////////////////////////////

void sim_target_resize(tex_t *target, int32_t width, int32_t height) {
	device_data.display_width  = width;
	device_data.display_height = height;

	if (*target == nullptr) {
		log_diagf("Creating target: %d<~BLK>x<~clr>%d", width, height);

		*target = tex_create(tex_type_rendertarget, tex_format_rgba32);
		tex_set_id       (*target, "sk/platform/swapchain");
		tex_set_color_arr(*target, width, height, nullptr, 1, nullptr, 8);

		tex_t zbuffer = tex_add_zbuffer(*target, render_preferred_depth_fmt());
		tex_set_id (zbuffer, "sk/platform/swapchain_zbuffer");
		tex_release(zbuffer);
		render_update_projection();
	}

	if (width == tex_get_width(*target) && height == tex_get_height(*target))
		return;

	log_diagf("Resizing target to: %d<~BLK>x<~clr>%d", width, height);

	tex_set_color_arr(*target, width, height, nullptr, 1, nullptr, 8);
	render_update_projection();
}

///////////////////////////////////////////

void simulator_shutdown() {
	recti_t r = platform_win_rect(sim_window);
	platform_key_save_bytes("WindowLocation", &r, sizeof(r));

	tex_release(sim_target);
	platform_win_destroy(sim_window);
	sim_window = -1;
}

///////////////////////////////////////////

void simulator_step_begin() {
	// Process events from the simulator window
	platform_evt_       evt  = {};
	platform_evt_data_t data = {};
	while (platform_win_next_event(sim_window, &evt, &data)) {
		switch (evt) {
		case platform_evt_app_focus:    sk_set_app_focus (data.app_focus); break;
		case platform_evt_key_press:    input_key_inject_press  (data.press_release); sim_physical_key_interact(); break;
		case platform_evt_key_release:  input_key_inject_release(data.press_release); sim_physical_key_interact(); break;
		case platform_evt_character:    input_text_inject_char  (data.character);                                  break;
		case platform_evt_mouse_press:  if (sk_app_focus() == app_focus_active) input_key_inject_press  (data.press_release); break;
		case platform_evt_mouse_release:if (sk_app_focus() == app_focus_active) input_key_inject_release(data.press_release); break;
		//case platform_evt_scroll:       if (sk_app_focus() == app_focus_active) win32_scroll += data.scroll;                  break;
		case platform_evt_close:        sk_quit(); break;
		case platform_evt_resize:       sim_target_resize(&sim_target, data.resize.width, data.resize.height); break;
		case platform_evt_none: break;
		default: break;
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

			platform_set_cursor(prev_pt);
			input_mouse_data.pos = prev_pt;

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
	input_eyes_track_state = button_make_state(input_eyes_track_state & button_state_active, sim_tracked);
	ray_t ray = {};
	if (sim_tracked && ray_from_mouse(input_mouse_data.pos, ray)) {
		input_eyes_pose_world.position    = ray.pos;
		input_eyes_pose_world.orientation = quat_lookat(vec3_zero, ray.dir);
		input_eyes_pose_local.position    = matrix_transform_pt(render_get_cam_final_inv(), ray.pos);
		input_eyes_pose_local.orientation = quat_lookat(vec3_zero, matrix_transform_dir(render_get_cam_final_inv(), ray.dir));
	}

	pointer_t *pointer_head = input_get_pointer(sim_gaze_pointer);
	pointer_head->tracked = button_state_active;
	pointer_head->ray.pos = input_eyes_pose_world.position;
	pointer_head->ray.dir = input_eyes_pose_world.orientation * vec3_forward;

	render_set_sim_origin(world_origin_offset);
	render_set_sim_head  (pose_t{ sim_head_pos, quat_from_angles(sim_head_rot.x, sim_head_rot.y, sim_head_rot.z) });
}

///////////////////////////////////////////

void simulator_step_end() {
	input_update_poses(true);

	skg_draw_begin();
	skg_swapchain_t* swapchain = platform_win_get_swapchain(sim_window);

	// If we have no swapchain (yet, it may arrive later), we won't want to do
	// normal rendering, so just check for screenshots and offscreen surfaces,
	// and call it a day.
	if (swapchain == nullptr) {
		skg_event_begin("Draw");
		render_check_viewpoints();
		render_check_screenshots();
		render_clear();
		skg_event_end();
		return;
	}

	skg_event_begin("Setup");
	{
		color128 col = render_get_clear_color_ln();
		skg_tex_target_bind(&sim_target->tex);
		skg_target_clear(true, &col.r);
	}
	skg_event_end();
	skg_event_begin("Draw");
	{
		matrix view = matrix_invert(render_get_cam_final());
		matrix proj = render_get_projection_matrix();
		render_draw_matrix(&view, &proj, 1, render_get_filter());
		render_clear();
	}
	skg_event_end();
	skg_event_begin("Present");
	{
		// This copies the color data over to the swapchain, and resolves any
		// multisampling on the primary target texture.
		skg_tex_copy_to_swapchain(&sim_target->tex, swapchain);

		sim_render_sys->profile_frame_duration = stm_since(sim_render_sys->profile_frame_start);
		skg_swapchain_present(swapchain);
	}
	skg_event_end();
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
	platform_keyboard_show(false, text_context_text);
}

}