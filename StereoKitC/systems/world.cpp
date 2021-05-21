#include "../stereokit.h"
#include "../_stereokit.h"
#include "../sk_memory.h"
#include "platform/openxr.h"

namespace sk {

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

XrSceneObserverMSFT xr_scene_observer = {};
XrSceneMSFT         xr_scene          = {};
bool                xr_scene_updating = false;
mesh_t              xr_scene_mesh     = nullptr;
vec3                xr_scene_last_center = { 1000000,1000000,100000 };
float               xr_scene_radius = 4;

void world_request_update() {
	log_info("Requesting scene update...");

	XrSceneComputeFeatureMSFT features[] = {
		XR_SCENE_COMPUTE_FEATURE_VISUAL_MESH_MSFT,
		XR_SCENE_COMPUTE_FEATURE_PLANE_MSFT,
		XR_SCENE_COMPUTE_FEATURE_COLLIDER_MESH_MSFT 
	};

	XrNewSceneComputeInfoMSFT compute_info = { (XrStructureType)XR_TYPE_NEW_SCENE_COMPUTE_INFO_MSFT };
	XrSceneSphereBoundMSFT    bound_sphere = { {}, xr_scene_radius };
	compute_info.requestedFeatures     = features;
	compute_info.requestedFeatureCount = _countof(features);
	compute_info.bounds.space       = xr_app_space;
	compute_info.bounds.time        = xr_time;
	compute_info.bounds.sphereCount = 1;
	compute_info.bounds.spheres     = &bound_sphere;
	if (XR_FAILED(xr_extensions.xrComputeNewSceneMSFT(xr_scene_observer, &compute_info))) {
		log_warn("xrComputeNewSceneMSFT failed");
		return;
	}

	log_info("Requested scene update!");
	xr_scene_updating = true;
}

///////////////////////////////////////////

void world_update_scene() {
	log_info("Received scene update...");

	XrSceneComponentsGetInfoMSFT info = { (XrStructureType)XR_TYPE_SCENE_COMPONENTS_GET_INFO_MSFT };
	info.componentType = XR_SCENE_COMPONENT_TYPE_VISUAL_MESH_MSFT;
	XrSceneMeshStatesMSFT mesh_states = { (XrStructureType)XR_TYPE_SCENE_MESH_STATES_MSFT };
	info.next = &mesh_states;

	XrSceneComponentStatesMSFT components = { (XrStructureType)XR_TYPE_SCENE_COMPONENT_STATES_MSFT };
	if (XR_FAILED(xr_extensions.xrGetSceneComponentsMSFT(xr_scene, &info, &components))) {
		log_warn("xrGetSceneComponentsMSFT failed");
		return;
	}

	XrSceneComponentStateMSFT *component_states = sk_malloc_t(XrSceneComponentStateMSFT, components.componentCountOutput);
	components.components             = component_states;
	components.componentCapacityInput = components.componentCountOutput;
	if (XR_FAILED(xr_extensions.xrGetSceneComponentsMSFT(xr_scene, &info, &components))) {
		log_warn("xrGetSceneComponentsMSFT failed");
		return;
	}

	for (uint32_t i = 0; i < mesh_states.sceneMeshCount; i++) {
		mesh_states.sceneMeshes[i].meshBufferId;

		XrSceneMeshBuffersGetInfoMSFT mesh_info = { (XrStructureType)XR_TYPE_SCENE_MESH_BUFFERS_GET_INFO_MSFT };
		mesh_info.meshBufferId = mesh_states.sceneMeshes[i].meshBufferId;
		XrSceneMeshBuffersMSFT buffers = {};
		xr_extensions.xrGetSceneMeshBuffersMSFT(xr_scene, &mesh_info, &buffers);
	}

	for (uint32_t i = 0; i < components.componentCountOutput; i++) {
		switch (component_states[i].componentType) {
		case XR_SCENE_COMPONENT_TYPE_VISUAL_MESH_MSFT: {
			XrSceneComponentsLocateInfoMSFT component_info = { (XrStructureType)XR_TYPE_SCENE_COMPONENTS_LOCATE_INFO_MSFT };
			XrSceneComponentLocationsMSFT   locations      = {};
			component_info.baseSpace = xr_app_space;
			component_info.idCount   = 1;
			component_info.ids       = &component_states[i].componentId;
			component_info.time      = xr_time;
			if (XR_FAILED(xr_extensions.xrLocateSceneComponentsMSFT(xr_scene, &component_info, &locations))) {
				log_warn("xrLocateSceneComponentsMSFT failed");
			} else {

			}
			
		} break;
		}
	}
	free(component_states);
}

///////////////////////////////////////////

bool world_init() {
	if (xr_ext_available.MSFT_scene_understanding_preview3) {
		XrSceneObserverCreateInfoMSFT create_info = { (XrStructureType)XR_TYPE_SCENE_OBSERVER_CREATE_INFO_MSFT };
		if (XR_FAILED(xr_extensions.xrCreateSceneObserverMSFT(xr_session, &create_info, &xr_scene_observer))) {
			log_err("xrCreateSceneObserverMSFT failed");
			return false;
		}
		log_info("Scene observer created!");
	}

	return true;
}

///////////////////////////////////////////

void world_update() {
	if (xr_ext_available.MSFT_scene_understanding_preview3) {

		if (!xr_scene_updating && vec3_magnitude_sq(input_head()->position - xr_scene_last_center) > xr_scene_radius * xr_scene_radius) {
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
				xr_extensions.xrDestroySceneMSFT(xr_scene);
				if (XR_FAILED(xr_extensions.xrCreateSceneMSFT(xr_scene_observer, &info, &xr_scene))) {
					log_warn("xrCreateSceneMSFT failed");
				} else {
					world_update_scene();
				}
			} else if (state == XR_SCENE_COMPUTE_STATE_COMPLETED_WITH_ERROR_MSFT) {
				log_warn("Scene computed failed with an error!");
				xr_scene_updating = false;
			} else {
			}
		}
	}
}

///////////////////////////////////////////

void world_shutdown() {
	if (xr_ext_available.MSFT_scene_understanding_preview3) {
		xr_extensions.xrDestroySceneObserverMSFT(xr_scene_observer);
	}
}

} // namespace sk