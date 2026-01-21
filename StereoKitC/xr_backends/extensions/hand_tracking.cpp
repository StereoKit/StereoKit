/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

// This implements XR_EXT_hand_tracking, augmented with XR_EXT_hand_tracking_data_source
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_EXT_hand_tracking
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_EXT_hand_tracking_data_source

#include "../../stereokit.h"
#include "../openxr.h"
#include "ext_management.h"
#include "hand_tracking.h"
#include "../../device.h"
#include "../../systems/input.h"
#include "../../systems/render.h"
#include "../../hands/input_hand.h"
#include "../../libraries/stref.h"

#define XR_EXT_FUNCTIONS( X )  \
	X(xrCreateHandTrackerEXT)  \
	X(xrDestroyHandTrackerEXT) \
	X(xrLocateHandJointsEXT)
OPENXR_DEFINE_FN_STATIC(XR_EXT_FUNCTIONS);

typedef struct xr_hand_tracking_state_t {
	bool             has_data_source;
	bool             available;
	bool             hand_active;
	bool             tip_fix;
	float            hand_joint_scale;
	XrHandTrackerEXT hand_tracker[2];
	sk::hand_joint_t hand_joints [2][26];
} xr_hand_tracking_state_t;
static xr_hand_tracking_state_t local = { };

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

xr_system_ xr_ext_hand_tracking_initialize         (void*);
void       xr_ext_hand_tracking_shutdown           (void*);
bool       xr_ext_hand_tracking_is_tracked         ();
void       xr_ext_hand_tracking_update_joints      ();
void       xr_ext_hand_tracking_update_states      ();

bool       xr_ext_hand_tracking_sys_available      ();
void       xr_ext_hand_tracking_sys_update_inactive();
void       xr_ext_hand_tracking_sys_update_frame   ();
void       xr_ext_hand_tracking_sys_update_poses   ();

///////////////////////////////////////////

