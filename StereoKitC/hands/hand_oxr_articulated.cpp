/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

#include "../platforms/platform.h"
#if defined(SK_XR_OPENXR)

#include "../stereokit.h"
#include "../_stereokit.h"
#include "../sk_memory.h"
#include "../xr_backends/openxr.h"
#include "../xr_backends/openxr_input.h"
#include "../xr_backends/openxr_extensions.h"
#include "../systems/input.h"
#include "../systems/render.h"
#include "input_hand.h"

#include <openxr/openxr.h>
#include <string.h>
#include <stdlib.h>

namespace sk {

///////////////////////////////////////////

XrHandTrackerEXT oxra_hand_tracker[2];
XrSpace          oxra_hand_space[2];
hand_joint_t     oxra_hand_joints[2][26];
bool             oxra_hand_active = true;
bool             oxra_system_initialized = false;
bool             oxra_mesh_dirty[2] = { true, true };
XrHandMeshMSFT   oxra_mesh_src[2] = { { XR_TYPE_HAND_MESH_MSFT }, { XR_TYPE_HAND_MESH_MSFT } };
float            oxra_hand_joint_scale = 1;

///////////////////////////////////////////

void hand_oxra_update_states();

///////////////////////////////////////////

void backend_openxr_set_hand_joint_scale(float joint_scale_factor) {
	oxra_hand_joint_scale = joint_scale_factor;
}

///////////////////////////////////////////

bool hand_oxra_available() {
	return
		sk_active_display_mode() == display_mode_mixedreality &&
		xr_session               != XR_NULL_HANDLE            &&
		xr_ext.EXT_hand_tracking == xr_ext_active             &&
		oxra_hand_active         == true;
}

///////////////////////////////////////////

bool hand_oxra_is_tracked() {
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
		xr_ext.EXT_hand_tracking != xr_ext_active)
		return;

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
		if (xr_ext.EXT_hand_tracking_data_source == xr_ext_active)
			info.next = &data_source;

		XrResult result = xr_extensions.xrCreateHandTrackerEXT(xr_session, &info, &oxra_hand_tracker[h]);
		if (XR_FAILED(result)) {
			log_warnf("xrCreateHandTrackerEXT failed: [%s]", openxr_string(result));
			xr_ext.EXT_hand_tracking = xr_ext_disabled;
			input_hand_refresh_system();
			return;
		}
	}
	oxra_hand_active        = hand_oxra_is_tracked();
	oxra_system_initialized = true;

	if (xr_ext.MSFT_hand_tracking_mesh == xr_ext_active) {
		// Pre-allocate memory for the hand mesh structure
		XrSystemProperties                      properties          = { XR_TYPE_SYSTEM_PROPERTIES };
		XrSystemHandTrackingMeshPropertiesMSFT  properties_handmesh = { XR_TYPE_SYSTEM_HAND_TRACKING_MESH_PROPERTIES_MSFT };
		properties.next = &properties_handmesh;
		xrGetSystemProperties(xr_instance, xr_system_id, &properties);

		// Initialize hand mesh trackers
		for (int32_t h = 0; h < handed_max; h++) {
			// Allocate memory for OpenXR to store hand mesh data in.
			oxra_mesh_src[h].indexBuffer.indexCapacityInput   = properties_handmesh.maxHandMeshIndexCount;
			oxra_mesh_src[h].indexBuffer.indices              = sk_malloc_t(uint32_t, properties_handmesh.maxHandMeshIndexCount);
			oxra_mesh_src[h].vertexBuffer.vertexCapacityInput = properties_handmesh.maxHandMeshVertexCount;
			oxra_mesh_src[h].vertexBuffer.vertices            = sk_malloc_t(XrHandMeshVertexMSFT, properties_handmesh.maxHandMeshVertexCount);

			// Create the hand mesh space that's used to track the hand mesh
			// lifecycle.
			XrHandMeshSpaceCreateInfoMSFT info = { XR_TYPE_HAND_MESH_SPACE_CREATE_INFO_MSFT };
			info.handPoseType        = XR_HAND_POSE_TYPE_TRACKED_MSFT;
			info.poseInHandMeshSpace = { {0,0,0,1}, {0,0,0} };
			XrResult result = xr_extensions.xrCreateHandMeshSpaceMSFT(oxra_hand_tracker[h], &info, &oxra_hand_space[h]);
			if (XR_FAILED(result)) {
				log_warnf("xrCreateHandMeshSpaceMSFT failed: %s", openxr_string(result));
			}
		}
	}
}

