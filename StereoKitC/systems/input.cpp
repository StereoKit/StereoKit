/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2026 Nick Klingensmith
 * Copyright (c) 2024-2026 Qualcomm Technologies, Inc.
 */

#include "../stereokit.h"
#include "input.h"
#include "input_keyboard.h"
#include "input_render.h"
#include "../hands/input_hand.h"
#include "../libraries/array.h"
#include "../libraries/ferr_thread.h"
#include "../libraries/profiler.h"
#include "../xr_backends/openxr.h"
#include "../xr_backends/openxr_input.h"
#include "../systems/render.h"

#include <sk_app.h>

namespace sk {

///////////////////////////////////////////

struct input_event_t {
	input_source_ source;
	button_state_ event;
	void (*event_callback)(input_source_ source, button_state_ evt, const pointer_t &pointer);
};

struct pose_info_t {
	pose_t       pose;
	track_state_ pos_tracked;
	track_state_ rot_tracked;
};

struct evt_pose_t {
	input_pose_   type;
	pose_info_t   value;
};

struct evt_float_t {
	input_float_  type;
	float         value;
};

struct evt_button_t {
	input_button_ type;
	bool          value;
};

struct evt_xy_t {
	input_xy_     type;
	vec2          value;
};

struct input_state_t {
	mouse_t               mouse_data;
	mouse_mode_           mouse_mode;        // What the app asked for
	mouse_mode_           mouse_mode_active; // What the cursor is actually doing
	bool                  mouse_captured;    // Relative mode asked for, and granted
	vec2                  mouse_lock_pos;    // The stationary position relative mode reports
	ska_window_t*         mouse_window;
	controller_t          controllers[2];
	bool                  controller_hand[2];
	button_state_         controller_menubtn;
	pose_t                palm_offset[2];

	button_state_         eyes_track_state;
	pose_t                eyes_pose_local;

	array_t<pose_info_t>  curr_poses;
	array_t<button_state_>curr_buttons;
	array_t<float>        curr_floats;
	array_t<vec2>         curr_xys;

	ft_mutex_t            mtx_poses;
	ft_mutex_t            mtx_buttons;
	ft_mutex_t            mtx_floats;
	ft_mutex_t            mtx_xys;

	array_t<evt_button_t> prev_evt_buttons;

	array_t<evt_pose_t>   evt_poses;
	array_t<evt_button_t> evt_buttons;
	array_t<evt_float_t>  evt_floats;
	array_t<evt_xy_t>     evt_xys;

