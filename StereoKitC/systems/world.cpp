#include "../platforms/platform_utils.h"

#include "../stereokit.h"
#include "../_stereokit.h"
#include "../sk_memory.h"
#include "../asset_types/assets.h"
#include "../asset_types/mesh_.h"
#include "../xr_backends/openxr.h"
#include "../xr_backends/openxr_extensions.h"
#include "render.h"

#include <float.h>

namespace sk {

#if defined(SK_XR_OPENXR)

///////////////////////////////////////////

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
	matrix   transform;
	matrix   inv_transform;
};

XrSceneObserverMSFT     xr_scene_observer         = {};
bool                    xr_scene_updating         = false;
bool                    xr_scene_update_requested = false;
XrSceneMSFT             xr_scene;
scene_request_info_t    xr_scene_last_req         = {};
scene_request_info_t    xr_scene_next_req         = {};
float                   xr_scene_last_refresh     = -1000;

array_t<scene_mesh_t>   xr_meshes                 = {};
array_t<su_mesh_inst_t> xr_scene_colliders        = {};
array_t<su_mesh_inst_t> xr_scene_visuals          = {};
material_t              xr_scene_material         = {};

array_t<vert_t>         world_verts_tmp           = {};
array_t<XrVector3f>     world_vbuffer_tmp         = {};
array_t<uint32_t>       world_ibuffer_tmp         = {};

///////////////////////////////////////////

bool world_is_su_needed      (scene_request_info_t info);
bool world_check_needs_update(scene_request_info_t info);
void world_scene_shutdown    ();
void world_load_scene_meshes (XrSceneComponentTypeMSFT type, array_t<scene_mesh_t> *mesh_list);

///////////////////////////////////////////

bool world_is_su_needed(scene_request_info_t info) {
	return info.occlusion || info.raycast;
}

///////////////////////////////////////////

bool world_check_needs_update(scene_request_info_t info) {
	switch (info.refresh_type) {
	case world_refresh_area:  return !vec3_in_radius(input_head()->position, info.center, info.radius * 0.5f);
	case world_refresh_timer: return (time_getf() - xr_scene_last_refresh) >= info.refresh_interval;
	default: return false;
	}
}

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

bool32_t world_raycast(ray_t ray, ray_t *out_intersection) {
	if (!xr_scene_next_req.raycast) return false;

	ray_t   result     = {};
	float   result_mag = FLT_MAX;
	int32_t result_id  = -1;
	for (int32_t i = 0; i < xr_scene_colliders.count; i++) {
		ray_t intersection = {};
		ray_t local_ray    = matrix_transform_ray(xr_scene_colliders[i].inv_transform, ray);
		if (mesh_ray_intersect(xr_scene_colliders[i].mesh_ref, local_ray, &intersection)) {
			float intersection_mag = vec3_magnitude_sq(intersection.pos - local_ray.pos);
			if (result_mag > intersection_mag) {
				result_mag        = intersection_mag;
				*out_intersection = intersection;
				result_id         = i;
			}
		}
	}
	if (result_id != -1) {
		*out_intersection = matrix_transform_ray(xr_scene_colliders[result_id].transform, *out_intersection);
		return true;
	} else {
		return false;
	}
}

///////////////////////////////////////////

void world_set_occlusion_enabled(bool32_t enabled) {
	enabled = sk_info.world_occlusion_present && enabled;
	if (xr_scene_next_req.occlusion == enabled) return;

	xr_scene_next_req.occlusion = enabled;
	if (enabled && xr_scene_next_req.occlusion != xr_scene_last_req.occlusion)
		xr_scene_update_requested = true;
}

///////////////////////////////////////////

bool32_t world_get_occlusion_enabled() {
	return xr_scene_next_req.occlusion;
}

///////////////////////////////////////////

void world_set_raycast_enabled(bool32_t enabled) {
	enabled = sk_info.world_raycast_present && enabled;
	if (xr_scene_next_req.raycast == enabled) return;

	xr_scene_next_req.raycast = enabled;
	if (enabled && xr_scene_next_req.raycast != xr_scene_last_req.raycast)
		xr_scene_update_requested = true;
}