void xr_ext_hand_tracking_register() {
	xr_system_t sys = {};
	sys.request_exts    [sys.request_ext_count    ++] = XR_EXT_HAND_TRACKING_EXTENSION_NAME;
	sys.request_opt_exts[sys.request_opt_ext_count++] = XR_EXT_HAND_TRACKING_DATA_SOURCE_EXTENSION_NAME;
	sys.evt_initialize  = { xr_ext_hand_tracking_initialize };
	sys.evt_shutdown    = { xr_ext_hand_tracking_shutdown };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

bool xr_ext_hand_tracking_available() {
	return local.available;
}

///////////////////////////////////////////

XrHandTrackerEXT xr_ext_hand_tracking_get_tracker(handed_ hand) {
	return local.hand_tracker[hand];
}

///////////////////////////////////////////

xr_system_ xr_ext_hand_tracking_initialize(void*) {
	// Check if we got our extension
	if (!backend_openxr_ext_enabled(XR_EXT_HAND_TRACKING_EXTENSION_NAME))
		return xr_system_fail;
	local.has_data_source = backend_openxr_ext_enabled(XR_EXT_HAND_TRACKING_DATA_SOURCE_EXTENSION_NAME);

	// Check the system properties, see if we can actually do hand
	// tracking.
	XrSystemProperties                properties          = { XR_TYPE_SYSTEM_PROPERTIES };
	XrSystemHandTrackingPropertiesEXT properties_tracking = { XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_EXT };
	properties.next = &properties_tracking;
	XrResult result = xrGetSystemProperties(xr_instance, xr_system_id, &properties);
	if (XR_FAILED(result)) {
		log_infof("%s: [%s]", "xrGetSystemProperties", openxr_string(result));
		return xr_system_fail;
	}
	if (properties_tracking.supportsHandTracking == false)
		return xr_system_fail;

	// Exception for the articulated Vive Index hand simulation. This
	// simulation is insufficient to treat it like true articulated hands.
	//
	// We can skip this exception if Ultraleap's hand tracking layer is
	// present, as the Leap hand tracking layer supercedes the built-in
	// extensions.
	//
	// TODO: Remove this when the hand tracking data source extension is more
	// generally available.
	xr_runtime_ runtime = openxr_get_known_runtime();
#if defined(_M_X64) && defined(SK_OS_WINDOWS)
	if (local.has_data_source == false && (runtime == xr_runtime_wmr || runtime == xr_runtime_steamvr)) {

		// Check if we have the Ultraleap layer
		bool     has_leap_layer = false;
		uint32_t xr_layer_count = 0;
		xrEnumerateApiLayerProperties(0, &xr_layer_count, nullptr);
		XrApiLayerProperties *xr_layers = sk_malloc_t(XrApiLayerProperties, xr_layer_count);
		for (uint32_t i = 0; i < xr_layer_count; i++) xr_layers[i] = { XR_TYPE_API_LAYER_PROPERTIES };
		xrEnumerateApiLayerProperties(xr_layer_count, &xr_layer_count, xr_layers);
		for (uint32_t i = 0; i < xr_layer_count; i++) {
			if (strcmp(xr_layers[i].layerName, "XR_APILAYER_ULTRALEAP_hand_tracking") == 0) {
				has_leap_layer = true;
				break;
			}
		}
		sk_free(xr_layers);

		if (has_leap_layer == false && ext_management_is_user_requested(XR_EXT_HAND_TRACKING_EXTENSION_NAME) == false) {
			log_diag("XR_EXT_hand_tracking - Rejected - Incompatible implementations on WMR and SteamVR.");
			return xr_system_fail;
		}
	}
#endif

	// Load all extension functions
	OPENXR_LOAD_FN_RETURN(XR_EXT_FUNCTIONS, xr_system_fail);

	for (int32_t h = 0; h < handed_max; h++) {
		XrHandTrackerCreateInfoEXT info = { XR_TYPE_HAND_TRACKER_CREATE_INFO_EXT };
		info.hand         = h == handed_left ? XR_HAND_LEFT_EXT : XR_HAND_RIGHT_EXT;
		info.handJointSet = XR_HAND_JOINT_SET_DEFAULT_EXT;

		// Tell OpenXR we _only_ want real articulated hands, nothing simulated
		// from controllers. Only works when EXT_hand_tracking_data_source is
		// present.
		XrHandTrackingDataSourceEXT     unobstructed = XR_HAND_TRACKING_DATA_SOURCE_UNOBSTRUCTED_EXT;
		XrHandTrackingDataSourceInfoEXT data_source  = { XR_TYPE_HAND_TRACKING_DATA_SOURCE_INFO_EXT };
		data_source.requestedDataSourceCount = 1;
		data_source.requestedDataSources     = &unobstructed;
		if (local.has_data_source)
			info.next = &data_source;

		XrResult result = xrCreateHandTrackerEXT(xr_session, &info, &local.hand_tracker[h]);
		if (XR_FAILED(result)) {
			log_warnf("xrCreateHandTrackerEXT failed: [%s]", openxr_string(result));
			input_hand_refresh_system();
			return xr_system_fail;
		}
	}
	local.hand_active      = xr_ext_hand_tracking_is_tracked();
	local.available        = true;
	local.hand_joint_scale = 1;

	// The Oculus runtime adds an offset position to the fingertips, so we
	// apply a fix to put them in the right place.
	if (runtime == xr_runtime_meta || runtime == xr_runtime_android_xr) {
		local.tip_fix = true;
	}

	device_data.has_hand_tracking = true;

	// Register this extension's functionality with our hand management system.
	hand_system_t sys = {};
	sys.priority        = 90;
	sys.system          = hand_system_oxr_articulated;
	sys.source          = hand_source_articulated;
	sys.pinch_blend     = 0.3f;
	sys.available       = xr_ext_hand_tracking_sys_available;
	sys.update_inactive = xr_ext_hand_tracking_sys_update_inactive;
	sys.update_frame    = xr_ext_hand_tracking_sys_update_frame;
	sys.update_poses    = xr_ext_hand_tracking_sys_update_poses;
	input_hand_system_register(sys);

	return xr_system_succeed;
}

///////////////////////////////////////////

void xr_ext_hand_tracking_shutdown(void*) {
	for (int32_t h = 0; h < handed_max; h++) {
		if (local.hand_tracker[h] != XR_NULL_HANDLE) xrDestroyHandTrackerEXT(local.hand_tracker[h]);
	}
	local = {};

	OPENXR_CLEAR_FN(XR_EXT_FUNCTIONS);
}

///////////////////////////////////////////

bool xr_ext_hand_tracking_sys_available() {
	return xr_session != XR_NULL_HANDLE && local.hand_active == true;
}

///////////////////////////////////////////

bool xr_ext_hand_tracking_is_tracked() {
	if (!local.available)
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
		xrLocateHandJointsEXT(local.hand_tracker[h], &locate_info, &locations);

		// We only want to say no if both hands are inactive.
		hands_active = hands_active || locations.isActive;
	}

	return hands_active;
}

