/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

// This implements XR_MSFT_hand_tracking_mesh
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_MSFT_hand_tracking_mesh

#include "hand_mesh.h"
#include "hand_tracking.h"
#include "ext_management.h"

#include "../../stereokit.h"
#include "../../systems/render.h"
#include "../../hands/input_hand.h"

#define XR_EXT_FUNCTIONS( X )    \
	X(xrCreateHandMeshSpaceMSFT) \
	X(xrUpdateHandMeshMSFT)
OPENXR_DEFINE_FN_STATIC(XR_EXT_FUNCTIONS);

typedef struct xr_hand_mesh_state_t {
	bool           available;
	XrSpace        hand_space[sk::handed_::handed_max];
	bool           mesh_dirty[sk::handed_::handed_max];
	XrHandMeshMSFT mesh_src  [sk::handed_::handed_max];
} xr_hand_mesh_state_t;
static xr_hand_mesh_state_t local = { };

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

xr_system_ xr_ext_msft_hand_mesh_initialize(void*);
void       xr_ext_msft_hand_mesh_shutdown  (void*);

///////////////////////////////////////////

void xr_ext_msft_hand_mesh_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_MSFT_HAND_TRACKING_MESH_EXTENSION_NAME;
	sys.evt_initialize  = { xr_ext_msft_hand_mesh_initialize };
	sys.evt_shutdown    = { xr_ext_msft_hand_mesh_shutdown };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_msft_hand_mesh_initialize(void*) {
	// Check if we got our extension
	if (!backend_openxr_ext_enabled(XR_MSFT_HAND_TRACKING_MESH_EXTENSION_NAME))
		return xr_system_fail;
	// This extension depends on the hand tracking extension, which we're
	// managing separately.
	if (!xr_ext_hand_tracking_available())
		return xr_system_fail;

	// Check the system properties, see if we can actually do a hand mesh.
	XrSystemProperties                     properties      = { XR_TYPE_SYSTEM_PROPERTIES };
	XrSystemHandTrackingMeshPropertiesMSFT properties_mesh = { XR_TYPE_SYSTEM_HAND_TRACKING_MESH_PROPERTIES_MSFT };
	properties.next = &properties_mesh;
	XrResult result = xrGetSystemProperties(xr_instance, xr_system_id, &properties);
	if (XR_FAILED(result)) {
		log_infof("%s: [%s]", "xrGetSystemProperties", openxr_string(result));
		return xr_system_fail;
	}
	if (properties_mesh.supportsHandTrackingMesh == false)
		return xr_system_fail;

	// If hand tracking data source is missing, we don't want to use this ext
	// on systems that provide bad quality controller based hand sims.
#if defined(_M_X64) && defined(SK_OS_WINDOWS)
	xr_runtime_ runtime = openxr_get_known_runtime();
	if (!backend_openxr_ext_enabled(XR_EXT_HAND_TRACKING_DATA_SOURCE_EXTENSION_NAME) && (runtime == xr_runtime_wmr || runtime == xr_runtime_steamvr)) {

		// Allow users to explicitly request this extension to bypass our
		// exception.
		if (ext_management_is_user_requested("XR_MSFT_hand_tracking_mesh") == false) {
			log_diag("XR_MSFT_hand_tracking_mesh - Rejected - Incompatible implementations on WMR and SteamVR.");
			return xr_system_fail;
		}
	}
#endif

	// Load all extension functions
	OPENXR_LOAD_FN_RETURN(XR_EXT_FUNCTIONS, xr_system_fail);

	// Pre-allocate memory for the hand mesh structure and initialize hand
	// mesh trackers.
	for (int32_t h = 0; h < handed_max; h++) {
		// Allocate memory for OpenXR to store hand mesh data in.
		local.mesh_src[h]                                  = { XR_TYPE_HAND_MESH_MSFT };
		local.mesh_src[h].indexBuffer.indexCapacityInput   = properties_mesh.maxHandMeshIndexCount;
		local.mesh_src[h].indexBuffer.indices              = sk_malloc_t(uint32_t, properties_mesh.maxHandMeshIndexCount);
		local.mesh_src[h].vertexBuffer.vertexCapacityInput = properties_mesh.maxHandMeshVertexCount;
		local.mesh_src[h].vertexBuffer.vertices            = sk_malloc_t(XrHandMeshVertexMSFT, properties_mesh.maxHandMeshVertexCount);
		local.mesh_dirty[h] = true;

		// Create the hand mesh space that's used to track the hand mesh
		// lifecycle.
		XrHandMeshSpaceCreateInfoMSFT info = { XR_TYPE_HAND_MESH_SPACE_CREATE_INFO_MSFT };
		info.handPoseType        = XR_HAND_POSE_TYPE_TRACKED_MSFT;
		info.poseInHandMeshSpace = { {0,0,0,1}, {0,0,0} };
		XrResult result = xrCreateHandMeshSpaceMSFT(xr_ext_hand_tracking_get_tracker((handed_)h), &info, &local.hand_space[h]);
		if (XR_FAILED(result)) {
			log_warnf("xrCreateHandMeshSpaceMSFT failed: %s", openxr_string(result));
			return xr_system_fail;
		}
	}

	local.available = true;
	return xr_system_succeed;
}

