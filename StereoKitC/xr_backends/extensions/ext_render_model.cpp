// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

// This implements XR_EXT_render_model
// https://registry.khronos.org/OpenXR/specs/1.1/html/xrspec.html#XR_EXT_render_model

#include "ext_render_model.h"
#include "ext_management.h"

#include <inttypes.h>
#include <stdio.h>

 ///////////////////////////////////////////

#define XR_EXT_FUNCTIONS( X )             \
	X(xrCreateRenderModelAssetEXT)        \
	X(xrCreateRenderModelEXT)             \
	X(xrCreateRenderModelSpaceEXT)        \
	X(xrDestroyRenderModelAssetEXT)       \
	X(xrDestroyRenderModelEXT)            \
	X(xrGetRenderModelAssetDataEXT)       \
	X(xrGetRenderModelAssetPropertiesEXT) \
	X(xrGetRenderModelPropertiesEXT)      \
	X(xrGetRenderModelStateEXT)
OPENXR_DEFINE_FN_STATIC(XR_EXT_FUNCTIONS);

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

typedef struct xr_ext_render_model_state_t {
	bool available;
} xr_ext_render_model_state_t;
static xr_ext_render_model_state_t local = { };

///////////////////////////////////////////

xr_system_ xr_ext_render_model_initialize(void*);
void       xr_ext_render_model_shutdown  (void*);

///////////////////////////////////////////

void xr_ext_render_model_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_EXT_RENDER_MODEL_EXTENSION_NAME;
	sys.request_exts[sys.request_ext_count++] = XR_EXT_UUID_EXTENSION_NAME;
	sys.evt_initialize = { xr_ext_render_model_initialize };
	sys.evt_shutdown   = { xr_ext_render_model_shutdown };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_render_model_initialize(void*) {
	if (!backend_openxr_ext_enabled(XR_EXT_RENDER_MODEL_EXTENSION_NAME))
		return xr_system_fail;

	OPENXR_LOAD_FN_RETURN(XR_EXT_FUNCTIONS, xr_system_fail);

	local.available = true;

	return xr_system_succeed;
}

///////////////////////////////////////////

void xr_ext_render_model_shutdown(void*) {
	OPENXR_CLEAR_FN(XR_EXT_FUNCTIONS);
}

///////////////////////////////////////////

