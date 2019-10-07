#ifndef SK_NO_LEAP_MOTION

#pragma comment(lib, "leapC.lib")

#include "input_leap.h"
#include "../stereokit.h"
#include "input_hand.h"
#include "render.h"

#include <leapC.h>
#include <string.h>
#include <stdlib.h>
#include <process.h>

namespace sk {

///////////////////////////////////////////

LEAP_CONNECTION      leap_handle      = nullptr;
LEAP_TRACKING_EVENT *leap_last_frame  = nullptr;
LEAP_DEVICE_INFO    *leap_last_device = nullptr;

bool leap_run        = true;
bool leap_has_device = false;
bool leap_has_new_hands = false;
pose_t leap_hands[2][5][5];

void copy_hand(pose_t *dest, LEAP_HAND &hand);
void input_leap_thread(void *arg);

///////////////////////////////////////////

bool input_leap_init() {
	if (LeapCreateConnection(nullptr, &leap_handle) != eLeapRS_Success)
		return false;
	if (LeapOpenConnection(leap_handle) != eLeapRS_Success)
		return false;

	_beginthread(input_leap_thread, 0, nullptr);

	return true;
}

///////////////////////////////////////////

void input_leap_shutdown() {
	leap_run = false;
}

///////////////////////////////////////////

void input_leap_update() {
	if (leap_has_new_hands) {
		leap_has_new_hands = false;

		for (size_t i = 0; i < handed_max; i++) {
			pose_t *pose = input_hand_get_pose_buffer((handed_)i);
			memcpy(pose, &leap_hands[i][0][0], sizeof(pose_t) * 25);
		}
	}
}

///////////////////////////////////////////

void input_leap_thread(void *arg) {
	LEAP_CONNECTION_MESSAGE msg;

	while (leap_run) {
		if (LeapPollConnection(leap_handle, 0, &msg) == eLeapRS_Success) {
			if (msg.type == eLeapEventType_Tracking) {
				const LEAP_TRACKING_EVENT *evt = msg.tracking_event;
				for (size_t i = 0; i < evt->nHands; i++) {
					LEAP_HAND &hand   = evt->pHands[i];
					handed_    handed = hand.type == eLeapHandType_Left ? handed_left : handed_right;
					pose_t    *pose   = input_hand_get_pose_buffer(handed);

					hand_t &inp_hand = (hand_t &)input_hand(handed);
					inp_hand.state = inp_hand.state | input_state_tracked;
					copy_hand(&leap_hands[handed][0][0], hand);
				}
				leap_has_new_hands = true;
			} else if (msg.type == eLeapEventType_Device) {
				leap_has_device = true;
			} else if (msg.type == eLeapEventType_DeviceLost) {
				leap_has_device = false;
			}
		}
	}
	LeapCloseConnection(leap_handle);
	LeapDestroyConnection(leap_handle);
}

///////////////////////////////////////////

void copy_hand(pose_t *dest, LEAP_HAND &hand) {
	camera_t    *cam;
	transform_t *tr;
	render_get_cam(&cam, &tr);
	vec3 offset = { 0,-0.25f,-0.15f };
	quat rot    = quat_euler(vec3{ 0,180,-90 });

	for (size_t f = 0; f < 5; f++) {
		LEAP_BONE &bone = hand.digits[f].bones[0];
		pose_t    *pose = (dest+f*5);

		memcpy(&pose->orientation, &bone.rotation,   sizeof(quat));
		memcpy(&pose->position,    &bone.prev_joint, sizeof(vec3));
		pose->position = transform_local_to_world(*tr, (pose->position * mm2m) + offset);
		pose->orientation = (rot * pose->orientation) * tr->_rotation;
	}
	for (size_t f = 0; f < 5; f++) {
		for (size_t j = 0; j < 4; j++) {
			LEAP_BONE &bone = hand.digits[f].bones[j];
			pose_t    *pose = ((dest+f*5)+(j+1));

			memcpy(&pose->orientation, &bone.rotation,   sizeof(quat));
			memcpy(&pose->position,    &bone.next_joint, sizeof(vec3));
			pose->position = transform_local_to_world(*tr, (pose->position * mm2m) + offset);
			pose->orientation = (rot * pose->orientation) * tr->_rotation;
		}
	}
}

} // namespace sk

#endif