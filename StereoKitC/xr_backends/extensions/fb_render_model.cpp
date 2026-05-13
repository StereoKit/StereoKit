// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith
// Copyright (c) 2026 Qualcomm Technologies, Inc.

// This implements XR_FB_render_model as a fallback for runtimes that don't yet
// support XR_EXT_render_model / XR_EXT_interaction_render_model (notably Quest
// as of this writing).
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_FB_render_model

#include "fb_render_model.h"
#include "ext_management.h"

#include <stdio.h>

///////////////////////////////////////////

#define XR_EXT_FUNCTIONS( X )           \
	X(xrEnumerateRenderModelPathsFB)    \
	X(xrGetRenderModelPropertiesFB)     \
	X(xrLoadRenderModelFB)
OPENXR_DEFINE_FN_STATIC(XR_EXT_FUNCTIONS);

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

typedef struct xr_fb_model_t {
	XrPath             path;
	XrRenderModelKeyFB key;
	uint32_t           version;
	bool               unavailable;
	model_t            model;
} xr_fb_model_t;

typedef struct xr_fb_render_model_state_t {
	bool                   available;
	int32_t                controller_idx[handed_max];
	array_t<xr_fb_model_t> models;
} xr_fb_render_model_state_t;
static xr_fb_render_model_state_t local = { };

///////////////////////////////////////////

xr_system_ xr_fb_render_model_initialize(void*);
void       xr_fb_render_model_shutdown  (void*);
void       xr_fb_render_model_poll      (void*, XrEventDataBuffer* event);
bool       xr_fb_render_model_load      (xr_fb_model_t* ref_model);

///////////////////////////////////////////