///////////////////////////////////////////

bool32_t world_get_raycast_enabled() {
	return xr_scene_next_req.raycast;
}

///////////////////////////////////////////

void world_set_occlusion_material(material_t material) {
	assets_safeswap_ref(
		(asset_header_t**)&xr_scene_material,
		(asset_header_t *)material);
}

///////////////////////////////////////////

material_t world_get_occlusion_material() {
	if (xr_scene_material)
		material_addref(xr_scene_material);
	return xr_scene_material;
}

///////////////////////////////////////////

void world_set_refresh_type(world_refresh_ refresh_type) {
	xr_scene_next_req.refresh_type = refresh_type;
	if (xr_scene_next_req.refresh_type != xr_scene_last_req.refresh_type) {
		xr_scene_update_requested = true;
	}
}

///////////////////////////////////////////

world_refresh_ world_get_refresh_type() {
	return xr_scene_next_req.refresh_type;
}

///////////////////////////////////////////

void world_set_refresh_radius(float radius_meters) {
	xr_scene_next_req.radius = radius_meters;
	if (xr_scene_next_req.radius != xr_scene_last_req.radius) {
		xr_scene_update_requested = true;
	}
}

///////////////////////////////////////////

float world_get_refresh_radius() {
	return xr_scene_next_req.radius;
}

///////////////////////////////////////////

void world_set_refresh_interval(float every_seconds) {
	xr_scene_next_req.refresh_interval = every_seconds;
	if (xr_scene_next_req.refresh_interval != xr_scene_last_req.refresh_interval) {
		xr_scene_update_requested = true;
	}
}

///////////////////////////////////////////

float world_get_refresh_interval() {
	return xr_scene_next_req.refresh_interval;
}

///////////////////////////////////////////

void world_request_update(scene_request_info_t info) {
	// SU EXT requires a new scene observer any time the occlusion optimized
	// flag changes.
	bool occlusion_only      = info             .occlusion && !info             .raycast;
	bool occlusion_only_prev = xr_scene_last_req.occlusion && !xr_scene_last_req.raycast;
	if (occlusion_only != occlusion_only_prev) {
		if (xr_scene_observer != XR_NULL_HANDLE) xr_extensions.xrDestroySceneObserverMSFT(xr_scene_observer);
		xr_scene_observer = XR_NULL_HANDLE;
	}

	xr_scene_last_req = info;
	if (!world_is_su_needed(info))
		return;

	xr_scene_last_req.center = input_head()->position;
	xr_scene_last_refresh    = time_getf();

	if (xr_scene_observer == XR_NULL_HANDLE) {
		XrSceneObserverCreateInfoMSFT create_info = { (XrStructureType)XR_TYPE_SCENE_OBSERVER_CREATE_INFO_MSFT };
		if (XR_FAILED(xr_extensions.xrCreateSceneObserverMSFT(xr_session, &create_info, &xr_scene_observer))) {
			log_err("xrCreateSceneObserverMSFT failed");
			return;
		}
	}
	array_t<XrSceneComputeFeatureMSFT> features = {};
	if (info.occlusion) features.add(XR_SCENE_COMPUTE_FEATURE_VISUAL_MESH_MSFT); 
	if (info.raycast  ) features.add(XR_SCENE_COMPUTE_FEATURE_COLLIDER_MESH_MSFT); 

	XrNewSceneComputeInfoMSFT compute_info = { XR_TYPE_NEW_SCENE_COMPUTE_INFO_MSFT };
	XrSceneSphereBoundMSFT    bound_sphere = { *(XrVector3f*)&input_head()->position, info.radius };
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

	XrResult result = xr_extensions.xrComputeNewSceneMSFT(xr_scene_observer, &compute_info);
	features.free();
	if (XR_FAILED(result)) {
		log_warnf("xrComputeNewSceneMSFT failed [%s]", openxr_string(result));
		return;
	}
	xr_scene_updating = true;
}

///////////////////////////////////////////

