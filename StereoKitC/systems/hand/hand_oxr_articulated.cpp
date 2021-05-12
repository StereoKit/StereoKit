#include "../../stereokit.h"
#include "../../_stereokit.h"
#include "../platform/openxr.h"
#include "../input.h"
#include "../render.h"
#include "input_hand.h"

#include <openxr/openxr.h>
#include <string.h>
#include <stdlib.h>

namespace sk {

///////////////////////////////////////////

XrHandTrackerEXT oxra_hand_tracker[2];
hand_joint_t     oxra_hand_joints[2][26];
bool             oxra_hand_active = true;
bool             oxra_system_initialized = false;

///////////////////////////////////////////

bool hand_oxra_available() {
	return
		sk_active_display_mode() == display_mode_mixedreality &&
		xr_session               != XR_NULL_HANDLE            &&
		xr_has_articulated_hands == true                      &&
		oxra_hand_active         == true;
}

///////////////////////////////////////////

bool hand_oxra_is_tracked() {
	if (xr_time <= 0)
		return true;
	if (!oxra_system_initialized)
		return false;

	// If hand tracking isn't active, we'll just want to check if it ever
	// resumes activity, and then switch back to it.
	bool hands_active = false;
	for (int32_t h = 0; h < handed_max; h++) {
		XrHandJointsLocateInfoEXT locate_info = { XR_TYPE_HAND_JOINTS_LOCATE_INFO_EXT };
		locate_info.time      = xr_time;
		locate_info.baseSpace = xr_app_space;

		XrHandJointLocationEXT  joint_locations[XR_HAND_JOINT_COUNT_EXT];
		XrHandJointLocationsEXT locations = { XR_TYPE_HAND_JOINT_LOCATIONS_EXT };
		locations.isActive       = XR_FALSE;
		locations.jointCount     = XR_HAND_JOINT_COUNT_EXT;
		locations.jointLocations = joint_locations;
		xr_extensions.xrLocateHandJointsEXT(oxra_hand_tracker[h], &locate_info, &locations);

		// We only want to say no if both hands are inactive.
		hands_active = hands_active || locations.isActive;
	}

	return hands_active;
}

///////////////////////////////////////////

void hand_oxra_init() {
	if (sk_active_display_mode() != display_mode_mixedreality ||
		xr_session               == XR_NULL_HANDLE            ||
		xr_has_articulated_hands == false)
		return;

	for (int32_t h = 0; h < handed_max; h++) {
		XrHandTrackerCreateInfoEXT info = { XR_TYPE_HAND_TRACKER_CREATE_INFO_EXT };
		info.hand         = h == handed_left ? XR_HAND_LEFT_EXT : XR_HAND_RIGHT_EXT;
		info.handJointSet = XR_HAND_JOINT_SET_DEFAULT_EXT;
		XrResult result = xr_extensions.xrCreateHandTrackerEXT(xr_session, &info, &oxra_hand_tracker[h]);
		if (XR_FAILED(result)) {
			log_warnf("xrCreateHandTrackerEXT failed: [%s]", openxr_string(result));
			xr_has_articulated_hands = false;
			input_hand_refresh_system();
			return;
		}
	}
	oxra_hand_active = hand_oxra_is_tracked();
	oxra_system_initialized = true;
}

///////////////////////////////////////////

void hand_oxra_shutdown() {
	if (!oxra_system_initialized) return;
	
	for (int32_t h = 0; h < handed_max; h++) {
		xr_extensions.xrDestroyHandTrackerEXT(oxra_hand_tracker[h]);
	}
}

///////////////////////////////////////////

void hand_oxra_update_joints() {
	if (xr_time <= 0)
		return;

	// Generate some shoulder data used for generating hand pointers

	// Average shoulder width for women:37cm, men:41cm, but 2/3 that feels a
	// bit better.
	const float avg_shoulder_width = (39.0f/2.0f) * cm2m * 0.66f;

	vec3 chest_center = input_head()->position;
	chest_center.y -= 25*cm2m;
	vec3 face_fwd = input_head()->orientation * vec3_forward;
	face_fwd.y = 0;
	vec3 face_right = vec3_normalize(vec3_cross(face_fwd, vec3_up)) * avg_shoulder_width; 

	bool hands_active = false;
	for (int32_t h = 0; h < handed_max; h++) {
		pointer_t* pointer = input_get_pointer(input_hand_pointer_id[h]);

		XrHandJointsLocateInfoEXT locate_info = { XR_TYPE_HAND_JOINTS_LOCATE_INFO_EXT };
		locate_info.time      = xr_time;
		locate_info.baseSpace = xr_app_space;

		XrHandJointLocationEXT  joint_locations[XR_HAND_JOINT_COUNT_EXT];
		XrHandJointLocationsEXT locations = { XR_TYPE_HAND_JOINT_LOCATIONS_EXT };
		locations.isActive       = XR_FALSE;
		locations.jointCount     = XR_HAND_JOINT_COUNT_EXT;
		locations.jointLocations = joint_locations;
		xr_extensions.xrLocateHandJointsEXT(oxra_hand_tracker[h], &locate_info, &locations);

		// Update the tracking state of the hand
		bool    valid_joints = (locations.jointLocations[0].locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) > 0;
		hand_t *inp_hand     = (hand_t*)input_hand((handed_)h);
		inp_hand->tracked_state = button_make_state(inp_hand->tracked_state & button_state_active, valid_joints);
		pointer->tracked = inp_hand->tracked_state;

		// If both hands aren't active at all, we'll want to to switch back
		// to controllers.
		hands_active = hands_active || locations.isActive;

		// No valid joints? Hand many not be visible, we can skip getting 
		// pose information.
		if (!valid_joints) {
			continue;
		}

		// Get joint poses from OpenXR
		matrix root   = render_get_cam_final();
		quat   root_q = matrix_extract_rotation(root);
		for (uint32_t j = 0; j < locations.jointCount; j++) {
			memcpy(&oxra_hand_joints[h][j].position,    &locations.jointLocations[j].pose.position,    sizeof(vec3));
			memcpy(&oxra_hand_joints[h][j].orientation, &locations.jointLocations[j].pose.orientation, sizeof(quat));
			oxra_hand_joints[h][j].radius      = locations.jointLocations[j].radius;
			oxra_hand_joints[h][j].position    = matrix_mul_point(root, oxra_hand_joints[h][j].position);
			oxra_hand_joints[h][j].orientation = oxra_hand_joints[h][j].orientation * root_q;
		}

		// Copy the pose data into our hand
		hand_joint_t* pose = input_hand_get_pose_buffer((handed_)h);
		memcpy(&pose[1], &oxra_hand_joints[h][XR_HAND_JOINT_THUMB_METACARPAL_EXT], sizeof(hand_joint_t) * 24);
		memcpy(&pose[0], &oxra_hand_joints[h][XR_HAND_JOINT_THUMB_METACARPAL_EXT], sizeof(hand_joint_t)); // Thumb metacarpal is duplicated at the same location

		static const quat face_forward = quat_from_angles(-90,0,0);
		inp_hand->palm  = pose_t{ oxra_hand_joints[h][XR_HAND_JOINT_PALM_EXT ].position, face_forward * oxra_hand_joints[h][XR_HAND_JOINT_PALM_EXT ].orientation };
		inp_hand->wrist = pose_t{ oxra_hand_joints[h][XR_HAND_JOINT_WRIST_EXT].position,                oxra_hand_joints[h][XR_HAND_JOINT_WRIST_EXT].orientation };

		// Create pointers for the hands
		vec3 shoulder  = chest_center + face_right * (h == handed_right ? 1.0f : -1.0f);
		vec3 ray_joint = oxra_hand_joints[h][XR_HAND_JOINT_INDEX_PROXIMAL_EXT].position;
		pose_t point_pose = {
			ray_joint,
			quat_lookat(shoulder, ray_joint) };
		pointer->ray.pos     = point_pose.position;
		pointer->ray.dir     = point_pose.orientation * vec3_forward;
		pointer->orientation = point_pose.orientation;
	}

	if (!hands_active) {
		oxra_hand_active = false;
		input_hand_refresh_system();
	}
}

///////////////////////////////////////////

void hand_oxra_update_inactive() {
	if (hand_oxra_is_tracked()) {
		oxra_hand_active = true;
		input_hand_refresh_system();
	}
}

///////////////////////////////////////////

void hand_oxra_update_frame() {
	hand_oxra_update_joints();
}

///////////////////////////////////////////

void hand_oxra_update_predicted() {
	hand_oxra_update_joints();
	input_hand_update_meshes();
}

}