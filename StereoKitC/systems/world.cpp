#include "../stereokit.h"
#include "../_stereokit.h"
#include "../sk_memory.h"
#include "../asset_types/mesh.h"
#include "platform/openxr.h"

#include <float.h>

namespace sk {

///////////////////////////////////////////

XrSceneObserverMSFT xr_scene_observer    = {};
XrSceneMSFT         xr_scene             = {};
bool                xr_scene_updating    = false;
mesh_t              xr_scene_mesh        = nullptr;
vec3                xr_scene_last_center = { 1000000,1000000,100000 };
float               xr_scene_radius      = 4;

struct scene_mesh_t {
	uint64_t buffer_id;
	mesh_t   mesh;
	int32_t  references;
	bool     buffer_dirty;
	XrTime   buffer_updated;
};

struct su_mesh_inst_t {
	mesh_t   mesh_ref;
	matrix   transform;
	matrix   inv_transform;
};

array_t<scene_mesh_t>   xr_meshes         = {};
array_t<su_mesh_inst_t> xr_scene_colliders= {};
array_t<su_mesh_inst_t> xr_scene_visuals  = {};
material_t              xr_scene_material = {};

array_t<vert_t>     world_verts_tmp   = {};
array_t<XrVector3f> world_vbuffer_tmp = {};
array_t<uint32_t>   world_ibuffer_tmp = {};

///////////////////////////////////////////

void world_load_scene_meshes(XrSceneComponentTypeMSFT type, array_t<scene_mesh_t> *mesh_list);

///////////////////////////////////////////

bool32_t world_has_bounds() {
	return xr_has_bounds;
}

///////////////////////////////////////////

vec2 world_get_bounds_size() {
	return xr_bounds_size;
}

///////////////////////////////////////////

pose_t world_get_bounds_pose() { 
	return xr_bounds_pose; 
}

///////////////////////////////////////////

bool32_t world_raycast(ray_t ray, ray_t &out_intersection) {
	ray_t   result     = {};
	float   result_mag = FLT_MAX;
	int32_t result_id  = -1;
	for (size_t i = 0; i < xr_scene_colliders.count; i++) {
		ray_t intersection = {};
		ray_t local_ray;
		local_ray.pos = matrix_mul_point    (xr_scene_colliders[i].inv_transform, ray.pos);
		local_ray.dir = matrix_mul_direction(xr_scene_colliders[i].inv_transform, ray.dir);
		if (mesh_ray_intersect(xr_scene_colliders[i].mesh_ref, local_ray, &intersection.pos)) {
			float intersection_mag = vec3_magnitude_sq(intersection.pos - local_ray.pos);
			if (result_mag > intersection_mag) {
				result_mag       = intersection_mag;
				out_intersection = intersection;
				result_id        = i;
			}
		}
	}
	if (result_id != -1) {
		out_intersection.pos = matrix_mul_point    (xr_scene_colliders[result_id].transform, out_intersection.pos);
		out_intersection.dir = matrix_mul_direction(xr_scene_colliders[result_id].transform, out_intersection.dir);
		return true;
	} else {
		return false;
	}
}

///////////////////////////////////////////

void world_request_update() {
	log_info("Requesting scene update...");

	XrSceneComputeFeatureMSFT features[] = {
		XR_SCENE_COMPUTE_FEATURE_VISUAL_MESH_MSFT,
		XR_SCENE_COMPUTE_FEATURE_PLANE_MSFT,
		XR_SCENE_COMPUTE_FEATURE_COLLIDER_MESH_MSFT 
	};

	XrNewSceneComputeInfoMSFT compute_info = { XR_TYPE_NEW_SCENE_COMPUTE_INFO_MSFT };
	XrSceneSphereBoundMSFT    bound_sphere = { *(XrVector3f*)&input_head()->position, xr_scene_radius };
	compute_info.consistency           = XR_SCENE_COMPUTE_CONSISTENCY_SNAPSHOT_COMPLETE_MSFT;
	compute_info.requestedFeatures     = features;
	compute_info.requestedFeatureCount = _countof(features);
	compute_info.bounds.space       = xr_app_space;
	compute_info.bounds.time        = xr_time;
	compute_info.bounds.sphereCount = 1;
	compute_info.bounds.spheres     = &bound_sphere;

	XrVisualMeshComputeLodInfoMSFT lod_info = { XR_TYPE_VISUAL_MESH_COMPUTE_LOD_INFO_MSFT };
	lod_info.lod = XR_MESH_COMPUTE_LOD_COARSE_MSFT;
	compute_info.next = &lod_info;

	XrResult result = xr_extensions.xrComputeNewSceneMSFT(xr_scene_observer, &compute_info);
	if (XR_FAILED(result)) {
		log_warnf("xrComputeNewSceneMSFT failed [%s]", openxr_string(result));
		return;
	}

	log_info("Requested scene update!");
	xr_scene_updating = true;
}

///////////////////////////////////////////

void world_update_meshes(array_t<scene_mesh_t> *mesh_list) {
	for (size_t i = 0; i < mesh_list->count; i++) {
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
		XrResult result = xr_extensions.xrGetSceneMeshBuffersMSFT(xr_scene, &mesh_info, &buffers);
		if (XR_FAILED(result)) {
			log_warnf("xrGetSceneMeshBuffersMSFT failed [%s]", openxr_string(result));
			continue;
		}
		uint32_t v_count = v_buffer.vertexCountOutput;
		uint32_t i_count = i_buffer.indexCountOutput;
		if (world_vbuffer_tmp.capacity < v_count) world_vbuffer_tmp.resize(v_count);
		if (world_verts_tmp  .capacity < v_count) world_verts_tmp  .resize(v_count);
		if (world_ibuffer_tmp.capacity < i_count) world_ibuffer_tmp.resize(i_count);
		i_buffer.indices             = world_ibuffer_tmp.data;
		v_buffer.vertices            = world_vbuffer_tmp.data;
		i_buffer.indexCapacityInput  = i_count;
		v_buffer.vertexCapacityInput = v_count;
		result = xr_extensions.xrGetSceneMeshBuffersMSFT(xr_scene, &mesh_info, &buffers);
		if (XR_FAILED(result)) {
			log_warnf("xrGetSceneMeshBuffersMSFT failed [%s]", openxr_string(result));
			continue;
		}

		if (mesh.mesh == nullptr) mesh.mesh = mesh_create();
		for (size_t v = 0; v < v_count; v++) {
			world_verts_tmp[v] = { *(vec3 *)&v_buffer.vertices[v], {0,1,0}, {}, {255,255,255,255} };
		}
		mesh_calculate_normals(world_verts_tmp.data, v_count, i_buffer.indices, i_count);
		mesh_set_inds (mesh.mesh, i_buffer.indices,     i_count);
		mesh_set_verts(mesh.mesh, world_verts_tmp.data, v_count);
	}
}

///////////////////////////////////////////

int32_t world_mesh_get_or_add(uint64_t buffer_id) {
	for (size_t i = 0; i < xr_meshes.count; i++) {
		if (xr_meshes[i].buffer_id == buffer_id) {
			xr_meshes[i].references += 1;
			return i;
		}
	}
	return xr_meshes.add(scene_mesh_t{ buffer_id, mesh_create(), 1 });
}

///////////////////////////////////////////

void world_load_scene_meshes(XrSceneComponentTypeMSFT type, array_t<su_mesh_inst_t> *mesh_list) {
	XrSceneComponentsGetInfoMSFT info       = { XR_TYPE_SCENE_COMPONENTS_GET_INFO_MSFT };
	XrSceneComponentsMSFT        components = { XR_TYPE_SCENE_COMPONENTS_MSFT };
	info.componentType = type;
	if (XR_FAILED(xr_extensions.xrGetSceneComponentsMSFT(xr_scene, &info, &components))) {
		log_warn("xrGetSceneComponentsMSFT failed");
		return;
	}
	XrSceneMeshesMSFT meshes = { XR_TYPE_SCENE_MESHES_MSFT };
	meshes.sceneMeshCount             = components.componentCountOutput;
	meshes.sceneMeshes                = sk_malloc_t(XrSceneMeshMSFT, components.componentCountOutput);
	components.next                   = &meshes;
	components.componentCapacityInput = components.componentCountOutput;
	components.components             = sk_malloc_t(XrSceneComponentMSFT, components.componentCountOutput);
	if (XR_FAILED(xr_extensions.xrGetSceneComponentsMSFT(xr_scene, &info, &components))) {
		log_warn("xrGetSceneComponentsMSFT failed");
		free(components.components);
		free(meshes.sceneMeshes);
		return;
	}

	// Find the location of these meshes
	XrUuidMSFT *component_ids = sk_malloc_t(XrUuidMSFT, components.componentCountOutput);
	for (size_t i = 0; i < components.componentCountOutput; i++) {
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
	xr_extensions.xrLocateSceneComponentsMSFT(xr_scene, &locate_info, &locations);
	free(component_ids);

	mesh_list->clear();
	if (mesh_list->capacity < meshes.sceneMeshCount)
		mesh_list->resize(meshes.sceneMeshCount);

	for (size_t i = 0; i < meshes.sceneMeshCount; i++) {
		pose_t pose = { vec3_zero, quat_identity };
		if (locations.locations[i].flags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
			pose.position = *(vec3 *)&locations.locations[i].pose.position;
		if (locations.locations[i].flags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)
			pose.orientation = *(quat *)&locations.locations[i].pose.orientation;

		int32_t        mesh_idx = world_mesh_get_or_add(meshes.sceneMeshes[i].meshBufferId);
		su_mesh_inst_t inst     = {};
		inst.mesh_ref      = xr_meshes[mesh_idx].mesh;
		inst.transform     = pose_matrix(pose);
		inst.inv_transform = matrix_invert(inst.transform);
		if (xr_meshes[mesh_idx].buffer_updated != components.components[i].updateTime) {
			xr_meshes[mesh_idx].buffer_updated  = components.components[i].updateTime;
			xr_meshes[mesh_idx].buffer_dirty    = true;
		}
	}

	free(locations.locations);
	free(components.components);
	free(meshes.sceneMeshes);
}

///////////////////////////////////////////

bool world_init() {
	if (xr_ext_available.MSFT_scene_understanding) {
		XrSceneObserverCreateInfoMSFT create_info = { (XrStructureType)XR_TYPE_SCENE_OBSERVER_CREATE_INFO_MSFT };
		if (XR_FAILED(xr_extensions.xrCreateSceneObserverMSFT(xr_session, &create_info, &xr_scene_observer))) {
			log_err("xrCreateSceneObserverMSFT failed");
			return false;
		}
		log_info("Scene observer created!");
	}

	xr_scene_material = material_copy_id(default_id_material);
	material_set_wireframe(xr_scene_material, true);

	return true;
}

///////////////////////////////////////////

void world_update() {
	if (xr_ext_available.MSFT_scene_understanding) {

		if (xr_time > 0 && !xr_scene_updating && vec3_magnitude_sq(input_head()->position - xr_scene_last_center) > xr_scene_radius * xr_scene_radius) {
			xr_scene_last_center = input_head()->position;
			world_request_update();
		}

		if (xr_scene_updating) {
			XrSceneComputeStateMSFT state;
			if (XR_FAILED(xr_extensions.xrGetSceneComputeStateMSFT(xr_scene_observer, &state))) {
				log_warn("xrGetSceneComputeStateMSFT failed");
			}

			if (state == XR_SCENE_COMPUTE_STATE_COMPLETED_MSFT) {
				log_info("Scene computed!");

				xr_scene_updating = false;
				XrSceneCreateInfoMSFT info = { (XrStructureType)XR_TYPE_SCENE_CREATE_INFO_MSFT };
				if (xr_scene != XR_NULL_HANDLE)
					xr_extensions.xrDestroySceneMSFT(xr_scene);
				if (XR_FAILED(xr_extensions.xrCreateSceneMSFT(xr_scene_observer, &info, &xr_scene))) {
					log_warn("xrCreateSceneMSFT failed");
				} else {
					world_load_scene_meshes(XR_SCENE_COMPONENT_TYPE_VISUAL_MESH_MSFT,   &xr_scene_visuals);
					world_load_scene_meshes(XR_SCENE_COMPONENT_TYPE_COLLIDER_MESH_MSFT, &xr_scene_colliders);
					world_update_meshes    (&xr_meshes);
				}
			} else if (state == XR_SCENE_COMPUTE_STATE_COMPLETED_WITH_ERROR_MSFT) {
				log_warn("Scene computed failed with an error!");
				xr_scene_updating = false;
			} else {
			}
		}
	}

	for (size_t i = 0; i < xr_scene_visuals.count; i++) {
		render_add_mesh(xr_scene_visuals[i].mesh_ref, xr_scene_material, xr_scene_visuals[i].transform, color_hsv((i % 8) / 8.0f, 0.8f, 0.8f, 1));
	}
	for (size_t i = 0; i < xr_scene_colliders.count; i++) {
		render_add_mesh(xr_scene_colliders[i].mesh_ref, xr_scene_material, xr_scene_colliders[i].transform, color_hsv(((i+4) % 8) / 8.0f, 0.4f, 0.8f, 1));
	}
}

///////////////////////////////////////////

void world_shutdown() {
	if (xr_ext_available.MSFT_scene_understanding) {
		xr_extensions.xrDestroySceneObserverMSFT(xr_scene_observer);
	}
	world_verts_tmp  .free();
	world_vbuffer_tmp.free();
	world_ibuffer_tmp.free();
}

} // namespace sk