void world_update_meshes(array_t<scene_mesh_t> *mesh_list) {
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
		XrResult result = xr_extensions.xrGetSceneMeshBuffersMSFT(xr_scene, &mesh_info, &buffers);
		if (XR_FAILED(result)) {
			log_warnf("xrGetSceneMeshBuffersMSFT failed [%s]", openxr_string(result));
			continue;
		}
		uint32_t v_count = v_buffer.vertexCountOutput;
		uint32_t i_count = i_buffer.indexCountOutput;
		if ((uint32_t)world_vbuffer_tmp.capacity < v_count) world_vbuffer_tmp.resize((int32_t)v_count);
		if ((uint32_t)world_verts_tmp  .capacity < v_count) world_verts_tmp  .resize((int32_t)v_count);
		if ((uint32_t)world_ibuffer_tmp.capacity < i_count) world_ibuffer_tmp.resize((int32_t)i_count);
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
		for (uint32_t v = 0; v < v_count; v++) {
			world_verts_tmp[v] = { *(vec3 *)&v_buffer.vertices[v], {0,1,0}, {}, {255,255,255,255} };
		}
		mesh_calculate_normals(world_verts_tmp.data, v_count, i_buffer.indices, i_count);
		mesh_set_keep_data(mesh.mesh, xr_scene_last_req.raycast);
		mesh_set_inds (mesh.mesh, i_buffer.indices,     i_count);
		mesh_set_verts(mesh.mesh, world_verts_tmp.data, v_count);
	}
}

///////////////////////////////////////////

int32_t world_mesh_get_or_add(uint64_t buffer_id) {
	for (int32_t i = 0; i < xr_meshes.count; i++) {
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
	xr_extensions.xrLocateSceneComponentsMSFT(xr_scene, &locate_info, &locations);
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

		int32_t        mesh_idx = world_mesh_get_or_add(meshes.sceneMeshes[i].meshBufferId);
		su_mesh_inst_t inst     = {};
		inst.mesh_ref        = xr_meshes[mesh_idx].mesh;
		inst.local_transform = pose_matrix(pose);
		inst.transform       = inst.local_transform * render_get_cam_final();
		inst.inv_transform   = matrix_invert(inst.transform);
		if (xr_meshes[mesh_idx].buffer_updated != components.components[i].updateTime ||
			(!mesh_get_keep_data(xr_meshes[mesh_idx].mesh) && xr_scene_last_req.raycast)) {
			xr_meshes[mesh_idx].buffer_updated  = components.components[i].updateTime;
			xr_meshes[mesh_idx].buffer_dirty    = true;
		}
		mesh_list->add(inst);
	}

	sk_free(locations.locations);
	sk_free(components.components);
	sk_free(meshes.sceneMeshes);
}

///////////////////////////////////////////

bool world_init() {
	xr_scene_material = material_copy_id(default_id_material_unlit);
	material_set_id   (xr_scene_material, "default/world_mat");
	material_set_color(xr_scene_material, "color", { 0,0,0,0 });

	xr_scene_next_req.center        = { 10000,10000,10000 };
	xr_scene_next_req.radius        = 4;
	xr_scene_next_req.refresh_type  = world_refresh_area;
	xr_scene_next_req.refresh_interval = 1.5f;

	xr_scene_last_req = xr_scene_next_req;

	return true;
}

///////////////////////////////////////////

