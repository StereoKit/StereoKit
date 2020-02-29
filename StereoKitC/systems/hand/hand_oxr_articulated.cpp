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

const XrHandJointMSFT oxra_joints[] = {
	XR_HAND_JOINT_THUMB_METACARPAL_MSFT,
	XR_HAND_JOINT_THUMB_METACARPAL_MSFT,
	XR_HAND_JOINT_THUMB_PROXIMAL_MSFT,
	XR_HAND_JOINT_THUMB_DISTAL_MSFT,
	XR_HAND_JOINT_THUMB_TIP_MSFT,
	XR_HAND_JOINT_INDEX_METACARPAL_MSFT,
	XR_HAND_JOINT_INDEX_PROXIMAL_MSFT,
	XR_HAND_JOINT_INDEX_INTERMEDIATE_MSFT,
	XR_HAND_JOINT_INDEX_DISTAL_MSFT,
	XR_HAND_JOINT_INDEX_TIP_MSFT,
	XR_HAND_JOINT_MIDDLE_METACARPAL_MSFT,
	XR_HAND_JOINT_MIDDLE_PROXIMAL_MSFT,
	XR_HAND_JOINT_MIDDLE_INTERMEDIATE_MSFT,
	XR_HAND_JOINT_MIDDLE_DISTAL_MSFT,
	XR_HAND_JOINT_MIDDLE_TIP_MSFT,
	XR_HAND_JOINT_RING_METACARPAL_MSFT,
	XR_HAND_JOINT_RING_PROXIMAL_MSFT,
	XR_HAND_JOINT_RING_INTERMEDIATE_MSFT,
	XR_HAND_JOINT_RING_DISTAL_MSFT,
	XR_HAND_JOINT_RING_TIP_MSFT,
	XR_HAND_JOINT_LITTLE_METACARPAL_MSFT,
	XR_HAND_JOINT_LITTLE_PROXIMAL_MSFT,
	XR_HAND_JOINT_LITTLE_INTERMEDIATE_MSFT,
	XR_HAND_JOINT_LITTLE_DISTAL_MSFT,
	XR_HAND_JOINT_LITTLE_TIP_MSFT,
	XR_HAND_JOINT_PALM_MSFT,
	XR_HAND_JOINT_WRIST_MSFT, };

XrHandTrackerMSFT oxra_hand_tracker[2];
XrSpace           oxra_joint_space[2][27];
hand_joint_t      oxra_hand_joints[2][27];

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
		XrHandTrackerCreateInfoMSFT info = { XR_TYPE_HAND_TRACKER_CREATE_INFO_MSFT };
		info.hand = h == handed_left ? XR_HAND_LEFT_MSFT : XR_HAND_RIGHT_MSFT;
		XrResult result = xr_extensions.xrCreateHandTrackerMSFT(xr_session, &info, &oxra_hand_tracker[h]);
		if (XR_FAILED(result)) {
			log_warnf("xrCreateHandTrackerMSFT failed: [%s]", openxr_string(result));
			return;
		}

		XrHandPoseTypeInfoMSFT pose_info{ XR_TYPE_HAND_POSE_TYPE_INFO_MSFT };
		pose_info.handPoseType = XR_HAND_POSE_TYPE_TRACKED_MSFT;
		for (int32_t j = 0; j < _countof(oxra_joints); j++) {
			XrHandJointSpaceCreateInfoMSFT joint_info{ XR_TYPE_HAND_JOINT_SPACE_CREATE_INFO_MSFT, &pose_info };
			joint_info.joint            = oxra_joints[j];
			joint_info.handTracker      = oxra_hand_tracker[h];
			joint_info.poseInJointSpace = { {0,0,0,1}, {0,0,0} };

			result = xr_extensions.xrCreateHandJointSpaceMSFT(xr_session, &joint_info, &oxra_joint_space[h][j]);
			if (XR_FAILED(result)) {
				log_warnf("xrCreateHandJointSpaceMSFT failed: [%s]", openxr_string(result));
				return;
			}
		}
	}
}

///////////////////////////////////////////

void hand_oxra_shutdown() {
	for (int32_t h = 0; h < handed_max; h++) {
		for (int32_t j = 0; j < _countof(oxra_joints); j++)
			xrDestroySpace(oxra_joint_space[h][j]);
		xr_extensions.xrDestroyHandTrackerMSFT(oxra_hand_tracker[h]);
	}
}

///////////////////////////////////////////

void hand_oxra_update_joints() {
	if (xr_time <= 0)
		return;

	for (int32_t h = 0; h < handed_max; h++) {
		// Find if the hand is tracked
		XrHandTrackerStateMSFT state  = { XR_TYPE_HAND_TRACKER_STATE_MSFT };
		XrResult               result = xr_extensions.xrGetHandTrackerStateMSFT(oxra_hand_tracker[h], xr_time, &state);
		if (XR_FAILED(result)) {
			log_warnf("xrGetHandTrackerStateMSFT failed: [%s]", openxr_string(result));
			return;
		}

		// Update the tracking state of the hand
		hand_t& inp_hand = (hand_t&)input_hand((handed_)h);
		inp_hand.tracked_state = button_make_state(inp_hand.tracked_state & button_state_active, state.isActive);

		// Not tracked? Then we don't care about the pose!
		if (!state.isActive)
			continue;

		// Get joint poses from OpenXR
		for (int32_t j = 0; j < _countof(oxra_joints); j++) {
			XrHandJointRadiusMSFT joint_radius   = { XR_TYPE_HAND_JOINT_RADIUS_MSFT };
			XrSpaceLocation       space_location = { XR_TYPE_SPACE_LOCATION, &joint_radius };
			xrLocateSpace(oxra_joint_space[h][j], xr_app_space, xr_time, &space_location);

			memcpy(&oxra_hand_joints[h][j].position,    &space_location.pose.position,    sizeof(vec3));
			memcpy(&oxra_hand_joints[h][j].orientation, &space_location.pose.orientation, sizeof(quat));
			oxra_hand_joints[h][j].radius = joint_radius.radius * 1.2f;
		}

		// Copy the pose data into our hand
		hand_joint_t* pose = input_hand_get_pose_buffer((handed_)h);
		memcpy(pose, &oxra_hand_joints[h], sizeof(hand_joint_t) * 25);

		static const quat face_forward = quat_from_angles(-90,0,0);
		inp_hand.palm  = pose_t{ oxra_hand_joints[h][25].position, face_forward * oxra_hand_joints[h][25].orientation };
		inp_hand.wrist = pose_t{ oxra_hand_joints[h][26].position, face_forward * oxra_hand_joints[h][26].orientation };
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