///////////////////////////////////////////

bool xr_ext_msft_hand_mesh_available() {
	return local.available;
}

///////////////////////////////////////////

void xr_ext_msft_hand_mesh_shutdown(void*) {
	for (int32_t h = 0; h < handed_max; h++) {
		sk_free(local.mesh_src[h].indexBuffer .indices );
		sk_free(local.mesh_src[h].vertexBuffer.vertices);

		if (local.hand_space[h] != XR_NULL_HANDLE) xrDestroySpace (local.hand_space[h]);
	}
	local = {};

	OPENXR_CLEAR_FN(XR_EXT_FUNCTIONS);
}

///////////////////////////////////////////

struct hand_tri_t {
	vind_t a, b, c;

	static int32_t compare_r(const void *a, const void *b) {
		const hand_tri_t *t1 = (hand_tri_t *)a;
		const hand_tri_t *t2 = (hand_tri_t *)b;
		XrVector3f a1 = local.mesh_src[handed_right].vertexBuffer.vertices[t1->a].position;
		XrVector3f a2 = local.mesh_src[handed_right].vertexBuffer.vertices[t1->b].position;
		XrVector3f a3 = local.mesh_src[handed_right].vertexBuffer.vertices[t1->c].position;
		XrVector3f b1 = local.mesh_src[handed_right].vertexBuffer.vertices[t2->a].position;
		XrVector3f b2 = local.mesh_src[handed_right].vertexBuffer.vertices[t2->b].position;
		XrVector3f b3 = local.mesh_src[handed_right].vertexBuffer.vertices[t2->c].position;
		return (int32_t) (((b1.x+b2.x+b3.x)*2000 + (b1.y+b2.y+b3.y)*-1000 + (b1.z+b2.z+b3.z)*-1000)
		                - ((a1.x+a2.x+a3.x)*2000 + (a1.y+a2.y+a3.y)*-1000 + (a1.z+a2.z+a3.z)*-1000));
	}
	static int32_t compare_l(const void *a, const void *b) {
		const hand_tri_t *t1 = (hand_tri_t *)a;
		const hand_tri_t *t2 = (hand_tri_t *)b;
		XrVector3f a1 = local.mesh_src[handed_left].vertexBuffer.vertices[t1->a].position;
		XrVector3f a2 = local.mesh_src[handed_left].vertexBuffer.vertices[t1->b].position;
		XrVector3f a3 = local.mesh_src[handed_left].vertexBuffer.vertices[t1->c].position;
		XrVector3f b1 = local.mesh_src[handed_left].vertexBuffer.vertices[t2->a].position;
		XrVector3f b2 = local.mesh_src[handed_left].vertexBuffer.vertices[t2->b].position;
		XrVector3f b3 = local.mesh_src[handed_left].vertexBuffer.vertices[t2->c].position;
		return (int32_t) (((b1.x+b2.x+b3.x)*-2000 + (b1.y+b2.y+b3.y)*-1000 + (b1.z+b2.z+b3.z)*-1000)
		                - ((a1.x+a2.x+a3.x)*-2000 + (a1.y+a2.y+a3.y)*-1000 + (a1.z+a2.z+a3.z)*-1000));
	}
};

///////////////////////////////////////////