///////////////////////////////////////////

void hand_oxra_shutdown() {
	if (!oxra_system_initialized) return;
	
	for (int32_t h = 0; h < handed_max; h++) {
		xr_extensions.xrDestroyHandTrackerEXT(oxra_hand_tracker[h]);
		if (oxra_hand_space[h] != XR_NULL_HANDLE) xrDestroySpace(oxra_hand_space[h]);
		sk_free(oxra_mesh_src[h].indexBuffer.indices);
		sk_free(oxra_mesh_src[h].vertexBuffer.vertices);
	}

	oxra_hand_joint_scale = 1;
}

///////////////////////////////////////////

void hand_oxra_update_joints() {

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
		xr_extensions.xrLocateHandJointsEXT(oxra_hand_tracker[h], &locate_info, &locations);

		// Update the tracking state of the hand, joint definitions here: https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_conventions_of_hand_joints
		// We're checking the index finger tip, palm, and wrist here to see if
		// any of them are tracked, just in case.
		// Joint 0, 6, 11, 16, 21 don't appear to be tracked on Pico 4
		bool valid_joints =
			(locations.jointLocations[10].locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) > 0 ||
			(locations.jointLocations[0 ].locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) > 0 ||
			(locations.jointLocations[1 ].locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) > 0;
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
			memcpy(&oxra_hand_joints[h][j].position,    &locations.jointLocations[j].pose.position,    sizeof(vec3));
			memcpy(&oxra_hand_joints[h][j].orientation, &locations.jointLocations[j].pose.orientation, sizeof(quat));
			oxra_hand_joints[h][j].radius      = locations.jointLocations[j].radius * oxra_hand_joint_scale;
			oxra_hand_joints[h][j].position    = matrix_transform_pt(root, oxra_hand_joints[h][j].position);
			oxra_hand_joints[h][j].orientation = oxra_hand_joints[h][j].orientation * root_q;
		}

		// Copy the pose data into our hand
		hand_joint_t* pose = input_hand_get_pose_buffer((handed_)h);
		memcpy(&pose[1], &oxra_hand_joints[h][XR_HAND_JOINT_THUMB_METACARPAL_EXT], sizeof(hand_joint_t) * 24);
		memcpy(&pose[0], &oxra_hand_joints[h][XR_HAND_JOINT_THUMB_METACARPAL_EXT], sizeof(hand_joint_t)); // Thumb metacarpal is duplicated at the same location

		static const quat face_forward = quat_from_angles(-90,0,0);
		inp_hand->palm  = pose_t{ oxra_hand_joints[h][XR_HAND_JOINT_PALM_EXT ].position, face_forward * oxra_hand_joints[h][XR_HAND_JOINT_PALM_EXT ].orientation };
		inp_hand->wrist = pose_t{ oxra_hand_joints[h][XR_HAND_JOINT_WRIST_EXT].position,                oxra_hand_joints[h][XR_HAND_JOINT_WRIST_EXT].orientation };
		
		// Update the aim values
		inp_hand->aim.position = oxra_hand_joints[h][XR_HAND_JOINT_INDEX_PROXIMAL_EXT].position;

		vec3 dir   = inp_hand->aim.position - shoulders[h];
		vec3 right = oxra_hand_joints[h][XR_HAND_JOINT_LITTLE_PROXIMAL_EXT].position - inp_hand->aim.position;
		inp_hand->aim.orientation = quat_lookat_up(vec3_zero, dir, vec3_cross(dir, right));
	}

	if (!hands_active) {
		oxra_hand_active = false;
		input_hand_refresh_system();
	}
}

///////////////////////////////////////////

void hand_oxra_update_states() {
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
		const pose_t* head = input_head();
		const float near_dist  = 0.2f;
		const float hand_angle = 0.25f; // ~150 degrees
		bool is_pinched   = (inp_hand->pinch_state   & button_state_active) > 0;
		bool hand_tracked = (inp_hand->tracked_state & button_state_active) > 0;
		bool is_facing    = vec3_dot(vec3_normalize(inp_hand->aim.position - head->position), inp_hand->palm.orientation * vec3_forward) > hand_angle;
		bool is_far       = vec2_distance_sq({inp_hand->aim.position.x, inp_hand->aim.position.z}, {head->position.x, head->position.z}) > (near_dist*near_dist);
		bool was_ready    = (inp_hand->aim_ready & button_state_active) > 0;
		inp_hand->aim_ready = button_make_state(was_ready, hand_tracked && ((was_ready && is_pinched) || (is_facing && is_far)));

		// Update the hand pointer
		pointer_t* pointer = input_get_pointer(input_hand_pointer_id[h]);
		pointer->ray.pos     = inp_hand->aim.position;
		pointer->ray.dir     = inp_hand->aim.orientation * vec3_forward;
		pointer->orientation = inp_hand->aim.orientation;
		pointer->tracked = inp_hand->aim_ready;
	}
}

