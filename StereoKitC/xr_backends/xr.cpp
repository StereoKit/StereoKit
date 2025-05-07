// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

#include "xr.h"
#include "openxr.h"
#include "..\systems\input.h"

namespace sk {

	
struct xr_backend_state_t {
	int32_t eyes_pointer;
};
static xr_backend_state_t local = {};

///////////////////////////////////////////

bool xr_init() {
	bool result = true;
#if defined(SK_XR_OPENXR)
	result = openxr_init();
#endif

	local.eyes_pointer = input_add_pointer(input_source_gaze | (device_has_eye_gaze() ? input_source_gaze_eyes : input_source_gaze_head));

	return result;
}

///////////////////////////////////////////

void xr_step_begin() {
#if defined(SK_XR_OPENXR)
	openxr_step_begin();
#endif

	// Update the gaze pointer
	pointer_t* pointer = input_get_pointer(local.eyes_pointer);
	if (device_has_eye_gaze()) {
		pose_t       pose = input_pose_get_world(input_pose_eyes);
		track_state_ pos_tracked, rot_tracked;
		input_pose_get_state(input_pose_eyes, &pos_tracked, &rot_tracked);

		pointer->ray.pos     = pose.position;
		pointer->ray.dir     = pose.orientation * vec3_forward;
		pointer->orientation = pose.orientation;
		pointer->tracked     = button_make_state(
			(pointer->tracked & button_state_active) != 0,
			pos_tracked != track_state_lost || rot_tracked != track_state_lost);
	} else {
		pose_t pose = input_head();
		pointer->ray.pos     = pose.position;
		pointer->ray.dir     = pose.orientation * vec3_forward;
		pointer->orientation = pose.orientation;
		pointer->tracked     = world_get_tracked();
	}
}

///////////////////////////////////////////

void xr_step_end() {
#if defined(SK_XR_OPENXR)
	openxr_step_end();
#else
#endif
}

///////////////////////////////////////////

void xr_shutdown() {
#if defined(SK_XR_OPENXR)
	openxr_shutdown();
#else
#endif

	local = {};
}

}