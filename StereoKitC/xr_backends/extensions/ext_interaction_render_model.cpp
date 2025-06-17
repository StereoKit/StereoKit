// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

// This implements XR_EXT_interaction_render_model
// https://registry.khronos.org/OpenXR/specs/1.1/html/xrspec.html#XR_EXT_interaction_render_model

#include "ext_interaction_render_model.h"
#include "ext_render_model.h"
#include "ext_management.h"

 ///////////////////////////////////////////

#define XR_EXT_FUNCTIONS( X ) \
	X(xrEnumerateInteractionRenderModelIdsEXT) \
	X(xrEnumerateRenderModelSubactionPathsEXT) \
	X(xrGetRenderModelPoseTopLevelUserPathEXT)
OPENXR_DEFINE_FN_STATIC(XR_EXT_FUNCTIONS);

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

typedef struct xr_ext_interaction_render_model_t {
	bool               available;
	xr_render_model_t* models;
	int32_t            model_count;
	int32_t            controller_idx[handed_max];
} xr_ext_interaction_render_model_t;
static xr_ext_interaction_render_model_t local = { };

///////////////////////////////////////////

xr_system_ xr_ext_interaction_render_model_initialize(void*);
void       xr_ext_interaction_render_model_shutdown  (void*);
void       xr_ext_interaction_render_model_poll      (void*, XrEventDataBuffer* event);

///////////////////////////////////////////

void xr_ext_interaction_render_model_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_EXT_INTERACTION_RENDER_MODEL_EXTENSION_NAME;
	sys.evt_initialize = { xr_ext_interaction_render_model_initialize };
	sys.evt_shutdown   = { xr_ext_interaction_render_model_shutdown   };
	sys.evt_poll       = { (void (*)(void*, void*))xr_ext_interaction_render_model_poll };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_interaction_render_model_initialize(void*) {
	local.controller_idx[0] = -1;
	local.controller_idx[1] = -1;
	if (!backend_openxr_ext_enabled(XR_EXT_INTERACTION_RENDER_MODEL_EXTENSION_NAME))
		return xr_system_fail;

	OPENXR_LOAD_FN_RETURN(XR_EXT_FUNCTIONS, xr_system_fail);

	local.available = true;

	return xr_system_succeed;
}

///////////////////////////////////////////

void xr_ext_interaction_render_model_shutdown(void*) {
	OPENXR_CLEAR_FN(XR_EXT_FUNCTIONS);
}

///////////////////////////////////////////

void _xr_ext_interaction_render_model_draw(xr_render_model_t *model) {
	xr_ext_render_model_update(model);

	XrSpaceLocation loc = { XR_TYPE_SPACE_LOCATION };
	if (XR_FAILED(xrLocateSpace(model->space, xr_app_space, xr_time, &loc)))
		return;
	if ((loc.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)    != 0 &&
		(loc.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0) {
		XrVector3f    p = loc.pose.position;
		XrQuaternionf r = loc.pose.orientation;
		model_draw(model->model, matrix_trs({ p.x, p.y, p.z }, { r.x, r.y, r.z, r.w }));
	}
}

///////////////////////////////////////////

void xr_ext_interaction_render_model_draw_others() {
	for (int32_t i = 0; i < local.model_count; i++) {
		if (i == local.controller_idx[0] ||
			i == local.controller_idx[1])
			continue;
		_xr_ext_interaction_render_model_draw(&local.models[i]);
	}
}

///////////////////////////////////////////

void xr_ext_interaction_render_model_draw_controller(handed_ hand) {
	if (local.controller_idx[hand] >= 0)
		_xr_ext_interaction_render_model_draw(&local.models[local.controller_idx[hand]]);
}

///////////////////////////////////////////

model_t xr_ext_interaction_render_model_get(handed_ hand) {
	return local.controller_idx[hand] >= 0
		? local.models[local.controller_idx[hand]].model
		: nullptr;
}

///////////////////////////////////////////

bool xr_ext_interaction_render_model_available() {
	return local.available;
}

///////////////////////////////////////////

void xr_ext_interaction_render_model_poll(void*, XrEventDataBuffer* event) {
	if (event->type != XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED &&
		event->type != XR_TYPE_EVENT_DATA_INTERACTION_RENDER_MODELS_CHANGED_EXT) // Should be this event, but it's not firing?
		return;

	// Enumerate the render model IDs
	uint32_t                                    count    = 0;
	XrInteractionRenderModelIdsEnumerateInfoEXT get_info = { XR_TYPE_INTERACTION_RENDER_MODEL_IDS_ENUMERATE_INFO_EXT };
	xrEnumerateInteractionRenderModelIdsEXT(xr_session, &get_info, 0, &count, NULL);
	XrRenderModelIdEXT* model_ids = sk_malloc_t(XrRenderModelIdEXT, count);
	XrResult r = xrEnumerateInteractionRenderModelIdsEXT(xr_session, &get_info, count, &count, model_ids);
	if (XR_FAILED(r)) log_warnf("%s: [%s]", "xrEnumerateInteractionRenderModelIdsEXT", openxr_string(r));

	// Get the new models
	int32_t            new_count = count;
	xr_render_model_t* new_list  = sk_malloc_t(xr_render_model_t, count);
	for (int32_t i = 0; i < count; i++) {
		new_list[i] = xr_ext_render_model_get(model_ids[i]);
	}
	sk_free(model_ids);

	// Release our previous list of models (this comes second so we don't
	// release models that are re-used in the new list).
	for (int32_t i = 0; i < local.model_count; i++) {
		xr_ext_render_model_destroy(&local.models[i]);
	}
	sk_free(local.models);

	// Swap over to the new list
	local.models      = new_list;
	local.model_count = new_count;

	// See which models are for the left or right controller
	XrPath hand_path[handed_max] = { };
	xrStringToPath(xr_instance, "/user/hand/left",  &hand_path[handed_left ]);
	xrStringToPath(xr_instance, "/user/hand/right", &hand_path[handed_right]);
	local.controller_idx[0] = -1;
	local.controller_idx[1] = -1;
	for (int32_t i = 0; i < local.model_count; i++) {
		XrInteractionRenderModelSubactionPathInfoEXT info = { XR_TYPE_INTERACTION_RENDER_MODEL_SUBACTION_PATH_INFO_EXT };
		uint32_t path_count = 0;
		xrEnumerateRenderModelSubactionPathsEXT(new_list[i].render_model, &info, 0,          &path_count, nullptr);
		XrPath* paths = sk_malloc_t(XrPath, path_count);
		xrEnumerateRenderModelSubactionPathsEXT(new_list[i].render_model, &info, path_count, &path_count, paths);

		for (int32_t p = 0; p < path_count; p++) {
			if      (paths[p] == hand_path[handed_left ]) local.controller_idx[handed_left ] = i;
			else if (paths[p] == hand_path[handed_right]) local.controller_idx[handed_right] = i;
		}

		sk_free(paths);
	}
}

} // namespace sk