///////////////////////////////////////////

void hand_oxra_update_inactive() {

	if (hand_oxra_is_tracked()) {
		oxra_mesh_dirty[0] = true;
		oxra_mesh_dirty[1] = true;

		oxra_hand_active = true;
		input_hand_refresh_system();
	}
}

///////////////////////////////////////////

void hand_oxra_update_frame() {
	hand_oxra_update_joints();
	hand_oxra_update_states();
}

///////////////////////////////////////////

void hand_oxra_update_poses() {
	hand_oxra_update_joints();
}

///////////////////////////////////////////

struct hand_tri_t {
	vind_t a, b, c;

	static int32_t compare_r(const void *a, const void *b) {
		const hand_tri_t *t1 = (hand_tri_t *)a;
		const hand_tri_t *t2 = (hand_tri_t *)b;
		XrVector3f a1 = oxra_mesh_src[handed_right].vertexBuffer.vertices[t1->a].position;
		XrVector3f a2 = oxra_mesh_src[handed_right].vertexBuffer.vertices[t1->b].position;
		XrVector3f a3 = oxra_mesh_src[handed_right].vertexBuffer.vertices[t1->c].position;
		XrVector3f b1 = oxra_mesh_src[handed_right].vertexBuffer.vertices[t2->a].position;
		XrVector3f b2 = oxra_mesh_src[handed_right].vertexBuffer.vertices[t2->b].position;
		XrVector3f b3 = oxra_mesh_src[handed_right].vertexBuffer.vertices[t2->c].position;
		return (int32_t) (((b1.x+b2.x+b3.x)*2000 + (b1.y+b2.y+b3.y)*-1000 + (b1.z+b2.z+b3.z)*-1000)
		                - ((a1.x+a2.x+a3.x)*2000 + (a1.y+a2.y+a3.y)*-1000 + (a1.z+a2.z+a3.z)*-1000));
	}
	static int32_t compare_l(const void *a, const void *b) {
		const hand_tri_t *t1 = (hand_tri_t *)a;
		const hand_tri_t *t2 = (hand_tri_t *)b;
		XrVector3f a1 = oxra_mesh_src[handed_left].vertexBuffer.vertices[t1->a].position;
		XrVector3f a2 = oxra_mesh_src[handed_left].vertexBuffer.vertices[t1->b].position;
		XrVector3f a3 = oxra_mesh_src[handed_left].vertexBuffer.vertices[t1->c].position;
		XrVector3f b1 = oxra_mesh_src[handed_left].vertexBuffer.vertices[t2->a].position;
		XrVector3f b2 = oxra_mesh_src[handed_left].vertexBuffer.vertices[t2->b].position;
		XrVector3f b3 = oxra_mesh_src[handed_left].vertexBuffer.vertices[t2->c].position;
		return (int32_t) (((b1.x+b2.x+b3.x)*-2000 + (b1.y+b2.y+b3.y)*-1000 + (b1.z+b2.z+b3.z)*-1000)
		                - ((a1.x+a2.x+a3.x)*-2000 + (a1.y+a2.y+a3.y)*-1000 + (a1.z+a2.z+a3.z)*-1000));
	}
};