void world_update() {
	if (world_is_su_needed(xr_scene_next_req) || world_is_su_needed(xr_scene_last_req)) {

		// Check if we've walked away from the current scene's center
		if (world_check_needs_update(xr_scene_last_req))
			xr_scene_update_requested = true;

		// Check if we need to request a new scene
		if (!xr_scene_updating && xr_scene_update_requested) {
			xr_scene_update_requested = false;
			world_request_update(xr_scene_next_req);
		}

		// If we requested a scene, check on the status
		if (xr_scene_updating) {
			XrSceneComputeStateMSFT state;
			if (XR_FAILED(xr_extensions.xrGetSceneComputeStateMSFT(xr_scene_observer, &state))) {
				log_warn("xrGetSceneComputeStateMSFT failed");
			}

			if (state == XR_SCENE_COMPUTE_STATE_COMPLETED_MSFT) {
				xr_scene_updating = false;
				XrSceneCreateInfoMSFT info = { (XrStructureType)XR_TYPE_SCENE_CREATE_INFO_MSFT };
				if (xr_scene != XR_NULL_HANDLE)
					xr_extensions.xrDestroySceneMSFT(xr_scene);
				if (XR_FAILED(xr_extensions.xrCreateSceneMSFT(xr_scene_observer, &info, &xr_scene))) {
					log_warn("xrCreateSceneMSFT failed");
				} else {
					if (xr_scene_last_req.occlusion) world_load_scene_meshes(XR_SCENE_COMPONENT_TYPE_VISUAL_MESH_MSFT,   &xr_scene_visuals);
					if (xr_scene_last_req.raycast)   world_load_scene_meshes(XR_SCENE_COMPONENT_TYPE_COLLIDER_MESH_MSFT, &xr_scene_colliders);
					world_update_meshes(&xr_meshes);
				}
			} else if (state == XR_SCENE_COMPUTE_STATE_COMPLETED_WITH_ERROR_MSFT) {
				log_warn("Scene computed failed with an error!");
				xr_scene_updating = false;
			} else {
			}
		}
	}

	if (xr_scene_next_req.occlusion && xr_scene_material != nullptr) {
		for (int32_t i = 0; i < xr_scene_visuals.count; i++) {
			render_add_mesh(xr_scene_visuals[i].mesh_ref, xr_scene_material, xr_scene_visuals[i].transform);
		}
	}
}

///////////////////////////////////////////

void world_scene_shutdown() {
	if (xr_scene          != XR_NULL_HANDLE) xr_extensions.xrDestroySceneMSFT        (xr_scene);
	if (xr_scene_observer != XR_NULL_HANDLE) xr_extensions.xrDestroySceneObserverMSFT(xr_scene_observer);

	xr_meshes.each([](scene_mesh_t &m) { mesh_release(m.mesh); });
	xr_meshes         .clear();
	xr_scene_colliders.clear();
	xr_scene_visuals  .clear();

	material_release(xr_scene_material);

	xr_scene_updating = false;
}

///////////////////////////////////////////

void world_shutdown() {
	world_scene_shutdown();

	world_verts_tmp  .free();
	world_vbuffer_tmp.free();
	world_ibuffer_tmp.free();

	xr_meshes         .free();
	xr_scene_colliders.free();
	xr_scene_visuals  .free();
}

///////////////////////////////////////////

inline void world_update_inst(su_mesh_inst_t &inst) {
	inst.transform     = inst.local_transform * render_get_cam_final();
	inst.inv_transform = matrix_invert(inst.transform);
}

///////////////////////////////////////////

void world_refresh_transforms() {
	xr_scene_colliders.each(world_update_inst);
	xr_scene_visuals  .each(world_update_inst);
	xr_bounds_pose = matrix_transform_pose(render_get_cam_final(), xr_bounds_pose_local);
}

#else

bool world_init() {
	return true;
}

void world_update() {
}

void world_shutdown() {
}

void world_refresh_transforms() {
}

bool32_t world_raycast(ray_t ray, ray_t *out_intersection) {
	return false;
}

void world_set_occlusion_enabled(bool32_t enabled) {
}

bool32_t world_get_occlusion_enabled() {
	return false;
}

void world_set_raycast_enabled(bool32_t enabled) {
}

bool32_t world_get_raycast_enabled() {
	return false;
}

void world_set_occlusion_material(material_t material) {
}

material_t world_get_occlusion_material() {
	return nullptr;
}

void world_set_refresh_type(world_refresh_ refresh_type) {
}

world_refresh_ world_get_refresh_type() {
	return world_refresh_area;
}

void world_set_refresh_radius(float radius_meters) {
}

float world_get_refresh_radius() {
	return 0;
}

void world_set_refresh_interval(float every_seconds) {
}

float world_get_refresh_interval() {
	return 0;
}


///////////////////////////////////////////

bool32_t world_has_bounds() {
	return false;
}

///////////////////////////////////////////

vec2 world_get_bounds_size() {
	return vec2_zero;
}

///////////////////////////////////////////

pose_t world_get_bounds_pose() {
	return pose_identity;
}

#endif

} // namespace sk