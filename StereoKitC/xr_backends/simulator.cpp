#include "simulator.h"

#include "../_stereokit.h"
#include "../stereokit_ui.h" // for ui_has_keyboard_focus
#include "../device.h"
#include "../libraries/stref.h"
#include "../platforms/platform_utils.h"
#include "../systems/input.h"
#include "../systems/render.h"
#include "../systems/world.h"

namespace sk {

///////////////////////////////////////////

int32_t sim_gaze_pointer;
vec3    sim_head_rot;
vec3    sim_head_pos;
pose_t  sim_bounds_pose;
bool    sim_mouse_look;

const float sim_move_speed = 1.4f; // average human walk speed, see: https://en.wikipedia.org/wiki/Preferred_walking_speed;
const vec2  sim_rot_speed  = { 10.f, 5.f }; // converting mouse pixel movement to rotation;

///////////////////////////////////////////

void simulator_mouse_step();

///////////////////////////////////////////

bool simulator_init() {
	device_data.has_hand_tracking = true;
	device_data.has_eye_gaze      = true;
	device_data.tracking          = device_tracking_6dof;
	device_data.name              = string_copy("Simulator");

	sim_head_rot     = { -21, 0.0001f, 0 };
	sim_head_pos     = { 0, 0.2f, 0.0f };
	sim_mouse_look   = false;
	sim_gaze_pointer = input_add_pointer(input_source_gaze | input_source_gaze_head);

	quat initial_rot = quat_from_angles(0, sim_head_rot.y, 0);
	switch (sk_get_settings_ref()->origin) {
	case origin_mode_local: world_origin_mode = origin_mode_local; world_origin_offset = { sim_head_pos,                  initial_rot }; sim_bounds_pose = { 0,-1.5f,0, quat_inverse(initial_rot) }; break;
	case origin_mode_floor: world_origin_mode = origin_mode_local; world_origin_offset = { sim_head_pos - vec3{0,1.5f,0}, initial_rot }; sim_bounds_pose = { world_origin_offset.position, quat_inverse(world_origin_offset.orientation) }; break;
	case origin_mode_stage: world_origin_mode = origin_mode_local; world_origin_offset = { sim_head_pos - vec3{0,1.5f,0}, initial_rot }; sim_bounds_pose = { world_origin_offset.position, quat_inverse(world_origin_offset.orientation) }; break;
	}

	render_set_sim_origin(world_origin_offset);
	render_set_sim_head  (pose_t{ sim_head_pos, quat_from_angles(sim_head_rot.x, sim_head_rot.y, sim_head_rot.z) });

	return true;
}

///////////////////////////////////////////

void simulator_shutdown() {
}

///////////////////////////////////////////

void simulator_step_begin() {
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
		render_set_sim_head({ sim_head_pos, orientation });
	} else {
		sim_mouse_look = false;
	}
	if (input_key(key_mouse_right) & button_state_just_inactive) {
		sim_mouse_look = false;
	}

	if (sk_get_settings_ref()->disable_flatscreen_mr_sim) {
		input_eyes_track_state = button_make_state(input_eyes_track_state & button_state_active, false);
	} else {
		bool sim_tracked = (input_key(key_alt) & button_state_active) > 0 ? true : false;
		input_eyes_track_state = button_make_state(input_eyes_track_state & button_state_active, sim_tracked);
		ray_t ray = {};
		if (sim_tracked && ray_from_mouse(input_mouse_data.pos, ray)) {
			input_eyes_pose_world.position    = ray.pos;
			input_eyes_pose_world.orientation = quat_lookat(vec3_zero, ray.dir);
			input_eyes_pose_local.position    = matrix_transform_pt(render_get_cam_final_inv(), ray.pos);
			input_eyes_pose_local.orientation = quat_lookat(vec3_zero, matrix_transform_dir(render_get_cam_final_inv(), ray.dir));
		}
	}

	pointer_t *pointer_head = input_get_pointer(sim_gaze_pointer);
	pointer_head->tracked = button_state_active;
	pointer_head->ray.pos = input_eyes_pose_world.position;
	pointer_head->ray.dir = input_eyes_pose_world.orientation * vec3_forward;
}

///////////////////////////////////////////

void simulator_step_end() {
	input_update_poses(true);
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


}