	bool                  initialized;
};
static input_state_t local = {};

// Scroll accumulator for sk_app event-driven scroll
static float ska_scroll_accumulator = 0.0f;

// TODO: these should be moved to local state
pose_t input_head_pose_local;

///////////////////////////////////////////

void input_mouse_update();

///////////////////////////////////////////

bool input_init() {
	profiler_zone();

	// Preserve values that may have been set before init, such as palm
	// offsets from OpenXR interaction profile events, or the window the
	// backend handed us - Platform initializes before Input.
	pose_t        palm_offset[2]     = { local.palm_offset[0],     local.palm_offset[1]     };
	bool          controller_hand[2] = { local.controller_hand[0], local.controller_hand[1] };
	ska_window_t* mouse_window       = local.mouse_window;

	local = {};
	input_head_pose_local    = pose_identity;
	local.eyes_pose_local    = pose_identity;
	local.palm_offset[0]     = palm_offset[0];
	local.palm_offset[1]     = palm_offset[1];
	local.controller_hand[0] = controller_hand[0];
	local.controller_hand[1] = controller_hand[1];
	local.mouse_window       = mouse_window;

	local.mtx_poses   = ft_mutex_create();
	local.mtx_floats  = ft_mutex_create();
	local.mtx_buttons = ft_mutex_create();
	local.mtx_xys     = ft_mutex_create();

	input_keyboard_initialize();
	input_hand_init();
	input_mouse_update();
	input_render_init();

	local.initialized = true;
	return true;
}

///////////////////////////////////////////

void input_shutdown() {
	if (local.mouse_mode_active != mouse_mode_normal) {
		// Capture is a pointer grab, not just a hidden cursor, so it outlives
		// the window if nobody releases it.
		ska_mouse_set_relative_mode(false);
		ska_cursor_show(true);
	}

	ft_mutex_destroy(&local.mtx_poses);
	ft_mutex_destroy(&local.mtx_floats);
	ft_mutex_destroy(&local.mtx_buttons);
	ft_mutex_destroy(&local.mtx_xys);
	local.prev_evt_buttons.free();
	local.evt_buttons     .free();
	local.evt_floats      .free();
	local.evt_poses       .free();
	local.evt_xys         .free();
	local.curr_buttons    .free();
	local.curr_floats     .free();
	local.curr_poses      .free();
	local.curr_xys        .free();

	input_render_shutdown();
	input_keyboard_shutdown();
	input_hand_shutdown();
	local = {};
}

///////////////////////////////////////////

void input_pose_info_update() {
	// Clear tracking state first, since we don't know if all poses will be
	// updated.
	for (int32_t i = 0; i < local.curr_poses.count; i++) {
		local.curr_poses[i].pos_tracked = track_state_lost;
		local.curr_poses[i].rot_tracked = track_state_lost;
	}
	// Now update all poses we have events for. Thread-safe.
	ft_mutex_lock(local.mtx_poses);
	for (int32_t i = 0; i < local.evt_poses.count; i++) {
		evt_pose_t e = local.evt_poses[i];
		if (e.type >= local.curr_poses.count) {
			int count = local.curr_poses.count;
			local.curr_poses.add_empties((e.type - local.curr_poses.count) + 1);
			// Set empty quats to identity
			for (int32_t p = count; p < local.curr_poses.count; p++) {
				local.curr_poses[p].pose.orientation = quat_identity;
			}
		}
		local.curr_poses[e.type] = e.value;
	}
	local.evt_poses.clear();
	ft_mutex_unlock(local.mtx_poses);

	// Handle palm pose, which may not be available from the system. If it's
	// not, we want to generate it from the grip pose!
	input_pose_ poses_palm[2]{ input_pose_l_palm, input_pose_r_palm };
	input_pose_ poses_grip[2]{ input_pose_l_grip, input_pose_r_grip };
	for (int32_t i = 0; i < 2; i++) {
		// Check if the palm is _not_ tracked, but the grip is.
		pose_state_ palm_state = input_pose_state(poses_palm[i]);
		pose_state_ grip_state = input_pose_state(poses_grip[i]);
		if (palm_state == pose_state_lost && grip_state != pose_state_lost) {

			// Make sure we have room in our input array for the pose
			if (poses_palm[i] >= local.curr_poses.count)
				local.curr_poses.add_empties((poses_palm[i] - local.curr_poses.count) + 1);

			// Set up the new palm pose, based on the grip
			pose_t      grip_pose = input_pose_get_local(poses_grip[i]);
			pose_info_t new_pose  = {
				grip_pose.position + grip_pose.orientation * local.palm_offset[i].position,
				local.palm_offset[i].orientation * grip_pose.orientation };
			local.curr_poses[poses_palm[i]]             = new_pose;
			local.curr_poses[poses_palm[i]].pos_tracked = (grip_state & pose_state_pos_known) > 0 ? track_state_known : (grip_state & pose_state_pos_inferred) > 0 ? track_state_inferred : track_state_lost;
			local.curr_poses[poses_palm[i]].rot_tracked = (grip_state & pose_state_rot_known) > 0 ? track_state_known : (grip_state & pose_state_rot_inferred) > 0 ? track_state_inferred : track_state_lost;
		}
	}
}

///////////////////////////////////////////

void input_buttons_update() {
	///////////////////////////////////////////
	// Update buttons
	///////////////////////////////////////////

	// Clear last frame's just-active/inactive flags, prev_evt_buttons will
	// contain last frame's button events.
	for (int32_t i = 0; i < local.prev_evt_buttons.count; i++) {
		evt_button_t e = local.prev_evt_buttons[i];
		if (e.value) local.curr_buttons[e.type] &= ~button_state_just_active;
		else         local.curr_buttons[e.type] &= ~button_state_just_inactive;
	}
	local.prev_evt_buttons.clear();

	// Copy new events, thread-safe
	ft_mutex_lock(local.mtx_buttons);
	local.prev_evt_buttons.add_range(local.evt_buttons.data, local.evt_buttons.count);
	local.evt_buttons.clear();
	ft_mutex_unlock(local.mtx_buttons);

	// Update our button states based on button events
	for (int32_t i = 0; i < local.prev_evt_buttons.count; i++) {
		evt_button_t e = local.prev_evt_buttons[i];
		// Make sure we have space allocated for this button
		if (e.type >= local.curr_buttons.count)
			local.curr_buttons.add_empties((e.type - local.curr_buttons.count) + 1);
		local.curr_buttons[e.type] = button_make_state(local.curr_buttons[e.type] & button_state_active, e.value);
	}

	///////////////////////////////////////////
	// Update floats
	///////////////////////////////////////////

	ft_mutex_lock(local.mtx_floats);
	for (int32_t i = 0; i < local.evt_floats.count; i++) {
		evt_float_t e = local.evt_floats[i];
		if (e.type >= local.curr_floats.count)
			local.curr_floats.add_empties((e.type - local.curr_floats.count) + 1);
		local.curr_floats[e.type] = e.value;
	}
	local.evt_floats.clear();
	ft_mutex_unlock(local.mtx_floats);

	///////////////////////////////////////////
	// Update XYs
	///////////////////////////////////////////

	ft_mutex_lock(local.mtx_xys);
	for (int32_t i = 0; i < local.evt_xys.count; i++) {
		evt_xy_t e = local.evt_xys[i];
		if (e.type >= local.curr_xys.count)
			local.curr_xys.add_empties((e.type - local.curr_xys.count) + 1);
		local.curr_xys[e.type] = e.value;
	}
	local.evt_xys.clear();
	ft_mutex_unlock(local.mtx_xys);
}

///////////////////////////////////////////

void input_step() {
	profiler_zone();

	///////////////////////////////////////////
	// Update input sources
	///////////////////////////////////////////

	input_pose_info_update();
	input_buttons_update  ();
	input_mouse_update    ();
	input_keyboard_update ();

	///////////////////////////////////////////
	// Make controllers from our inputs
	///////////////////////////////////////////

	// Left
	local.controllers[handed_left].aim          = input_pose  (input_pose_l_aim);
	local.controllers[handed_left].palm         = input_pose  (input_pose_l_palm);
	local.controllers[handed_left].pose         = input_pose  (input_pose_l_grip);
	local.controllers[handed_left].grip         = input_float (input_float_l_grip);
	local.controllers[handed_left].trigger      = input_float (input_float_l_trigger);
	local.controllers[handed_left].stick_click  = input_button(input_button_l_stick);
	local.controllers[handed_left].x1           = input_button(input_button_l_x1);
	local.controllers[handed_left].x2           = input_button(input_button_l_x2);
	local.controllers[handed_left].stick        = input_xy    (input_xy_l_stick);

	pose_state_ l_grip_state = input_pose_state(input_pose_l_grip);
	local.controllers[handed_left].tracked_pos  = (l_grip_state & pose_state_pos_known) > 0 ? track_state_known : (l_grip_state & pose_state_pos_inferred) > 0 ? track_state_inferred : track_state_lost;
	local.controllers[handed_left].tracked_rot  = (l_grip_state & pose_state_rot_known) > 0 ? track_state_known : (l_grip_state & pose_state_rot_inferred) > 0 ? track_state_inferred : track_state_lost;
	local.controllers[handed_left].tracked      = button_make_state(
		(local.controllers[handed_left].tracked & button_state_active) > 0,
		local.controllers[handed_left].tracked_pos != track_state_lost ||
		local.controllers[handed_left].tracked_rot != track_state_lost);

	// Right
	local.controllers[handed_right].aim         = input_pose  (input_pose_r_aim);
	local.controllers[handed_right].palm        = input_pose  (input_pose_r_palm);
	local.controllers[handed_right].pose        = input_pose  (input_pose_r_grip);
	local.controllers[handed_right].grip        = input_float (input_float_r_grip);
	local.controllers[handed_right].trigger     = input_float (input_float_r_trigger);
	local.controllers[handed_right].stick_click = input_button(input_button_r_stick);
	local.controllers[handed_right].x1          = input_button(input_button_r_x1);
	local.controllers[handed_right].x2          = input_button(input_button_r_x2);
	local.controllers[handed_right].stick       = input_xy    (input_xy_r_stick);

	pose_state_ r_grip_state = input_pose_state(input_pose_r_grip);
	local.controllers[handed_right].tracked_pos = (r_grip_state & pose_state_pos_known) > 0 ? track_state_known : (r_grip_state & pose_state_pos_inferred) > 0 ? track_state_inferred : track_state_lost;
	local.controllers[handed_right].tracked_rot = (r_grip_state & pose_state_rot_known) > 0 ? track_state_known : (r_grip_state & pose_state_rot_inferred) > 0 ? track_state_inferred : track_state_lost;
	local.controllers[handed_right].tracked     = button_make_state(
		(local.controllers[handed_right].tracked & button_state_active) > 0,
		local.controllers[handed_right].tracked_pos != track_state_lost ||
		local.controllers[handed_right].tracked_rot != track_state_lost);

	// Both
	local.controller_menubtn = button_make_state(
		(local.controller_menubtn & button_state_active) != 0,
		(input_button(input_button_l_menu) & button_state_active) != 0 ||
		(input_button(input_button_r_menu) & button_state_active) != 0);

	///////////////////////////////////////////
	// Make eyes from our inputs
	///////////////////////////////////////////

	if (device_has_eye_gaze()) {
		pose_state_ state = input_pose_state(input_pose_eyes);
		local.eyes_pose_local  = input_pose_get_local(input_pose_eyes);
		local.eyes_track_state = button_make_state(
			(local.eyes_track_state & button_state_active) != 0,
			state != pose_state_lost);
	}

	///////////////////////////////////////////
	// Update more input systems
	///////////////////////////////////////////

	// Hands may depend on controllers
	input_hand_update();

	// Rendering depends on inputs
	input_render_step();
}

///////////////////////////////////////////

void input_step_late() {
	profiler_zone();

	input_update_poses();
	input_pose_info_update();
	input_render_step_late();
}

///////////////////////////////////////////

void input_subscribe(input_source_ source, button_state_ input_event, void (*input_event_callback)(input_source_ source, button_state_ input_event, const pointer_t &in_pointer)) {
	log_warnf("Input events are obsolete");

}

///////////////////////////////////////////

void input_unsubscribe(input_source_ source, button_state_ input_event, void (*input_event_callback)(input_source_ source, button_state_ input_event, const pointer_t &in_pointer)) {
	log_warnf("Input events are obsolete");
}

///////////////////////////////////////////

void input_fire_event(input_source_ source, button_state_ input_event, const pointer_t &pointer) {
	log_warnf("Input events are obsolete");
}

///////////////////////////////////////////

void input_update_poses() {
#if defined(SK_XR_OPENXR)
	if (backend_xr_get_type() == backend_xr_type_openxr)
		oxri_update_poses();
#endif
	input_hand_update_poses();
}

///////////////////////////////////////////

const mouse_t *input_mouse() {
	return &local.mouse_data;
}

///////////////////////////////////////////

button_state_ input_key(key_ key) {
	return input_keyboard_get(key);
}

///////////////////////////////////////////

pose_t input_head() {
	return render_cam_final_transform(input_head_pose_local);
}

///////////////////////////////////////////

pose_t input_eyes() {
	return render_cam_final_transform(local.eyes_pose_local);
}

///////////////////////////////////////////

button_state_ input_eyes_tracked() {
	return local.eyes_track_state;
}

///////////////////////////////////////////

void input_eyes_tracked_set(button_state_ state) {
	local.eyes_track_state = state;
}

///////////////////////////////////////////

const controller_t* input_controller(handed_ hand) {
	return &local.controllers[hand];
}

///////////////////////////////////////////

controller_t* input_controller_ref(handed_ handed) {
	return &local.controllers[handed];
}

///////////////////////////////////////////

button_state_ input_controller_menu() {
	return local.controller_menubtn;
}

///////////////////////////////////////////

void input_controller_menu_set(button_state_ state) {
	local.controller_menubtn = state;
}

///////////////////////////////////////////

bool input_controller_key(handed_ hand, controller_key_ key, float *out_amount) {
	*out_amount = 0;
	switch (key) {
	case controller_key_trigger: if (local.controllers[hand].trigger > 0.1f) { *out_amount = local.controllers[hand].trigger; return true; } else { return false; }
	case controller_key_grip:    if (local.controllers[hand].grip    > 0.1f) { *out_amount = local.controllers[hand].grip;    return true; } else { return false; }
	case controller_key_menu:  return (local.controller_menubtn & button_state_active) > 0;
	case controller_key_stick: return (local.controllers[hand].stick_click & button_state_active) > 0;
	case controller_key_x1:    return (local.controllers[hand].x1 & button_state_active) > 0;
	case controller_key_x2:    return (local.controllers[hand].x2 & button_state_active) > 0;
	default: return false;
	}
}

///////////////////////////////////////////

bool input_controller_is_hand(handed_ hand) {
	return local.controller_hand[hand];
}

///////////////////////////////////////////

void input_controller_set_hand(handed_ hand, bool is_hand) {
	local.controller_hand[hand] = is_hand;
}

///////////////////////////////////////////

pose_t input_controller_detached(handed_ hand) {
	input_pose_ detached_pose = hand == handed_left ? input_pose_l_detached : input_pose_r_detached;
	return input_pose(detached_pose);
}

///////////////////////////////////////////

// sk_app reports the mouse in screen coordinates while StereoKit works in render
// pixels, and on a scaled display those differ by the drawable ratio.
static float input_mouse_pixel_scale() {
	if (local.mouse_window == nullptr) return 1.0f;

	int32_t content_w = 0, content_h = 0, drawable_w = 0, drawable_h = 0;
	ska_window_get_content_size (local.mouse_window, &content_w, &content_h);
	ska_window_get_drawable_size(local.mouse_window, &drawable_w, &drawable_h);
	return content_w > 0 ? (float)drawable_w / (float)content_w : 1.0f;
}

void input_mouse_update() {
	// Get mouse position from sk_app
	int32_t  mouse_x = 0, mouse_y = 0;
	uint32_t button_state = ska_mouse_get_state(&mouse_x, &mouse_y);
	float    pixel_scale  = input_mouse_pixel_scale();
	vec2     mouse_pos    = { mouse_x * pixel_scale, mouse_y * pixel_scale };

	// Mouse is available if we have focus (button_state is non-zero or we have position data)
	local.mouse_data.available = sk_app_focus() == app_focus_active;

	// Mouse scroll - use accumulated scroll from sk_app events
	if (sk_app_focus() == app_focus_active) {
		local.mouse_data.scroll_change = ska_scroll_accumulator - local.mouse_data.scroll;
		local.mouse_data.scroll        = ska_scroll_accumulator;
	}

	// Losing focus always restores the cursor, so alt-tabbing out of a window
	// that captured the mouse doesn't strand the user without one. Backends
	// with no window of their own just remember the mode.
	mouse_mode_ mode = local.mouse_data.available && local.mouse_window != nullptr
		? local.mouse_mode
		: mouse_mode_normal;
	if (mode != local.mouse_mode_active) {
		// Relative mode hides the cursor and delivers unaccelerated deltas with
		// the pointer pinned. Warping back by hand cannot work on Wayland, which
		// has no pointer warp at all.
		// Not every platform can capture the pointer; where it can't, position
		// deltas stand in for the motion capture would have reported.
		bool want_relative      = mode == mouse_mode_relative;
		bool captured           = ska_mouse_set_relative_mode(want_relative);
		local.mouse_captured    = want_relative && captured;
		ska_cursor_show(mode == mouse_mode_normal);
		local.mouse_lock_pos    = mouse_pos;
		local.mouse_mode_active = mode;
	}

	// Mouse position and on-screen
	if (local.mouse_data.available) {
		if (local.mouse_captured) {
			// The pointer does not move in relative mode, so accumulated motion
			// is the only source, and the app sees a stationary pos.
			int32_t rel_x = 0, rel_y = 0;
			ska_mouse_get_delta(&rel_x, &rel_y);
			// Deltas are device units, not pixels, so they deliberately do not get
			// the display scale: the same hand movement should turn the view the
			// same amount on any monitor.
			local.mouse_data.pos_change = { (float)rel_x, (float)rel_y };
			local.mouse_data.pos        = local.mouse_lock_pos;
		} else {
			local.mouse_data.pos_change = mouse_pos - local.mouse_data.pos;
			local.mouse_data.pos        = mouse_pos;
		}
	}
}

///////////////////////////////////////////

void input_mouse_set_window(ska_window_t* window) {
	// Restore now, while the window that captured the mouse is still around.
	if (window == nullptr && local.mouse_mode_active != mouse_mode_normal) {
		ska_mouse_set_relative_mode(false);
		ska_cursor_show(true);
		local.mouse_captured    = false;
		local.mouse_mode_active = mouse_mode_normal;
	}
	local.mouse_window = window;
}

///////////////////////////////////////////

void input_mouse_mode_set(mouse_mode_ mode) {
	local.mouse_mode = mode;
}

///////////////////////////////////////////

mouse_mode_ input_mouse_mode_get(void) {
	return local.mouse_mode;
}

///////////////////////////////////////////

void input_pose_inject  (input_pose_   pose_type,   pose_t pose, track_state_ pos_tracked, track_state_ rot_tracked) { ft_mutex_lock(local.mtx_poses); local.evt_poses.add({ pose_type, {pose, pos_tracked, rot_tracked} }); ft_mutex_unlock(local.mtx_poses); }
void input_float_inject (input_float_  float_type,  float value) { ft_mutex_lock(local.mtx_floats ); local.evt_floats .add({ float_type,  value }); ft_mutex_unlock(local.mtx_floats ); }
void input_button_inject(input_button_ button_type, bool  value) { ft_mutex_lock(local.mtx_buttons); local.evt_buttons.add({ button_type, value }); ft_mutex_unlock(local.mtx_buttons); }
void input_xy_inject    (input_xy_     xy_type,     vec2  value) { ft_mutex_lock(local.mtx_xys    ); local.evt_xys    .add({ xy_type,     value }); ft_mutex_unlock(local.mtx_xys    ); }
void input_scroll_inject(float scroll_delta)                     { ska_scroll_accumulator += scroll_delta; }

///////////////////////////////////////////

input_haptic_caps_ input_haptic_caps(input_haptic_ haptic_type) {
#if defined(SK_XR_OPENXR)
	if (backend_xr_get_type() == backend_xr_type_openxr)
		return oxri_haptic_caps(haptic_type);
#endif
	return input_haptic_caps_none;
}

float input_haptic_preferred_rate(input_haptic_ haptic_type) {
#if defined(SK_XR_OPENXR)
	if (backend_xr_get_type() == backend_xr_type_openxr)
		return oxri_haptic_preferred_rate(haptic_type);
#endif
	return 0;
}

void input_haptic_pulse(input_haptic_ haptic_type, float frequency, float amplitude, float duration_seconds) {
#if defined(SK_XR_OPENXR)
	if (backend_xr_get_type() == backend_xr_type_openxr)
		oxri_haptic_pulse(haptic_type, frequency, amplitude, (double)duration_seconds);
#endif
}

void input_haptic_waveform(input_haptic_ haptic_type, const float* in_arr_samples, int32_t sample_count, float sample_rate_hz, bool32_t append, int32_t* out_prev_samples_consumed) {
	if (out_prev_samples_consumed) *out_prev_samples_consumed = 0;
#if defined(SK_XR_OPENXR)
	if (backend_xr_get_type() == backend_xr_type_openxr)
		oxri_haptic_waveform(haptic_type, in_arr_samples, sample_count, sample_rate_hz, append != 0, out_prev_samples_consumed);
#endif
}

void input_haptic_curve(input_haptic_ haptic_type, const float* in_arr_amplitudes, int32_t sample_count, float sample_rate_hz) {
#if defined(SK_XR_OPENXR)
	if (backend_xr_get_type() == backend_xr_type_openxr)
		oxri_haptic_curve(haptic_type, in_arr_amplitudes, sample_count, sample_rate_hz);
#endif
}

void input_haptic_stop(input_haptic_ haptic_type) {
#if defined(SK_XR_OPENXR)
	if (backend_xr_get_type() == backend_xr_type_openxr)
		oxri_haptic_stop(haptic_type);
#endif
}

///////////////////////////////////////////

pose_t        input_pose_get_local(input_pose_   pose_type)   { return pose_type   >= 0 && pose_type   < local.curr_poses  .count ? local.curr_poses[pose_type].pose : pose_identity; }
pose_t        input_pose          (input_pose_   pose_type)   { return pose_type   >= 0 && pose_type   < local.curr_poses  .count ? render_cam_final_transform(local.curr_poses[pose_type].pose) : pose_identity; }
float         input_float         (input_float_  float_type)  { return float_type  >= 0 && float_type  < local.curr_floats .count ? local.curr_floats [float_type]     : 0; }
button_state_ input_button        (input_button_ button_type) { return button_type >= 0 && button_type < local.curr_buttons.count ? local.curr_buttons[button_type]    : button_state_inactive; }
vec2          input_xy            (input_xy_     xy_type)     { return xy_type     >= 0 && xy_type     < local.curr_xys    .count ? local.curr_xys    [xy_type]        : vec2_zero; }

///////////////////////////////////////////

void input_reset() {
	if (!local.initialized) return;

	// Set all poses to un-tracked.
	ft_mutex_lock(local.mtx_poses);
	local.evt_poses.clear();
	ft_mutex_unlock(local.mtx_poses);
	for (int32_t i = 0; i < local.curr_poses.count; i++) {
		local.curr_poses[i].pos_tracked = track_state_lost;
		local.curr_poses[i].rot_tracked = track_state_lost;
	}

	// If any buttons are pressed, send an event to un-press them
	ft_mutex_lock(local.mtx_buttons);
	local.evt_buttons.clear();
	for (int32_t i = 0; i < local.curr_buttons.count; i++) {
		if (local.curr_buttons[i] & button_state_active)
			local.evt_buttons.add({(input_button_)i, false});
	}
	ft_mutex_unlock(local.mtx_buttons);

	// Reset floats to 0
	ft_mutex_lock(local.mtx_floats);
	local.evt_floats.clear();
	ft_mutex_unlock(local.mtx_floats);
	for (int32_t i = 0; i < local.curr_floats.count; i++) {
		local.curr_floats[i] = 0;
	}

	// Reset XYs to 0
	ft_mutex_lock(local.mtx_xys);
	local.evt_xys.clear();
	ft_mutex_unlock(local.mtx_xys);
	for (int32_t i = 0; i < local.curr_xys.count; i++) {
		local.curr_xys[i] = vec2_zero;
	}
}

///////////////////////////////////////////

void input_set_palm_offset(handed_ hand, pose_t offset) {
	local.palm_offset[hand] = offset;
}

///////////////////////////////////////////

pose_state_ input_pose_state(input_pose_ pose_type) {
	if (pose_type < 0 || pose_type >= local.curr_poses.count)
		return pose_state_lost;

	pose_info_t info   = local.curr_poses[pose_type];
	pose_state_ result = pose_state_lost;
	if (info.pos_tracked == track_state_inferred) result = (pose_state_)(result | pose_state_pos_inferred);
	if (info.pos_tracked == track_state_known   ) result = (pose_state_)(result | pose_state_pos_known);
	if (info.rot_tracked == track_state_inferred) result = (pose_state_)(result | pose_state_rot_inferred);
	if (info.rot_tracked == track_state_known   ) result = (pose_state_)(result | pose_state_rot_known);
	return result;
}

///////////////////////////////////////////

void body_make_shoulders(vec3* out_left, vec3* out_right) {
	// Average shoulder width for women:37cm, men:41cm, center of shoulder
	// joint is around 4cm inwards
	const float avg_shoulder_width = ((39.0f/2.0f)-4.0f)*cm2m;
	const float head_length        = 10*cm2m;
	const float neck_length        = 7*cm2m;

	// Chest center is down to the base of the head, and then down the neck.
	pose_t head = input_head();
	vec3 chest_center = head.position + head.orientation * vec3{0,-head_length,0};
	chest_center.y   -= neck_length;

	// Shoulder forward facing direction is head direction weighted equally 
	// with the direction of both hands.
	vec3 face_fwd = head.orientation * vec3_forward;
	face_fwd.y = 0;
	face_fwd   = vec3_normalize(face_fwd) * 2;
	face_fwd  += vec3_normalize(input_hand(handed_left )->wrist.position - chest_center);
	face_fwd  += vec3_normalize(input_hand(handed_right)->wrist.position - chest_center);
	face_fwd  *= 0.25f;
	vec3 face_right = vec3_normalize(vec3_cross(face_fwd, vec3_up)) * avg_shoulder_width;

	if (out_left)  *out_left  = chest_center - face_right;
	if (out_right) *out_right = chest_center + face_right;
}

} // namespace sk