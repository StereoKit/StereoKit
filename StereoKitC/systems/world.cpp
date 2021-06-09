#include "../stereokit.h"
#include "../_stereokit.h"
#include "../sk_memory.h"
#include "../asset_types/mesh.h"
#include "platform/openxr.h"

namespace sk {

///////////////////////////////////////////

XrSceneObserverMSFT xr_scene_observer    = {};
XrSceneMSFT         xr_scene             = {};
bool                xr_scene_updating    = false;
mesh_t              xr_scene_mesh        = nullptr;
vec3                xr_scene_last_center = { 1000000,1000000,100000 };
float               xr_scene_radius      = 4;

struct scene_mesh_t {
	mesh_t mesh;
	matrix transform;
};
array_t<scene_mesh_t> xr_scene_meshes   = {};
material_t            xr_scene_material = {};

///////////////////////////////////////////


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

	if (XR_FAILED(xr_extensions.xrComputeNewSceneMSFT(xr_scene_observer, &compute_info))) {
		log_warn("xrComputeNewSceneMSFT failed");
		return;
	}

	log_info("Requested scene update!");
	xr_scene_updating = true;
}

///////////////////////////////////////////

void world_update_scene_meshes() {
	log_info("Received scene update...");

	XrSceneComponentsGetInfoMSFT info       = { XR_TYPE_SCENE_COMPONENTS_GET_INFO_MSFT };
	XrSceneComponentsMSFT        components = { XR_TYPE_SCENE_COMPONENTS_MSFT };
	XrSceneMeshesMSFT            meshes     = { XR_TYPE_SCENE_MESHES_MSFT };
	info.componentType = XR_SCENE_COMPONENT_TYPE_VISUAL_MESH_MSFT;
	components.next    = &meshes;
	if (XR_FAILED(xr_extensions.xrGetSceneComponentsMSFT(xr_scene, &info, &components))) {
		log_warn("xrGetSceneComponentsMSFT failed");
		return;
	}
	components.componentCapacityInput = components.componentCountOutput;
	components.components             = sk_malloc_t(XrSceneComponentMSFT, components.componentCapacityInput);
	meshes.sceneMeshCount             = components.componentCapacityInput;
	meshes.sceneMeshes                = sk_malloc_t(XrSceneMeshMSFT, components.componentCapacityInput);
	if (XR_FAILED(xr_extensions.xrGetSceneComponentsMSFT(xr_scene, &info, &components))) {
		log_warn("xrGetSceneComponentsMSFT failed");
		return;
	}


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


	xr_scene_meshes.each([](scene_mesh_t &m) { mesh_release(m.mesh); });
	xr_scene_meshes.clear();
	for (size_t i = 0; i < meshes.sceneMeshCount; i++) {
		XrSceneMeshBuffersMSFT        buffers   = { XR_TYPE_SCENE_MESH_BUFFERS_MSFT };
		XrSceneMeshVertexBufferMSFT   v_buffer  = { XR_TYPE_SCENE_MESH_VERTEX_BUFFER_MSFT };
		XrSceneMeshIndicesUint32MSFT  i_buffer  = { XR_TYPE_SCENE_MESH_INDICES_UINT32_MSFT };
		XrSceneMeshBuffersGetInfoMSFT mesh_info = { XR_TYPE_SCENE_MESH_BUFFERS_GET_INFO_MSFT };
		buffers  .next         = &v_buffer;
		v_buffer .next         = &i_buffer;
		mesh_info.meshBufferId = meshes.sceneMeshes[i].meshBufferId;
		if (XR_FAILED(xr_extensions.xrGetSceneMeshBuffersMSFT(xr_scene, &mesh_info, &buffers))) {
			log_warn("xrGetSceneMeshBuffersMSFT failed");
			return;
		}
		i_buffer.indices             = sk_malloc_t(uint32_t,   i_buffer.indexCountOutput);
		v_buffer.vertices            = sk_malloc_t(XrVector3f, v_buffer.vertexCountOutput);
		i_buffer.indexCapacityInput  = i_buffer.indexCountOutput;
		v_buffer.vertexCapacityInput = v_buffer.vertexCountOutput;
		if (XR_FAILED(xr_extensions.xrGetSceneMeshBuffersMSFT(xr_scene, &mesh_info, &buffers))) {
			log_warn("xrGetSceneMeshBuffersMSFT failed");
			return;
		}

		scene_mesh_t mesh = { mesh_create() };
		vert_t *verts = sk_malloc_t(vert_t, v_buffer.vertexCountOutput);
		for (size_t v = 0; v < v_buffer.vertexCountOutput; v++) {
			verts[v] = { *(vec3 *)&v_buffer.vertices[v], {0,1,0}, {}, {255,255,255,255} };
		}
		mesh_calculate_normals(verts, v_buffer.vertexCountOutput, i_buffer.indices, i_buffer.indexCountOutput);
		mesh_set_inds (mesh.mesh, i_buffer.indices, i_buffer.indexCountOutput );
		mesh_set_verts(mesh.mesh, verts,            v_buffer.vertexCountOutput);
		
		free(i_buffer.indices);
		free(v_buffer.vertices);

		pose_t pose = { vec3_zero, quat_identity };
		if (locations.locations[i].flags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
			pose.position = *(vec3 *)&locations.locations[i].pose.position;
		if (locations.locations[i].flags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)
			pose.orientation = *(quat *)&locations.locations[i].pose.orientation;
		mesh.transform = pose_matrix(pose);

		xr_scene_meshes.add(mesh);
	}

	free(locations.locations);
	free(components.components);
	free(meshes.sceneMeshes);
}

///////////////////////////////////////////

void world_update_scene_planes() {
	log_info("Received scene update...");

	XrSceneComponentsGetInfoMSFT info       = { XR_TYPE_SCENE_COMPONENTS_GET_INFO_MSFT };
	XrSceneComponentsMSFT        components = { XR_TYPE_SCENE_COMPONENTS_MSFT };
	XrSceneMeshesMSFT            meshes     = { XR_TYPE_SCENE_MESHES_MSFT };
	info.componentType = XR_SCENE_COMPONENT_TYPE_VISUAL_MESH_MSFT;
	components.next    = &meshes;
	if (XR_FAILED(xr_extensions.xrGetSceneComponentsMSFT(xr_scene, &info, &components))) {
		log_warn("xrGetSceneComponentsMSFT failed");
		return;
	}
	components.componentCapacityInput = components.componentCountOutput;
	components.components             = sk_malloc_t(XrSceneComponentMSFT, components.componentCapacityInput);
	meshes.sceneMeshCount             = components.componentCapacityInput;
	meshes.sceneMeshes                = sk_malloc_t(XrSceneMeshMSFT, components.componentCapacityInput);
	if (XR_FAILED(xr_extensions.xrGetSceneComponentsMSFT(xr_scene, &info, &components))) {
		log_warn("xrGetSceneComponentsMSFT failed");
		return;
	}


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


	xr_scene_meshes.each([](scene_mesh_t &m) { mesh_release(m.mesh); });
	xr_scene_meshes.clear();
	for (size_t i = 0; i < meshes.sceneMeshCount; i++) {
		XrSceneMeshBuffersMSFT        buffers   = { XR_TYPE_SCENE_MESH_BUFFERS_MSFT };
		XrSceneMeshVertexBufferMSFT   v_buffer  = { XR_TYPE_SCENE_MESH_VERTEX_BUFFER_MSFT };
		XrSceneMeshIndicesUint32MSFT  i_buffer  = { XR_TYPE_SCENE_MESH_INDICES_UINT32_MSFT };
		XrSceneMeshBuffersGetInfoMSFT mesh_info = { XR_TYPE_SCENE_MESH_BUFFERS_GET_INFO_MSFT };
		buffers  .next         = &v_buffer;
		v_buffer .next         = &i_buffer;
		mesh_info.meshBufferId = meshes.sceneMeshes[i].meshBufferId;
		if (XR_FAILED(xr_extensions.xrGetSceneMeshBuffersMSFT(xr_scene, &mesh_info, &buffers))) {
			log_warn("xrGetSceneMeshBuffersMSFT failed");
			return;
		}
		i_buffer.indices             = sk_malloc_t(uint32_t,   i_buffer.indexCountOutput);
		v_buffer.vertices            = sk_malloc_t(XrVector3f, v_buffer.vertexCountOutput);
		i_buffer.indexCapacityInput  = i_buffer.indexCountOutput;
		v_buffer.vertexCapacityInput = v_buffer.vertexCountOutput;
		if (XR_FAILED(xr_extensions.xrGetSceneMeshBuffersMSFT(xr_scene, &mesh_info, &buffers))) {
			log_warn("xrGetSceneMeshBuffersMSFT failed");
			return;
		}

		scene_mesh_t mesh = { mesh_create() };
		vert_t *verts = sk_malloc_t(vert_t, v_buffer.vertexCountOutput);
		for (size_t v = 0; v < v_buffer.vertexCountOutput; v++) {
			verts[v] = { *(vec3 *)&v_buffer.vertices[v], {0,1,0}, {}, {255,255,255,255} };
		}
		mesh_calculate_normals(verts, v_buffer.vertexCountOutput, i_buffer.indices, i_buffer.indexCountOutput);
		mesh_set_inds (mesh.mesh, i_buffer.indices, i_buffer.indexCountOutput );
		mesh_set_verts(mesh.mesh, verts,            v_buffer.vertexCountOutput);

		free(i_buffer.indices);
		free(v_buffer.vertices);

		pose_t pose = { vec3_zero, quat_identity };
		if (locations.locations[i].flags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
			pose.position = *(vec3 *)&locations.locations[i].pose.position;
		if (locations.locations[i].flags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)
			pose.orientation = *(quat *)&locations.locations[i].pose.orientation;
		mesh.transform = pose_matrix(pose);

		xr_scene_meshes.add(mesh);
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
	//material_set_wireframe(xr_scene_material, true);

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
					world_update_scene_meshes();
				}
			} else if (state == XR_SCENE_COMPUTE_STATE_COMPLETED_WITH_ERROR_MSFT) {
				log_warn("Scene computed failed with an error!");
				xr_scene_updating = false;
			} else {
			}
		}
	}

	for (size_t i = 0; i < xr_scene_meshes.count; i++) {
		render_add_mesh(xr_scene_meshes[i].mesh, xr_scene_material, xr_scene_meshes[i].transform, color_hsv((i % 8) / 8.0f, 0.8f, 0.8f, 1));
	}
}

///////////////////////////////////////////

void world_shutdown() {
	if (xr_ext_available.MSFT_scene_understanding) {
		xr_extensions.xrDestroySceneObserverMSFT(xr_scene_observer);
	}
}

} // namespace sk