#include "../platforms/platform_utils.h"
#if defined(SK_XR_OPENXR)

#include "../stereokit.h"
#include "../_stereokit.h"
#include "../sk_memory.h"
#include "../xr_backends/openxr.h"
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

///////////////////////////////////////////

void hand_oxra_update_system_meshes();

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
	oxra_hand_active        = hand_oxra_is_tracked();
	oxra_system_initialized = true;

	if (xr_has_hand_meshes) {
		// Pre-allocate memory for the hand mesh structure
		XrSystemProperties                      properties          = { XR_TYPE_SYSTEM_PROPERTIES };
		XrSystemHandTrackingMeshPropertiesMSFT  properties_handmesh = { XR_TYPE_SYSTEM_HAND_TRACKING_MESH_PROPERTIES_MSFT };
		properties.next = &properties_handmesh;
		xrGetSystemProperties(xr_instance, xr_system_id, &properties);

		// Initialize hand mesh trackers
		for (int32_t h = 0; h < handed_max; h++) {
			hand_mesh_t *hand_mesh = input_hand_mesh_data((handed_)h);

			// Allocate memory for OpenXR to store hand mesh data in.
			oxra_mesh_src[h].indexBuffer.indexCapacityInput   = properties_handmesh.maxHandMeshIndexCount;
			oxra_mesh_src[h].indexBuffer.indices              = sk_malloc_t(uint32_t, properties_handmesh.maxHandMeshIndexCount);
			oxra_mesh_src[h].vertexBuffer.vertexCapacityInput = properties_handmesh.maxHandMeshVertexCount;
			oxra_mesh_src[h].vertexBuffer.vertices            = sk_malloc_t(XrHandMeshVertexMSFT, properties_handmesh.maxHandMeshIndexCount);

			// Allocate memory for the SK format mesh data.
			if (hand_mesh->ind_count < properties_handmesh.maxHandMeshIndexCount) {
				sk_free(hand_mesh->inds);
				hand_mesh->inds      = sk_malloc_t(vind_t, properties_handmesh.maxHandMeshIndexCount);
				hand_mesh->ind_count = properties_handmesh.maxHandMeshIndexCount;
			}
			if (hand_mesh->vert_count < properties_handmesh.maxHandMeshVertexCount) {
				sk_free(hand_mesh->verts);
				hand_mesh->verts      = sk_malloc_t(vert_t, properties_handmesh.maxHandMeshVertexCount);
				hand_mesh->vert_count = properties_handmesh.maxHandMeshVertexCount;
				for (uint32_t i = 0; i < hand_mesh->vert_count; i++)
					hand_mesh->verts[i] = { vec3_zero, vec3_zero, {0.5f,0.5f}, {255,255,255,255} };
			}

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
		xrDestroySpace(oxra_hand_space[h]);
		sk_free(oxra_mesh_src[h].indexBuffer.indices);
		sk_free(oxra_mesh_src[h].vertexBuffer.vertices);
	}
}

///////////////////////////////////////////

void hand_oxra_update_joints() {
	// Generate some shoulder data used for generating hand pointers

	// Average shoulder width for women:37cm, men:41cm, center of shoulder
	// joint is around 4cm inwards
	const float avg_shoulder_width = ((39.0f/2.0f)-4.0f)*cm2m;
	const float head_length        = 10*cm2m;
	const float neck_length        = 7*cm2m;

	// Chest center is down to the base of the head, and then down the neck.
	const pose_t *head = input_head();
	vec3 chest_center = head->position + head->orientation * vec3{0,-head_length,0};
	chest_center.y   -= neck_length;

	// Shoulder forward facing direction is head direction weighted equally 
	// with the direction of both hands.
	vec3 face_fwd = input_head()->orientation * vec3_forward;
	face_fwd.y = 0;
	face_fwd   = vec3_normalize(face_fwd) * 2;
	face_fwd  += vec3_normalize(input_hand(handed_left )->wrist.position - chest_center);
	face_fwd  += vec3_normalize(input_hand(handed_right)->wrist.position - chest_center);
	face_fwd  *= 0.25f;
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

		// Create pointers for the hands
		vec3   shoulder   = chest_center + face_right * (h == handed_right ? 1.0f : -1.0f);
		vec3   ray_joint  = oxra_hand_joints[h][XR_HAND_JOINT_INDEX_PROXIMAL_EXT].position;
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
	oxra_mesh_dirty[0] = true;
	oxra_mesh_dirty[1] = true;

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

void hand_oxra_update_poses(bool update_visuals) {
	hand_oxra_update_joints();

	if (update_visuals) {
		if (xr_has_hand_meshes) hand_oxra_update_system_meshes();
		else                    input_hand_update_meshes();
	}
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

void hand_oxra_update_system_meshes() {
	for (int32_t h = 0; h < handed_max; h++) {
		hand_mesh_t *hand_mesh = input_hand_mesh_data((handed_)h);

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

}

#endif