void hand_oxra_update_system_mesh(handed_ handed, hand_mesh_t* hand_mesh) {
	int32_t h = handed;

	if (hand_mesh->mesh == nullptr) {
		hand_mesh->mesh = mesh_create();
		mesh_set_keep_data(hand_mesh->mesh, false);

		hand_mesh->ind_count  = oxra_mesh_src[h].indexBuffer.indexCapacityInput;
		hand_mesh->vert_count = oxra_mesh_src[h].vertexBuffer.vertexCapacityInput;
		hand_mesh->inds       = sk_malloc_t(vind_t, hand_mesh->ind_count);
		hand_mesh->verts      = sk_malloc_t(vert_t, hand_mesh->vert_count);
		for (uint32_t i = 0; i < hand_mesh->vert_count; i++)
			hand_mesh->verts[i] = { vec3_zero, vec3_zero, {0.5f,0.5f}, {255,255,255,255} };
	}

	XrHandMeshUpdateInfoMSFT info = { XR_TYPE_HAND_MESH_UPDATE_INFO_MSFT };
	info.handPoseType = XR_HAND_POSE_TYPE_TRACKED_MSFT;
	info.time         = xr_time;
	xr_extensions.xrUpdateHandMeshMSFT(oxra_hand_tracker[h], &info, &oxra_mesh_src[h]);

	// Hand mesh is in hand local space, so we'll provide a base matrix 
	// transform for the mesh.
	pose_t pose;
	if (openxr_get_space(oxra_hand_space[h], &pose, xr_time))
		hand_mesh->root_transform = pose_matrix(pose) * render_get_cam_final();

	if (oxra_mesh_src[h].isActive) {
		// Update hand mesh indices when they've changed, or when we've
		// just switched away from another mesh type
		if (oxra_mesh_src[h].indexBufferChanged || oxra_mesh_dirty[h]) {
			for (uint32_t i = 0; i < oxra_mesh_src[h].indexBuffer.indexCountOutput; i+=3) {
				hand_mesh->inds[i  ] = oxra_mesh_src[h].indexBuffer.indices[i+2];
				hand_mesh->inds[i+1] = oxra_mesh_src[h].indexBuffer.indices[i+1];
				hand_mesh->inds[i+2] = oxra_mesh_src[h].indexBuffer.indices[i];
			}
			// Sort the faces better rendering with transparency
			hand_tri_t *tris = (hand_tri_t*)hand_mesh->inds;
			qsort(tris, oxra_mesh_src[h].indexBuffer.indexCountOutput / 3, sizeof(hand_tri_t), h == handed_left ? hand_tri_t::compare_l : hand_tri_t::compare_r);

			mesh_set_inds(hand_mesh->mesh, hand_mesh->inds, oxra_mesh_src[h].indexBuffer.indexCountOutput);
		}

		// Update hand mesh vertices when they've changed, or when we've
		// just switched away from another mesh type
		if (oxra_mesh_src[h].vertexBufferChanged || oxra_mesh_dirty[h]) {
			for (uint32_t v = 0; v < oxra_mesh_src[h].vertexBuffer.vertexCountOutput; v++) {
				memcpy(&hand_mesh->verts[v].pos,  &oxra_mesh_src[h].vertexBuffer.vertices[v].position, sizeof(vec3));
				memcpy(&hand_mesh->verts[v].norm, &oxra_mesh_src[h].vertexBuffer.vertices[v].normal,   sizeof(vec3));
			}
			mesh_set_verts(hand_mesh->mesh, hand_mesh->verts, oxra_mesh_src[h].vertexBuffer.vertexCountOutput, false);
		}

		// Calculate the UVs from the reference pose if the mesh is dirty
		if (oxra_mesh_dirty[h]) {
			XrHandMeshUpdateInfoMSFT mesh_info = { XR_TYPE_HAND_MESH_UPDATE_INFO_MSFT };
			mesh_info.handPoseType = XR_HAND_POSE_TYPE_REFERENCE_OPEN_PALM_MSFT;
			mesh_info.time         = xr_time;
			xr_extensions.xrUpdateHandMeshMSFT(oxra_hand_tracker[h], &mesh_info, &oxra_mesh_src[h]);

			// Calculate UVs roughly by using their polar coordinates,
			// Y axis is distance from the origin, and X axis is based on
			// the angle from one side to the other.
			for (uint32_t i = 0; i < oxra_mesh_src[h].vertexBuffer.vertexCountOutput; i++) {
				vec2 pt = {
					oxra_mesh_src[h].vertexBuffer.vertices[i].position.x,
					oxra_mesh_src[h].vertexBuffer.vertices[i].position.y };
				float angle = atan2f(fabsf(pt.y), h==handed_left?-pt.x:pt.x) * rad2deg;
				float mag   = fmaxf(0, fminf(pt.y*2, sqrtf(pt.x*pt.x + pt.y*pt.y)));

				hand_mesh->verts[i].uv = { 
					(fminf(125, fmaxf(55, angle))-55)/70.0f, 
					1-(fminf(1,mag / .17f)) };
			}
		}
		oxra_mesh_dirty[h] = false;
	}
}

}

#endif