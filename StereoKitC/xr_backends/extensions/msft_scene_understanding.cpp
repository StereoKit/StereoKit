/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

// This implements XR_MSFT_scene_understanding
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_MSFT_scene_understanding

#include "msft_scene_understanding.h"
#include "ext_management.h"
#include "../../_stereokit.h"
#include "../../systems/render.h"
#include "../../asset_types/mesh_.h"
#include <float.h>

#define XR_EXT_FUNCTIONS( X )              \
	X(xrCreateSceneMSFT)                   \
	X(xrCreateSceneObserverMSFT)           \
	X(xrComputeNewSceneMSFT)               \
	X(xrEnumerateSceneComputeFeaturesMSFT) \
	X(xrGetSceneMeshBuffersMSFT)           \
	X(xrGetSceneComputeStateMSFT)          \
	X(xrGetSceneComponentsMSFT)            \
	X(xrLocateSceneComponentsMSFT)         \
	X(xrDestroySceneMSFT)                  \
	X(xrDestroySceneObserverMSFT)
OPENXR_DEFINE_FN_STATIC(XR_EXT_FUNCTIONS);

///////////////////////////////////////////

namespace sk {

struct scene_request_info_t {
	bool32_t       occlusion;
	bool32_t       raycast;
	vec3           center;
	float          radius;
	world_refresh_ refresh_type;
	float          refresh_interval;
};

struct scene_mesh_t {
	uint64_t buffer_id;
	mesh_t   mesh;
	int32_t  references;
	bool     buffer_dirty;
	XrTime   buffer_updated;
};

struct su_mesh_inst_t {
	mesh_t   mesh_ref;
	matrix   local_transform;
	matrix   local_transform_inv;
};

typedef struct xr_scene_understanding_state_t {
	bool                    available;
	XrSceneObserverMSFT     scene_observer;
	bool                    scene_updating;
	bool                    scene_update_requested;
	XrSceneMSFT             scene;
	scene_request_info_t    scene_last_req;
	scene_request_info_t    scene_next_req;
	float                   scene_last_refresh;

	array_t<scene_mesh_t>   meshes;
	array_t<su_mesh_inst_t> scene_colliders;
	array_t<su_mesh_inst_t> scene_visuals;

	array_t<vert_t>         verts_tmp;
	array_t<XrVector3f>     vbuffer_tmp;
	array_t<uint32_t>       ibuffer_tmp;
} xr_scene_understanding_state_t;
static xr_scene_understanding_state_t local = { };

///////////////////////////////////////////

xr_system_ xr_ext_msft_su_initialize(void*);
void       xr_ext_msft_su_step_begin(void*);
void       xr_ext_msft_su_shutdown  (void*);

bool       oxr_su_is_su_needed             (scene_request_info_t info);
bool       oxr_su_check_needs_update       (scene_request_info_t info);
void       oxr_su_request_update           (scene_request_info_t info);
void       oxr_su_load_scene_meshes        (XrSceneComponentTypeMSFT type, array_t<su_mesh_inst_t>* mesh_list);
void       oxr_su_update_meshes            (array_t<scene_mesh_t>* mesh_list);

///////////////////////////////////////////

void xr_ext_msft_scene_understanding_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_MSFT_SCENE_UNDERSTANDING_EXTENSION_NAME;
	sys.evt_initialize  = { xr_ext_msft_su_initialize };
	sys.evt_step_end    = { xr_ext_msft_su_step_begin };
	sys.evt_shutdown    = { xr_ext_msft_su_shutdown   };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_msft_su_initialize(void*) {
	// Check if we got our extension
	if (!backend_openxr_ext_enabled(XR_MSFT_SCENE_UNDERSTANDING_EXTENSION_NAME))
		return xr_system_fail;

	// Load all extension functions
	OPENXR_LOAD_FN_RETURN(XR_EXT_FUNCTIONS, xr_system_fail);

	// Check scene understanding features, these may be dependant on Session
	// creation in the context of Holographic Remoting.
	system_info_t* sys_info = sk_get_info_ref();
	uint32_t       count    = 0;
	xrEnumerateSceneComputeFeaturesMSFT(xr_instance, xr_system_id, 0, &count, nullptr);
	XrSceneComputeFeatureMSFT* features = sk_malloc_t(XrSceneComputeFeatureMSFT, count);
	xrEnumerateSceneComputeFeaturesMSFT(xr_instance, xr_system_id, count, &count, features);
	for (uint32_t i = 0; i < count; i++) {
		if      (features[i] == XR_SCENE_COMPUTE_FEATURE_VISUAL_MESH_MSFT  ) sys_info->world_occlusion_present = true;
		else if (features[i] == XR_SCENE_COMPUTE_FEATURE_COLLIDER_MESH_MSFT) sys_info->world_raycast_present = true;
	}
	sk_free(features);

	if (sys_info->world_occlusion_present) log_diagf("XR_MSFT_scene_understanding - Supports %s.", "occlusion");
	if (sys_info->world_raycast_present)   log_diagf("XR_MSFT_scene_understanding - Supports %s.", "raycast");

	local.available = true;
	local.scene_last_refresh = -1000;
	return xr_system_succeed;
}

///////////////////////////////////////////

void xr_ext_msft_su_shutdown(void*) {
	if (local.scene          != XR_NULL_HANDLE) xrDestroySceneMSFT        (local.scene);
	if (local.scene_observer != XR_NULL_HANDLE) xrDestroySceneObserverMSFT(local.scene_observer);

	local.meshes.each([](scene_mesh_t &m) { mesh_release(m.mesh); });

	local.verts_tmp      .free();
	local.vbuffer_tmp    .free();
	local.ibuffer_tmp    .free();
	local.meshes         .free();
	local.scene_colliders.free();
	local.scene_visuals  .free();
	local = {};

	OPENXR_CLEAR_FN(XR_EXT_FUNCTIONS);
}

///////////////////////////////////////////

bool oxr_su_is_su_needed(scene_request_info_t info) {
	return info.occlusion || info.raycast;
}

///////////////////////////////////////////

bool oxr_su_check_needs_update(scene_request_info_t info) {
	switch (info.refresh_type) {
	case world_refresh_area:  return !vec3_in_radius(input_head().position, info.center, info.radius * 0.5f);
	case world_refresh_timer: return (time_totalf_unscaled() - local.scene_last_refresh) >= info.refresh_interval;
	default: return false;
	}
}

///////////////////////////////////////////

void xr_ext_msft_su_step_begin(void*) {
	if (oxr_su_is_su_needed(local.scene_next_req) || oxr_su_is_su_needed(local.scene_last_req)) {

		// Check if we've walked away from the current scene's center
		if (oxr_su_check_needs_update(local.scene_last_req))
			local.scene_update_requested = true;

		// Check if we need to request a new scene
		if (!local.scene_updating && local.scene_update_requested) {
			local.scene_update_requested = false;
			oxr_su_request_update(local.scene_next_req);
		}

		// If we requested a scene, check on the status
		if (local.scene_updating) {
			XrSceneComputeStateMSFT state;
			if (XR_FAILED(xrGetSceneComputeStateMSFT(local.scene_observer, &state))) {
				log_warn("xrGetSceneComputeStateMSFT failed");
			}

			if (state == XR_SCENE_COMPUTE_STATE_COMPLETED_MSFT) {
				local.scene_updating = false;
				XrSceneCreateInfoMSFT info = { (XrStructureType)XR_TYPE_SCENE_CREATE_INFO_MSFT };
				if (local.scene != XR_NULL_HANDLE)
					xrDestroySceneMSFT(local.scene);
				if (XR_FAILED(xrCreateSceneMSFT(local.scene_observer, &info, &local.scene))) {
					log_warn("xrCreateSceneMSFT failed");
				} else {
					if (local.scene_last_req.occlusion) oxr_su_load_scene_meshes(XR_SCENE_COMPONENT_TYPE_VISUAL_MESH_MSFT,   &local.scene_visuals);
					if (local.scene_last_req.raycast)   oxr_su_load_scene_meshes(XR_SCENE_COMPONENT_TYPE_COLLIDER_MESH_MSFT, &local.scene_colliders);
					oxr_su_update_meshes(&local.meshes);
				}
			} else if (state == XR_SCENE_COMPUTE_STATE_COMPLETED_WITH_ERROR_MSFT) {
				log_warn("Scene computed failed with an error!");
				local.scene_updating = false;
			} else {
			}
		}
	}

	if (local.scene_next_req.occlusion) {
		material_t mat = world_get_occlusion_material();
		if (mat) {
			matrix root = render_get_cam_final();
			for (int32_t i = 0; i < local.scene_visuals.count; i++) {
				render_add_mesh(local.scene_visuals[i].mesh_ref, mat, root * local.scene_visuals[i].local_transform);
			}
			material_release(mat);
		}
	}
}

///////////////////////////////////////////

void oxr_su_request_update(scene_request_info_t info) {
	// SU EXT requires a new scene observer any time the occlusion optimized
	// flag changes.
	bool occlusion_only      = info                .occlusion && !info                .raycast;
	bool occlusion_only_prev = local.scene_last_req.occlusion && !local.scene_last_req.raycast;
	if (occlusion_only != occlusion_only_prev) {
		if (local.scene_observer != XR_NULL_HANDLE) xrDestroySceneObserverMSFT(local.scene_observer);
		local.scene_observer = XR_NULL_HANDLE;
	}

	local.scene_last_req = info;
	if (!oxr_su_is_su_needed(info))
		return;

	pose_t head = input_head();
	local.scene_last_req.center = head.position;
	local.scene_last_refresh    = time_totalf_unscaled();

	if (local.scene_observer == XR_NULL_HANDLE) {
		XrSceneObserverCreateInfoMSFT create_info = { (XrStructureType)XR_TYPE_SCENE_OBSERVER_CREATE_INFO_MSFT };
		if (XR_FAILED(xrCreateSceneObserverMSFT(xr_session, &create_info, &local.scene_observer))) {
			log_err("xrCreateSceneObserverMSFT failed");
			return;
		}
	}
	array_t<XrSceneComputeFeatureMSFT> features = {};
	if (info.occlusion) features.add(XR_SCENE_COMPUTE_FEATURE_VISUAL_MESH_MSFT); 
	if (info.raycast  ) features.add(XR_SCENE_COMPUTE_FEATURE_COLLIDER_MESH_MSFT); 

	XrNewSceneComputeInfoMSFT compute_info = { XR_TYPE_NEW_SCENE_COMPUTE_INFO_MSFT };
	XrSceneSphereBoundMSFT    bound_sphere = { *(XrVector3f*)&head.position, info.radius };
	compute_info.consistency           = occlusion_only
		? XR_SCENE_COMPUTE_CONSISTENCY_OCCLUSION_OPTIMIZED_MSFT
		: XR_SCENE_COMPUTE_CONSISTENCY_SNAPSHOT_COMPLETE_MSFT;
	compute_info.requestedFeatures     = features.data;
	compute_info.requestedFeatureCount = (uint32_t)features.count;
	compute_info.bounds.space       = xr_app_space;
	compute_info.bounds.time        = xr_time;
	compute_info.bounds.sphereCount = 1;
	compute_info.bounds.spheres     = &bound_sphere;

	XrVisualMeshComputeLodInfoMSFT lod_info = { XR_TYPE_VISUAL_MESH_COMPUTE_LOD_INFO_MSFT };
	lod_info.lod = XR_MESH_COMPUTE_LOD_COARSE_MSFT;
	compute_info.next = &lod_info;

	XrResult result = xrComputeNewSceneMSFT(local.scene_observer, &compute_info);
	features.free();
	if (XR_FAILED(result)) {
		log_warnf("xrComputeNewSceneMSFT failed [%s]", openxr_string(result));
		return;
	}
	local.scene_updating = true;
}

///////////////////////////////////////////

void oxr_su_update_meshes(array_t<scene_mesh_t> *mesh_list) {
	for (int32_t i = 0; i < mesh_list->count; i++) {
		scene_mesh_t &mesh = mesh_list->get(i);
		if (mesh.references == 0) {
			mesh_list->remove(i);
			i--;
			continue;
		}
		if (!mesh.buffer_dirty) continue;
		mesh.buffer_dirty = false;
		mesh.references   = 0;

		XrSceneMeshBuffersMSFT        buffers   = { XR_TYPE_SCENE_MESH_BUFFERS_MSFT };
		XrSceneMeshVertexBufferMSFT   v_buffer  = { XR_TYPE_SCENE_MESH_VERTEX_BUFFER_MSFT };
		XrSceneMeshIndicesUint32MSFT  i_buffer  = { XR_TYPE_SCENE_MESH_INDICES_UINT32_MSFT };
		XrSceneMeshBuffersGetInfoMSFT mesh_info = { XR_TYPE_SCENE_MESH_BUFFERS_GET_INFO_MSFT };
		buffers  .next         = &v_buffer;
		v_buffer .next         = &i_buffer;
		mesh_info.meshBufferId = mesh.buffer_id;
		XrResult result = xrGetSceneMeshBuffersMSFT(local.scene, &mesh_info, &buffers);
		if (XR_FAILED(result)) {
			log_warnf("xrGetSceneMeshBuffersMSFT failed [%s]", openxr_string(result));
			continue;
		}
		uint32_t v_count = v_buffer.vertexCountOutput;
		uint32_t i_count = i_buffer.indexCountOutput;
		if ((uint32_t)local.vbuffer_tmp.capacity < v_count) local.vbuffer_tmp.resize((int32_t)v_count);
		if ((uint32_t)local.verts_tmp  .capacity < v_count) local.verts_tmp  .resize((int32_t)v_count);
		if ((uint32_t)local.ibuffer_tmp.capacity < i_count) local.ibuffer_tmp.resize((int32_t)i_count);
		i_buffer.indices             = local.ibuffer_tmp.data;
		v_buffer.vertices            = local.vbuffer_tmp.data;
		i_buffer.indexCapacityInput  = i_count;
		v_buffer.vertexCapacityInput = v_count;
		result = xrGetSceneMeshBuffersMSFT(local.scene, &mesh_info, &buffers);
		if (XR_FAILED(result)) {
			log_warnf("xrGetSceneMeshBuffersMSFT failed [%s]", openxr_string(result));
			continue;
		}

		if (mesh.mesh == nullptr) mesh.mesh = mesh_create();
		for (uint32_t v = 0; v < v_count; v++) {
			local.verts_tmp[v] = { *(vec3 *)&v_buffer.vertices[v], {0,1,0}, {}, {255,255,255,255} };
		}
		mesh_calculate_normals(local.verts_tmp.data, v_count, i_buffer.indices, i_count);
		mesh_set_keep_data(mesh.mesh, local.scene_last_req.raycast);
		mesh_set_data     (mesh.mesh, local.verts_tmp.data, v_count, i_buffer.indices, i_count);
	}
}
///////////////////////////////////////////

int32_t oxr_su_mesh_get_or_add(uint64_t buffer_id) {
	for (int32_t i = 0; i < local.meshes.count; i++) {
		if (local.meshes[i].buffer_id == buffer_id) {
			local.meshes[i].references += 1;
			return i;
		}
	}
	return local.meshes.add(scene_mesh_t{ buffer_id, mesh_create(), 1 });
}


///////////////////////////////////////////

void oxr_su_load_scene_meshes(XrSceneComponentTypeMSFT type, array_t<su_mesh_inst_t> *mesh_list) {
	XrSceneComponentsGetInfoMSFT info       = { XR_TYPE_SCENE_COMPONENTS_GET_INFO_MSFT };
	XrSceneComponentsMSFT        components = { XR_TYPE_SCENE_COMPONENTS_MSFT };
	info.componentType = type;
	if (XR_FAILED(xrGetSceneComponentsMSFT(local.scene, &info, &components))) {
		log_warn("xrGetSceneComponentsMSFT failed");
		return;
	}
	XrSceneMeshesMSFT meshes = { XR_TYPE_SCENE_MESHES_MSFT };
	meshes.sceneMeshCount             = components.componentCountOutput;
	meshes.sceneMeshes                = sk_malloc_t(XrSceneMeshMSFT, components.componentCountOutput);
	components.next                   = &meshes;
	components.componentCapacityInput = components.componentCountOutput;
	components.components             = sk_malloc_t(XrSceneComponentMSFT, components.componentCountOutput);
	if (XR_FAILED(xrGetSceneComponentsMSFT(local.scene, &info, &components))) {
		log_warn("xrGetSceneComponentsMSFT failed");
		sk_free(components.components);
		sk_free(meshes.sceneMeshes);
		return;
	}

	// Find the location of these meshes
	XrUuidMSFT *component_ids = sk_malloc_t(XrUuidMSFT, components.componentCountOutput);
	for (uint32_t i = 0; i < components.componentCountOutput; i++) {
		component_ids[i] = components.components[i].id;
	}
	XrSceneComponentLocationsMSFT   locations   = { XR_TYPE_SCENE_COMPONENT_LOCATIONS_MSFT };
	XrSceneComponentsLocateInfoMSFT locate_info = { XR_TYPE_SCENE_COMPONENTS_LOCATE_INFO_MSFT };
	locations  .locationCount    = components.componentCountOutput;
	locations  .locations        = sk_malloc_t(XrSceneComponentLocationMSFT, locations.locationCount);
	locate_info.baseSpace        = xr_app_space;
	locate_info.time             = xr_time;
	locate_info.componentIdCount = components.componentCountOutput;
	locate_info.componentIds     = component_ids;
	xrLocateSceneComponentsMSFT(local.scene, &locate_info, &locations);
	sk_free(component_ids);

	mesh_list->clear();
	if (mesh_list->capacity < (int32_t)meshes.sceneMeshCount)
		mesh_list->resize(meshes.sceneMeshCount);

	for (uint32_t i = 0; i < meshes.sceneMeshCount; i++) {
		pose_t pose = { vec3_zero, quat_identity };
		if (locations.locations[i].flags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
			pose.position = *(vec3 *)&locations.locations[i].pose.position;
		if (locations.locations[i].flags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)
			pose.orientation = *(quat *)&locations.locations[i].pose.orientation;

		int32_t        mesh_idx = oxr_su_mesh_get_or_add(meshes.sceneMeshes[i].meshBufferId);
		su_mesh_inst_t inst     = {};
		inst.mesh_ref            = local.meshes[mesh_idx].mesh;
		inst.local_transform     = pose_matrix    (pose);
		inst.local_transform_inv = pose_matrix_inv(pose);
		if (local.meshes[mesh_idx].buffer_updated != components.components[i].updateTime ||
			(!mesh_get_keep_data(local.meshes[mesh_idx].mesh) && local.scene_last_req.raycast)) {
			local.meshes[mesh_idx].buffer_updated  = components.components[i].updateTime;
			local.meshes[mesh_idx].buffer_dirty    = true;
		}
		mesh_list->add(inst);
	}

	sk_free(locations.locations);
	sk_free(components.components);
	sk_free(meshes.sceneMeshes);
}

///////////////////////////////////////////

bool32_t oxr_su_raycast(ray_t ray_world, ray_t *out_intersection) {
	if (!local.scene_next_req.raycast) return false;

	ray_t   ray_camera = matrix_transform_ray(render_get_cam_final_inv(), ray_world);

	ray_t   result     = {};
	float   result_mag = FLT_MAX;
	int32_t result_id  = -1;
	for (int32_t i = 0; i < local.scene_colliders.count; i++) {
		ray_t intersection = {};
		ray_t local_ray    = matrix_transform_ray(local.scene_colliders[i].local_transform_inv, ray_camera);
		if (mesh_ray_intersect(local.scene_colliders[i].mesh_ref, local_ray, cull_back, &intersection)) {
			float intersection_mag = vec3_magnitude_sq(intersection.pos - local_ray.pos);
			if (result_mag > intersection_mag) {
				result_mag        = intersection_mag;
				*out_intersection = intersection;
				result_id         = i;
			}
		}
	}
	if (result_id != -1) {
		*out_intersection = matrix_transform_ray(local.scene_colliders[result_id].local_transform, *out_intersection);
		*out_intersection = matrix_transform_ray(render_get_cam_final(), *out_intersection);
		return true;
	} else {
		return false;
	}
}

///////////////////////////////////////////

void oxr_su_set_occlusion_enabled(bool32_t enabled) {
	enabled = sk_get_info_ref()->world_occlusion_present > 0 && enabled > 0;
	if (local.scene_next_req.occlusion == enabled) return;

	local.scene_next_req.occlusion = enabled;
	if (enabled && local.scene_next_req.occlusion != local.scene_last_req.occlusion)
		local.scene_update_requested = true;
}

///////////////////////////////////////////

bool32_t oxr_su_get_occlusion_enabled() {
	return local.scene_next_req.occlusion;
}

///////////////////////////////////////////

void oxr_su_set_raycast_enabled(bool32_t enabled) {
	enabled = sk_get_info_ref()->world_raycast_present > 0 && enabled > 0;
	if (local.scene_next_req.raycast == enabled) return;

	local.scene_next_req.raycast = enabled;
	if (enabled && local.scene_next_req.raycast != local.scene_last_req.raycast)
		local.scene_update_requested = true;
}

///////////////////////////////////////////

bool32_t oxr_su_get_raycast_enabled() {
	return local.scene_next_req.raycast;
}

///////////////////////////////////////////

void oxr_su_set_refresh_type(world_refresh_ refresh_type) {
	local.scene_next_req.refresh_type = refresh_type;
	if (local.scene_next_req.refresh_type != local.scene_last_req.refresh_type) {
		local.scene_update_requested = true;
	}
}

///////////////////////////////////////////

world_refresh_ oxr_su_get_refresh_type() {
	return local.scene_next_req.refresh_type;
}

///////////////////////////////////////////

void oxr_su_set_refresh_radius(float radius_meters) {
	local.scene_next_req.radius = radius_meters;
	if (local.scene_next_req.radius != local.scene_last_req.radius) {
		local.scene_update_requested = true;
	}
}

///////////////////////////////////////////

float oxr_su_get_refresh_radius() {
	return local.scene_next_req.radius;
}

///////////////////////////////////////////

void oxr_su_set_refresh_interval(float every_seconds) {
	local.scene_next_req.refresh_interval = every_seconds;
	if (local.scene_next_req.refresh_interval != local.scene_last_req.refresh_interval) {
		local.scene_update_requested = true;
	}
}

///////////////////////////////////////////

float oxr_su_get_refresh_interval() {
	return local.scene_next_req.refresh_interval;
}

///////////////////////////////////////////

} // namespace sk