void xr_fb_render_model_register() {
	local = {};

	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_FB_RENDER_MODEL_EXTENSION_NAME;
	sys.evt_initialize = { xr_fb_render_model_initialize };
	sys.evt_shutdown   = { xr_fb_render_model_shutdown   };
	sys.evt_poll       = { (void (*)(void*, void*))xr_fb_render_model_poll };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_fb_render_model_initialize(void*) {
	local.controller_idx[0] = -1;
	local.controller_idx[1] = -1;
	if (!backend_openxr_ext_enabled(XR_FB_RENDER_MODEL_EXTENSION_NAME))
		return xr_system_fail;

	// Strict fallback: if the cross-vendor EXT pair is enabled, defer to it
	// entirely. Don't load FB functions, don't enumerate, don't mark ourselves
	// available - so callers see a clean "FB unavailable" signal.
	if (backend_openxr_ext_enabled(XR_EXT_RENDER_MODEL_EXTENSION_NAME) &&
		backend_openxr_ext_enabled(XR_EXT_INTERACTION_RENDER_MODEL_EXTENSION_NAME))
		return xr_system_fail;

	OPENXR_LOAD_FN_RETURN(XR_EXT_FUNCTIONS, xr_system_fail);

	// xrGetRenderModelPropertiesFB requires that xrEnumerateRenderModelPathsFB
	// has been called first, so we do it once up front. The path set is static
	// per runtime - the *models* behind each path can change as devices come
	// and go, but the paths themselves don't.
	uint32_t count = 0;
	XrResult r = xrEnumerateRenderModelPathsFB(xr_session, 0, &count, nullptr);
	if (XR_FAILED(r)) { log_warnf("%s [%s]", "xrEnumerateRenderModelPathsFB", openxr_string(r)); return xr_system_fail; }
	XrRenderModelPathInfoFB* paths = sk_malloc_t(XrRenderModelPathInfoFB, count);
	for (uint32_t i = 0; i < count; i++) paths[i] = { XR_TYPE_RENDER_MODEL_PATH_INFO_FB };
	r = xrEnumerateRenderModelPathsFB(xr_session, count, &count, paths);
	if (XR_FAILED(r)) { log_warnf("%s [%s]", "xrEnumerateRenderModelPathsFB", openxr_string(r)); sk_free(paths); return xr_system_fail; }

	XrPath path_ctrl[handed_max] = {};
	xrStringToPath(xr_instance, "/model_fb/controller/left",  &path_ctrl[handed_left ]);
	xrStringToPath(xr_instance, "/model_fb/controller/right", &path_ctrl[handed_right]);

	for (uint32_t i = 0; i < count; i++) {
		xr_fb_model_t m = {};
		m.path = paths[i].path;
		int32_t idx = local.models.add(m);
		if      (paths[i].path == path_ctrl[handed_left ]) local.controller_idx[handed_left ] = idx;
		else if (paths[i].path == path_ctrl[handed_right]) local.controller_idx[handed_right] = idx;
	}
	sk_free(paths);

	local.available = true;
	return xr_system_succeed;
}

///////////////////////////////////////////

void xr_fb_render_model_shutdown(void*) {
	for (int32_t i = 0; i < local.models.count; i++) {
		model_release(local.models[i].model);
	}
	local.models.free();
	OPENXR_CLEAR_FN(XR_EXT_FUNCTIONS);
	local = {};
}

///////////////////////////////////////////

bool xr_fb_render_model_load(xr_fb_model_t* ref_model) {
	XrRenderModelCapabilitiesRequestFB caps = { XR_TYPE_RENDER_MODEL_CAPABILITIES_REQUEST_FB };
	caps.flags = XR_RENDER_MODEL_SUPPORTS_GLTF_2_0_SUBSET_1_BIT_FB | XR_RENDER_MODEL_SUPPORTS_GLTF_2_0_SUBSET_2_BIT_FB;
	XrRenderModelPropertiesFB props = { XR_TYPE_RENDER_MODEL_PROPERTIES_FB };
	props.next = &caps;
	XrResult r = xrGetRenderModelPropertiesFB(xr_session, ref_model->path, &props);
	if (r == XR_RENDER_MODEL_UNAVAILABLE_FB || props.modelKey == XR_NULL_RENDER_MODEL_KEY_FB) {
		ref_model->unavailable = true;
		return false;
	}
	if (XR_FAILED(r)) { log_warnf("%s [%s]", "xrGetRenderModelPropertiesFB", openxr_string(r)); return false; }
	ref_model->unavailable = false;

	// Already up to date?
	if (ref_model->model != nullptr && ref_model->key == props.modelKey && ref_model->version == props.modelVersion)
		return true;

	// Build a unique asset id so duplicate loads share the same model_t
	char name[160];
	snprintf(name, sizeof(name), "sk/xr_model_fb/%u_%s_v%u.glb", props.vendorId, props.modelName, props.modelVersion);

	model_release(ref_model->model);
	ref_model->model = model_find(name);
	if (ref_model->model == nullptr) {
		XrRenderModelLoadInfoFB load_info = { XR_TYPE_RENDER_MODEL_LOAD_INFO_FB };
		load_info.modelKey = props.modelKey;
		XrRenderModelBufferFB buffer = { XR_TYPE_RENDER_MODEL_BUFFER_FB };
		r = xrLoadRenderModelFB(xr_session, &load_info, &buffer);
		if (r == XR_RENDER_MODEL_UNAVAILABLE_FB || buffer.bufferCountOutput == 0) { ref_model->unavailable = true; return false; }
		if (XR_FAILED(r)) { log_warnf("%s [%s]", "xrLoadRenderModelFB(size)", openxr_string(r)); return false; }
		buffer.bufferCapacityInput = buffer.bufferCountOutput;
		buffer.buffer              = sk_malloc_t(uint8_t, buffer.bufferCountOutput);
		r = xrLoadRenderModelFB(xr_session, &load_info, &buffer);
		if (XR_FAILED(r)) { log_warnf("%s [%s]", "xrLoadRenderModelFB", openxr_string(r)); sk_free(buffer.buffer); return false; }

		ref_model->model = model_create_mem(name, buffer.buffer, buffer.bufferCountOutput);
		model_set_id(ref_model->model, name);

		sk_free(buffer.buffer);

		// StereoKit's glTF loader applies a 180-degree-about-Y "correction" to
		// root node transforms (model_gltf.cpp), assuming the asset is authored
		// in glTF's spec +Z-forward convention and needs flipping to fit a -Z-
		// forward world. XR render models are authored against OpenXR's grip
		// pose, not glTF's "forward face" recommendation, so that correction
		// just rotates them away from where the runtime expects. Undo it on
		// every root node by re-applying the same 180-Y rotation (self-inverse).
		matrix undo = matrix_trs(vec3_zero, quat_from_angles(0, 180, 0));
		for (model_node_id n = model_node_get_root(ref_model->model); n != -1; n = model_node_sibling(ref_model->model, n)) {
			model_node_set_transform_local(ref_model->model, n,
				model_node_get_transform_local(ref_model->model, n) * undo);
		}
	}

	ref_model->key     = props.modelKey;
	ref_model->version = props.modelVersion;
	return true;
}

///////////////////////////////////////////

void xr_fb_render_model_poll(void*, XrEventDataBuffer* event) {
	if (event->type != XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED) return;

	// Profile change is the strongest signal we get that a controller has been
	// (re)connected, so refresh anything that wasn't loadable last time.
	for (int32_t i = 0; i < local.models.count; i++) {
		if (local.models[i].model == nullptr || local.models[i].unavailable)
			xr_fb_render_model_load(&local.models[i]);
	}
}

///////////////////////////////////////////

bool xr_fb_render_model_available() {
	return local.available;
}

///////////////////////////////////////////

model_t xr_fb_render_model_get(handed_ hand) {
	if (!local.available || local.controller_idx[hand] < 0) return nullptr;

	xr_fb_model_t* m = &local.models[local.controller_idx[hand]];
	if (m->model == nullptr && !m->unavailable) xr_fb_render_model_load(m);
	return m->model;
}

///////////////////////////////////////////

void xr_fb_render_model_draw_controller(handed_ hand) {
	model_t m = xr_fb_render_model_get(hand);
	if (m == nullptr) return;

	// Models are authored at the OpenXR grip pose, and the glTF-spec axis
	// correction was already stripped at load time, so we can apply the grip
	// transform directly with no extra fixup.
	const controller_t* c = input_controller(hand);
	render_add_model(m, matrix_trs(c->pose.position, c->pose.orientation));
}

} // namespace sk
