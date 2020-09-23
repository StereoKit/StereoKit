#include "../../stereokit.h"
#include "../../_stereokit.h"
#include "../platform/openxr.h"
#include "../input.h"
#include "input_hand.h"

#include <openxr/openxr.h>
#include <string.h>
#include <stdlib.h>

namespace sk {

///////////////////////////////////////////

XrHandTrackerEXT oxra_hand_tracker[2];
hand_joint_t     oxra_hand_joints[2][26];

///////////////////////////////////////////

bool hand_oxra_available() {
	return
		sk_active_runtime()  == runtime_mixedreality &&
		xr_session           != XR_NULL_HANDLE       &&
		xr_articulated_hands == true;
}

///////////////////////////////////////////

void hand_oxra_init() {
	for (int32_t h = 0; h < handed_max; h++) {
		XrHandTrackerCreateInfoEXT info = { XR_TYPE_HAND_TRACKER_CREATE_INFO_EXT };
		info.hand         = h == handed_left ? XR_HAND_LEFT_EXT : XR_HAND_RIGHT_EXT;
		info.handJointSet = XR_HAND_JOINT_SET_DEFAULT_EXT;
		XrResult result = xr_extensions.xrCreateHandTrackerEXT(xr_session, &info, &oxra_hand_tracker[h]);
		if (XR_FAILED(result)) {
			log_warnf("xrCreateHandTrackerEXT failed: [%s]", openxr_string(result));
			return;
		}
	}
}

///////////////////////////////////////////

void hand_oxra_shutdown() {
	for (int32_t h = 0; h < handed_max; h++) {
		xr_extensions.xrDestroyHandTrackerEXT(oxra_hand_tracker[h]);
	}
}

///////////////////////////////////////////

void hand_oxra_update_joints() {
	if (xr_time <= 0)
		return;

	for (int32_t h = 0; h < handed_max; h++) {
		XrHandJointsLocateInfoEXT locate_info = { XR_TYPE_HAND_JOINTS_LOCATE_INFO_EXT };
		locate_info.time      = xr_time;
		locate_info.baseSpace = xr_app_space;

		XrHandJointLocationEXT joint_locations[XR_HAND_JOINT_COUNT_EXT];
		XrHandJointLocationsEXT locations = { XR_TYPE_HAND_JOINT_LOCATIONS_EXT };
		locations.isActive = XR_FALSE;
		locations.jointCount = XR_HAND_JOINT_COUNT_EXT;
		locations.jointLocations = joint_locations;
		xr_extensions.xrLocateHandJointsEXT(oxra_hand_tracker[h], &locate_info, &locations);

		// Update the tracking state of the hand
		hand_t *inp_hand = (hand_t*)input_hand((handed_)h);
		inp_hand->tracked_state = button_make_state(inp_hand->tracked_state & button_state_active, locations.isActive);

		// Not tracked? Then we don't care about the pose!
		if (!locations.isActive)
			continue;

		// Get joint poses from OpenXR
		matrix root = render_get_cam_root();
		for (uint32_t j = 0; j < locations.jointCount; j++) {
			memcpy(&oxra_hand_joints[h][j].position,    &locations.jointLocations[j].pose.position,    sizeof(vec3));
			memcpy(&oxra_hand_joints[h][j].orientation, &locations.jointLocations[j].pose.orientation, sizeof(quat));
			oxra_hand_joints[h][j].radius = locations.jointLocations[j].radius * 0.85f;

			oxra_hand_joints[h][j].position    = matrix_mul_point   (root, oxra_hand_joints[h][j].position);
			oxra_hand_joints[h][j].orientation = matrix_mul_rotation(root, oxra_hand_joints[h][j].orientation);
		}

		// Copy the pose data into our hand
		hand_joint_t* pose = input_hand_get_pose_buffer((handed_)h);
		memcpy(&pose[1], &oxra_hand_joints[h][XR_HAND_JOINT_THUMB_METACARPAL_EXT], sizeof(hand_joint_t) * 24);
		memcpy(&pose[0], &oxra_hand_joints[h][XR_HAND_JOINT_THUMB_METACARPAL_EXT], sizeof(hand_joint_t)); // Thumb metacarpal is duplicated at the same location

		static const quat face_forward = quat_from_angles(-90,0,0);
		inp_hand->palm  = pose_t{ oxra_hand_joints[h][XR_HAND_JOINT_PALM_EXT ].position, face_forward * oxra_hand_joints[h][XR_HAND_JOINT_PALM_EXT ].orientation };
		inp_hand->wrist = pose_t{ oxra_hand_joints[h][XR_HAND_JOINT_WRIST_EXT].position, face_forward * oxra_hand_joints[h][XR_HAND_JOINT_WRIST_EXT].orientation };
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