xr_render_model_t xr_ext_render_model_get(XrRenderModelIdEXT id) {
	if (!local.available) return {};

	const char* gltf_exts[] = {
		"KHR_texture_basisu",
		"KHR_materials_unlit",
		"KHR_texture_transform",
		"EXT_meshopt_compression",
	};

	xr_render_model_t result = {};
	result.id = id;

	XrRenderModelCreateInfoEXT model_info = { XR_TYPE_RENDER_MODEL_CREATE_INFO_EXT };
	model_info.renderModelId      = id;
	model_info.gltfExtensionCount = sizeof(gltf_exts) / sizeof(gltf_exts[0]);
	model_info.gltfExtensions     = gltf_exts;
	XrResult r = xrCreateRenderModelEXT(xr_session, &model_info, &result.render_model);
	if (XR_FAILED(r)) log_warnf("%s: [%s]", "xrCreateRenderModelEXT", openxr_string(r));

	XrRenderModelSpaceCreateInfoEXT   space_info       = { XR_TYPE_RENDER_MODEL_SPACE_CREATE_INFO_EXT };
	space_info.renderModel = result.render_model;
	r = xrCreateRenderModelSpaceEXT(xr_session, &space_info, &result.space);
	if (XR_FAILED(r)) log_warnf("%s: [%s]", "xrCreateRenderModelSpaceEXT", openxr_string(r));

	XrRenderModelPropertiesGetInfoEXT props_info       = { XR_TYPE_RENDER_MODEL_PROPERTIES_GET_INFO_EXT };
	XrRenderModelPropertiesEXT        props            = { XR_TYPE_RENDER_MODEL_PROPERTIES_EXT          };
	r = xrGetRenderModelPropertiesEXT(result.render_model, &props_info, &props);
	if (XR_FAILED(r)) log_warnf("%s: [%s]","xrGetRenderModelPropertiesEXT", openxr_string(r));

	XrRenderModelAssetEXT            asset;
	XrRenderModelAssetCreateInfoEXT  asset_info = { XR_TYPE_RENDER_MODEL_ASSET_CREATE_INFO_EXT };
	asset_info.cacheId = props.cacheId;
	r = xrCreateRenderModelAssetEXT(xr_session, &asset_info, &asset);
	if (XR_FAILED(r)) log_warnf("xrCreateRenderModelAssetEXT: [%s]", openxr_string(r));

	char name[128];
	XrUuidEXT uuid = props.cacheId;
	snprintf(name, sizeof(name), "sk/xr_model/%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x.glb",
		uuid.data[0 ], uuid.data[1 ], uuid.data[2 ], uuid.data[3 ],
		uuid.data[4 ], uuid.data[5 ], uuid.data[6 ], uuid.data[7 ],
		uuid.data[8 ], uuid.data[9 ], uuid.data[10], uuid.data[11],
		uuid.data[12], uuid.data[13], uuid.data[14], uuid.data[15]);
	result.model = model_find(name);

	if (result.model == nullptr) {
		XrRenderModelAssetDataGetInfoEXT data_info = { XR_TYPE_RENDER_MODEL_ASSET_DATA_GET_INFO_EXT };
		XrRenderModelAssetDataEXT        data      = { XR_TYPE_RENDER_MODEL_ASSET_DATA_EXT };
		xrGetRenderModelAssetDataEXT(asset, &data_info, &data);
		data.bufferCapacityInput = data.bufferCountOutput;
		data.buffer              = sk_malloc_t(uint8_t, data.bufferCountOutput);
		r = xrGetRenderModelAssetDataEXT(asset, &data_info, &data);
		if (XR_FAILED(r)) log_warnf("%s: [%s]", "xrGetRenderModelAssetDataEXT", openxr_string(r));

		result.model = model_create_mem(name, data.buffer, data.bufferCountOutput);
		model_set_id(result.model, name);

		sk_free(data.buffer);
	}
	result.model = model_copy(result.model);

	XrRenderModelAssetPropertiesGetInfoEXT asset_props_info = { XR_TYPE_RENDER_MODEL_ASSET_PROPERTIES_GET_INFO_EXT };
	XrRenderModelAssetPropertiesEXT        asset_props      = { XR_TYPE_RENDER_MODEL_ASSET_PROPERTIES_EXT };
	XrRenderModelAssetNodePropertiesEXT*   node_props       = sk_malloc_t(XrRenderModelAssetNodePropertiesEXT, props.animatableNodeCount);
	asset_props.nodePropertyCount = props.animatableNodeCount;
	asset_props.nodeProperties    = node_props;
	r = xrGetRenderModelAssetPropertiesEXT(asset, &asset_props_info, &asset_props);
	if (XR_FAILED(r)) log_warnf("%s: [%s]", "xrGetRenderModelAssetPropertiesEXT", openxr_string(r));
	result.anim_nodes      = sk_malloc_t(model_node_id, asset_props.nodePropertyCount);
	result.anim_node_count = asset_props.nodePropertyCount;
	for (uint32_t i = 0; i < asset_props.nodePropertyCount; i++) {
		result.anim_nodes[i] = model_node_find(result.model, asset_props.nodeProperties[i].uniqueName);
		matrix m = model_node_get_transform_local(result.model, result.anim_nodes[i]);
		vec3   p = matrix_extract_translation(m);
	}

	result.state_query                = { XR_TYPE_RENDER_MODEL_STATE_EXT };
	result.state_query.nodeStateCount = asset_props.nodePropertyCount;
	result.state_query.nodeStates     = sk_malloc_t(XrRenderModelNodeStateEXT, asset_props.nodePropertyCount);

	sk_free(node_props);

	xrDestroyRenderModelAssetEXT(asset);

	return result;
}

///////////////////////////////////////////

void xr_ext_render_model_update(xr_render_model_t* ref_model) {
	XrRenderModelStateGetInfoEXT info  = { XR_TYPE_RENDER_MODEL_STATE_GET_INFO_EXT };
	info.displayTime = xr_time;
	XrResult r = xrGetRenderModelStateEXT(ref_model->render_model, &info, &ref_model->state_query);
	if (XR_FAILED(r)) log_warnf("%s: [%s]", "xrGetRenderModelStateEXT", openxr_string(r));

	for (uint32_t i = 0; i < ref_model->state_query.nodeStateCount; i++) {
		model_node_id node = ref_model->anim_nodes[i];

		XrVector3f    p = ref_model->state_query.nodeStates[i].nodePose.position;
		XrQuaternionf r = ref_model->state_query.nodeStates[i].nodePose.orientation;
		model_node_set_transform_local(ref_model->model, node, matrix_trs({ p.x, p.y, p.z }, {r.x, r.y, r.z, r.w}));
		model_node_set_visible        (ref_model->model, node, ref_model->state_query.nodeStates[i].isVisible);
	}
}

///////////////////////////////////////////

void xr_ext_render_model_destroy(xr_render_model_t* ref_model) {
	model_release (ref_model->model);
	xrDestroySpace(ref_model->space);
	sk_free(ref_model->anim_nodes);
	sk_free(ref_model->state_query.nodeStates);
	*ref_model = {};
}

} // namespace sk