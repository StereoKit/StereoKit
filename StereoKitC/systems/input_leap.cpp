#ifndef SK_NO_LEAP_MOTION

#pragma comment(lib, "leapC.lib")

#include "input.h"
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
bool leap_lock       = false;
hand_joint_t leap_hands[2][5][5];

const float leap_joint_size [5] = {.01f,.026f,.023f,.02f,.015f}; // in order of hand_joint_. found by measuring the width of my pointer finger when flattened on a ruler
const float leap_finger_size[5] = {1.15f,1,1,.85f,.75f}; // in order of hand_finger_. Found by comparing the distal joint of my index finger, with my other distal joints


void copy_hand(hand_t &sk_hand, hand_joint_t *dest, LEAP_HAND &hand);
void input_leap_thread(void *arg);

///////////////////////////////////////////

bool input_leap_init() {
	if (LeapCreateConnection(nullptr, &leap_handle) != eLeapRS_Success) {
		log_diag("Couldn't create connection to Leap Motion.");
		return false;
	}
	if (LeapOpenConnection(leap_handle) != eLeapRS_Success) {
		log_diag("Couldn't open a connection to Leap Motion.");
		return false;
	}
	
	_beginthread(input_leap_thread, 0, nullptr);

	log_diag("Leap Motion is installed on the system.");
	return true;
}

///////////////////////////////////////////

void input_leap_shutdown() {
	leap_run = false;
}

///////////////////////////////////////////

bool input_leap_update() {
	bool result = false;
	if (leap_has_new_hands && !leap_lock) {
		leap_lock = true;
		leap_has_new_hands = false;

		for (size_t i = 0; i < handed_max; i++) {
			hand_joint_t *pose = input_hand_get_pose_buffer((handed_)i);
			memcpy(pose, &leap_hands[i][0][0], sizeof(hand_joint_t) * 25);
		}
		leap_lock = false;
		result = true;
	}
	return result;
}

///////////////////////////////////////////

void input_leap_thread(void *arg) {
	LEAP_CONNECTION_MESSAGE msg;

	while (leap_run) {
		if (LeapPollConnection(leap_handle, 0, &msg) == eLeapRS_Success) {
			if (msg.type == eLeapEventType_Tracking) {
				const LEAP_TRACKING_EVENT *evt = msg.tracking_event;

				if (!leap_lock) {
					leap_lock = true;
					
					bool32_t track_state_prev[2] = { 
						input_hand((handed_)0).tracked_state & button_state_active, 
						input_hand((handed_)1).tracked_state & button_state_active,};
					bool32_t track_state_curr[2] = {};

					for (size_t i = 0; i < evt->nHands; i++) {
						LEAP_HAND    &hand   = evt->pHands[i];
						handed_       handed = hand.type == eLeapHandType_Left ? handed_left : handed_right;
						hand_joint_t *pose   = input_hand_get_pose_buffer(handed);
						track_state_curr[handed] = true;

						hand_t &inp_hand = (hand_t &)input_hand(handed);						
						copy_hand(inp_hand, &leap_hands[handed][0][0], hand);
					}

					for (size_t i = 0; i < handed_max; i++) {
						hand_t &inp_hand = (hand_t &)input_hand((handed_)i);
						inp_hand.tracked_state = button_make_state(track_state_prev[i], track_state_curr[i]);
					}
					
					leap_lock = false;
					leap_has_new_hands = true;
				}
			} else if (msg.type == eLeapEventType_Device) {
				log_diag("Connected to Leap Motion device!");
				LeapSetPolicyFlags(leap_handle, eLeapPolicyFlag_OptimizeHMD, 0);
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

void copy_hand(hand_t &sk_hand, hand_joint_t *dest, LEAP_HAND &hand) {
	vec3 offset = {}; //{ 0, -0.25f, -0.15f };

	matrix axis_convert = { {
		{-1,0,0,0},
		{0,0,-1,0},
		{0,-1,0,0},
		{0,0,-80,1}
	} };
	const pose_t &head = input_head();
	matrix to_world = axis_convert * matrix_trs(vec3_zero, quat_identity, vec3_one * 0.001f);
	to_world = to_world * matrix_trs(vec3_zero, head.orientation);
	auto to_quat = [](LEAP_QUATERNION q) {return quat_from_angles(90,0,180)*quat{ -q.x, -q.z, -q.y, q.w }; };

	// Copy the palm
	memcpy(&sk_hand.palm.position, &hand.palm.position, sizeof(vec3));
	sk_hand.palm.orientation = quat_from_angles(-90,0,0) * to_quat(hand.palm.orientation) * head.orientation;
	sk_hand.palm.position    = matrix_mul_point( to_world, sk_hand.palm.position) + head.position;

	for (size_t f = 0; f < 5; f++) {
		LEAP_BONE    &bone = hand.digits[f].bones[0];
		hand_joint_t *pose = (dest+f*5);

		memcpy(&pose->position, &bone.prev_joint, sizeof(vec3));
		pose->orientation = to_quat(bone.rotation) *head.orientation;
		pose->position    = matrix_mul_point( to_world, pose->position) + head.position;
		pose->radius      = leap_finger_size[f] * leap_joint_size[0] * 0.35f;
	}
	for (size_t f = 0; f < 5; f++) {
		for (size_t j = 0; j < 4; j++) {
			LEAP_BONE    &bone = hand.digits[f].bones[j];
			hand_joint_t *pose = ((dest+f*5)+(j+1));

			memcpy(&pose->position,    &bone.next_joint, sizeof(vec3));
			pose->orientation = to_quat(bone.rotation) *head.orientation;
			pose->position    = matrix_mul_point(to_world, pose->position) + head.position;
			pose->radius      = leap_finger_size[f] * leap_joint_size[j+1] * 0.35f;
		}
	}
}

} // namespace sk

#endif