void xr_ext_msft_hand_mesh_update_mesh(handed_ handed, hand_mesh_t* ref_hand_mesh) {
	int32_t          h       = handed;
	XrHandTrackerEXT tracker = xr_ext_hand_tracking_get_tracker(handed);

	if (ref_hand_mesh->mesh == nullptr) {
		ref_hand_mesh->mesh = mesh_create();
		mesh_set_keep_data(ref_hand_mesh->mesh, false);

		ref_hand_mesh->ind_count  = local.mesh_src[h].indexBuffer.indexCapacityInput;
		ref_hand_mesh->vert_count = local.mesh_src[h].vertexBuffer.vertexCapacityInput;
		ref_hand_mesh->inds       = sk_malloc_t(vind_t, ref_hand_mesh->ind_count );
		ref_hand_mesh->verts      = sk_malloc_t(vert_t, ref_hand_mesh->vert_count);
		for (uint32_t i = 0; i < ref_hand_mesh->vert_count; i++)
			ref_hand_mesh->verts[i] = { vec3_zero, vec3_zero, {0.5f,0.5f}, {255,255,255,255} };
	}

	XrHandMeshUpdateInfoMSFT info = { XR_TYPE_HAND_MESH_UPDATE_INFO_MSFT };
	info.handPoseType = XR_HAND_POSE_TYPE_TRACKED_MSFT;
	info.time         = xr_time;
	xrUpdateHandMeshMSFT(tracker, &info, &local.mesh_src[h]);

	// Hand mesh is in hand local space, so we'll provide a base matrix 
	// transform for the mesh.
	pose_t pose;
	if (openxr_get_space(local.hand_space[h], &pose, xr_time))
		ref_hand_mesh->root_transform = pose_matrix(pose) * render_get_cam_final();

	if (local.mesh_src[h].isActive) {
		// Update hand mesh indices when they've changed, or when we've
		// just switched away from another mesh type
		if (local.mesh_src[h].indexBufferChanged || local.mesh_dirty[h]) {
			for (uint32_t i = 0; i < local.mesh_src[h].indexBuffer.indexCountOutput; i+=3) {
				ref_hand_mesh->inds[i  ] = local.mesh_src[h].indexBuffer.indices[i+2];
				ref_hand_mesh->inds[i+1] = local.mesh_src[h].indexBuffer.indices[i+1];
				ref_hand_mesh->inds[i+2] = local.mesh_src[h].indexBuffer.indices[i];
			}
			// Sort the faces better rendering with transparency
			hand_tri_t *tris = (hand_tri_t*)ref_hand_mesh->inds;
			qsort(tris, local.mesh_src[h].indexBuffer.indexCountOutput / 3, sizeof(hand_tri_t), h == handed_left ? hand_tri_t::compare_l : hand_tri_t::compare_r);

			mesh_set_inds(ref_hand_mesh->mesh, ref_hand_mesh->inds, local.mesh_src[h].indexBuffer.indexCountOutput);
		}

		// Update hand mesh vertices when they've changed, or when we've
		// just switched away from another mesh type
		if (local.mesh_src[h].vertexBufferChanged || local.mesh_dirty[h]) {
			for (uint32_t v = 0; v < local.mesh_src[h].vertexBuffer.vertexCountOutput; v++) {
				memcpy(&ref_hand_mesh->verts[v].pos,  &local.mesh_src[h].vertexBuffer.vertices[v].position, sizeof(vec3));
				memcpy(&ref_hand_mesh->verts[v].norm, &local.mesh_src[h].vertexBuffer.vertices[v].normal,   sizeof(vec3));
			}
			mesh_set_verts(ref_hand_mesh->mesh, ref_hand_mesh->verts, local.mesh_src[h].vertexBuffer.vertexCountOutput, false);
		}

		// Calculate the UVs from the reference pose if the mesh is dirty
		if (local.mesh_dirty[h]) {
			XrHandMeshUpdateInfoMSFT mesh_info = { XR_TYPE_HAND_MESH_UPDATE_INFO_MSFT };
			mesh_info.handPoseType = XR_HAND_POSE_TYPE_REFERENCE_OPEN_PALM_MSFT;
			mesh_info.time         = xr_time;
			xrUpdateHandMeshMSFT(tracker, &mesh_info, &local.mesh_src[h]);

			// Calculate UVs roughly by using their polar coordinates,
			// Y axis is distance from the origin, and X axis is based on
			// the angle from one side to the other.
			for (uint32_t i = 0; i < local.mesh_src[h].vertexBuffer.vertexCountOutput; i++) {
				vec2 pt = {
					local.mesh_src[h].vertexBuffer.vertices[i].position.x,
					local.mesh_src[h].vertexBuffer.vertices[i].position.y };
				float angle = atan2f(fabsf(pt.y), h==handed_left?-pt.x:pt.x) * rad2deg;
				float mag   = fmaxf(0, fminf(pt.y*2, sqrtf(pt.x*pt.x + pt.y*pt.y)));

				ref_hand_mesh->verts[i].uv = { 
					(fminf(125, fmaxf(55, angle))-55)/70.0f, 
					1-(fminf(1,mag / .17f)) };
			}
		}
		local.mesh_dirty[h] = false;
	}
}

} // namespace sk