///////////////////////////////////////////

void xr_ext_hand_tracking_update_joints() {

	vec3 shoulders[2];
	body_make_shoulders(&shoulders[handed_left], &shoulders[handed_right]);

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
		xrLocateHandJointsEXT(local.hand_tracker[h], &locate_info, &locations);

		// Update the tracking state of the hand, joint definitions here: https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_conventions_of_hand_joints
		// We're checking the index finger tip, palm, and wrist here to see if
		// any of them are tracked, just in case.
		// Joint 0, 6, 11, 16, 21 don't appear to be tracked on Pico 4
		bool valid_joints =
			(locations.jointLocations[10].locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) > 0 ||
			(locations.jointLocations[0 ].locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) > 0 ||
			(locations.jointLocations[1] .locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) > 0;
		// Vive XR Elite will mark joints as valid, but then send invalid joint
		// data such as 0,0,0,0 quats which will crash math functions.
		// Unfortunately, it seems impossible to restrict this to specific
		// joints, so all joints must be checked.
		for (uint32_t j = 0; j < locations.jointCount; j++) {
			XrHandJointLocationEXT jt = locations.jointLocations[j];
			float sum = jt.pose.orientation.x + jt.pose.orientation.y + jt.pose.orientation.z + jt.pose.orientation.w;
			if (sum == 0) {
				valid_joints = false;
				break;
			}
		}
		hand_t *inp_hand = input_hand_ref((handed_)h);
		inp_hand->tracked_state = button_make_state((inp_hand->tracked_state & button_state_active) > 0, valid_joints);

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
			memcpy(&local.hand_joints[h][j].position,    &locations.jointLocations[j].pose.position,    sizeof(vec3));
			memcpy(&local.hand_joints[h][j].orientation, &locations.jointLocations[j].pose.orientation, sizeof(quat));
			local.hand_joints[h][j].radius      = locations.jointLocations[j].radius * local.hand_joint_scale;
			local.hand_joints[h][j].position    = matrix_transform_pt(root, local.hand_joints[h][j].position);
			local.hand_joints[h][j].orientation = local.hand_joints[h][j].orientation * root_q;
		}

		// Some runtimes provide the finger tip at the very tip of the finger,
		// rather than the tip minus the radius. This will make the finger too
		// long when adding the radius, so we add this offset manually here.
		if (local.tip_fix) {
			for (int32_t t = 0; t < 5; t++) {
				int32_t idx = 1 + t * 5 + hand_joint_tip;
				local.hand_joints[h][idx].position += local.hand_joints[h][idx].orientation * vec3{ 0,0,local.hand_joints[h][idx].radius };
			}
		}

		// Copy the pose data into our hand
		hand_joint_t* pose = input_hand_get_pose_buffer((handed_)h);
		memcpy(&pose[1], &local.hand_joints[h][XR_HAND_JOINT_THUMB_METACARPAL_EXT], sizeof(hand_joint_t) * 24);
		memcpy(&pose[0], &local.hand_joints[h][XR_HAND_JOINT_THUMB_METACARPAL_EXT], sizeof(hand_joint_t)); // Thumb metacarpal is duplicated at the same location

		static const quat face_forward = quat_from_angles(-90,0,0);
		inp_hand->palm  = pose_t{ local.hand_joints[h][XR_HAND_JOINT_PALM_EXT ].position, face_forward * local.hand_joints[h][XR_HAND_JOINT_PALM_EXT ].orientation };
		inp_hand->wrist = pose_t{ local.hand_joints[h][XR_HAND_JOINT_WRIST_EXT].position,                local.hand_joints[h][XR_HAND_JOINT_WRIST_EXT].orientation };
		
		// Update the aim values
		inp_hand->aim.position = local.hand_joints[h][XR_HAND_JOINT_INDEX_PROXIMAL_EXT].position;

		vec3 dir   = inp_hand->aim.position - shoulders[h];
		vec3 right = local.hand_joints[h][XR_HAND_JOINT_LITTLE_PROXIMAL_EXT].position - inp_hand->aim.position;
		inp_hand->aim.orientation = quat_lookat_up(vec3_zero, dir, vec3_cross(dir, right));
	}

	if (!hands_active) {
		local.hand_active = false;
		input_hand_refresh_system();
	}
}

///////////////////////////////////////////

void xr_ext_hand_tracking_update_states() {
	for (int32_t h = 0; h < handed_max; h++) {
		hand_t* inp_hand = input_hand_ref((handed_)h);

		// Pinch values from the hand interaction profile typically have deep
		// knowledge about the hands, beyond what can be gleaned from the joint
		// data. In StereoKit, all interaction profiles are reported as
		// controllers, so we're checking if this controller's source is from
		// such a hand profile.
		if (input_controller_is_hand((handed_)h)) {
			inp_hand->pinch_activation = input_controller((handed_)h)->trigger;
			inp_hand->pinch_state      = button_make_state((inp_hand->pinch_state & button_state_active) > 0, inp_hand->pinch_activation >= 1);
		} else {
			input_hand_sim_trigger(inp_hand->pinch_state, inp_hand->fingers[hand_finger_index][hand_joint_tip], inp_hand->fingers[hand_finger_thumb][hand_joint_tip],
				PINCH_ACTIVATION_DIST, PINCH_DEACTIVATION_DIST, PINCH_MAX_DIST,
				&inp_hand->pinch_state, &inp_hand->pinch_activation);
		}
		input_hand_sim_trigger(inp_hand->grip_state, inp_hand->fingers[hand_finger_ring][hand_joint_tip], inp_hand->fingers[hand_finger_ring][hand_joint_root],
			GRIP_ACTIVATION_DIST, GRIP_DEACTIVATION_DIST, GRIP_MAX_DIST,
			&inp_hand->grip_state, &inp_hand->grip_activation);

		// The pointer should be tracked when the hand is tracked and in a
		// "ready pose" (facing the same general direction as the user). It
		// should remain tracked if it was activated, even if it's no longer in
		// a ready pose.
		pose_t head = input_head();
		const float near_dist  = 0.2f;
		const float hand_angle = 0.25f; // ~150 degrees
		bool is_pinched   = (inp_hand->pinch_state   & button_state_active) > 0;
		bool hand_tracked = (inp_hand->tracked_state & button_state_active) > 0;
		bool is_facing    = vec3_dot(vec3_normalize(inp_hand->aim.position - head.position), inp_hand->palm.orientation * vec3_forward) > hand_angle;
		bool is_far       = vec2_distance_sq({inp_hand->aim.position.x, inp_hand->aim.position.z}, {head.position.x, head.position.z}) > (near_dist*near_dist);
		bool was_ready    = (inp_hand->aim_ready & button_state_active) > 0;
		inp_hand->aim_ready = button_make_state(was_ready, hand_tracked && ((was_ready && is_pinched) || (is_facing && is_far)));
	}
}

///////////////////////////////////////////

void xr_ext_hand_tracking_sys_update_inactive() {
	if (xr_ext_hand_tracking_is_tracked()) {
		local.hand_active = true;
		input_hand_refresh_system();
	}
}

///////////////////////////////////////////

void xr_ext_hand_tracking_sys_update_frame() {
	xr_ext_hand_tracking_update_joints();
	xr_ext_hand_tracking_update_states();
}

///////////////////////////////////////////

void xr_ext_hand_tracking_sys_update_poses() {
	xr_ext_hand_tracking_update_joints();
}

///////////////////////////////////////////

void backend_openxr_set_hand_joint_scale(float joint_scale_factor) {
	local.hand_joint_scale = joint_scale_factor;
}

} // namespace sk