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

typedef struct xr_render_model_source_t {
	XrRenderModelIdEXT id;
	XrRenderModelEXT   render_model;
	model_t            model_asset;
	model_node_id*     anim_nodes;
	int32_t            anim_node_count;
} xr_model_source_t;

typedef struct xr_ext_render_model_state_t {
	bool                              available;
	array_t<xr_render_model_source_t> sources;
} xr_ext_render_model_state_t;
static xr_ext_render_model_state_t local = { };

///////////////////////////////////////////

xr_system_ xr_ext_render_model_initialize(void*);
void       xr_ext_render_model_shutdown  (void*);
void       xr_ext_render_model_source_destroy    (xr_render_model_source_t* ref_source);
bool       xr_ext_render_model_source_instantiate(int32_t model_idx, xr_render_model_t* out_inst);

///////////////////////////////////////////

void xr_ext_render_model_register() {
	local = {};

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
	for (int32_t i = 0; i < local.sources.count; i++) {
		xr_ext_render_model_source_destroy(&local.sources[i]);
	}
	local.sources.free();

	OPENXR_CLEAR_FN(XR_EXT_FUNCTIONS);
	local = {};
}

///////////////////////////////////////////

bool xr_ext_render_model_get(XrRenderModelIdEXT id, xr_render_model_t* out_model) {
	*out_model = {};
	if (!local.available) return false;

	// Check if we've loaded this render model already! We keep references to
	// all render models the runtime provides.
	for (int32_t i = 0; i < local.sources.count; i++) {
		if (local.sources[i].id == id)
			return xr_ext_render_model_source_instantiate(i, out_model);
	}

	// New model, lets go load it and parse it for use!

	const char* gltf_exts[] = {
		"KHR_texture_basisu",
		"KHR_materials_unlit",
		"KHR_texture_transform",
		"EXT_meshopt_compression",
	};

	xr_render_model_source_t source = {};
	source.id = id;

	XrRenderModelCreateInfoEXT model_info = { XR_TYPE_RENDER_MODEL_CREATE_INFO_EXT };
	model_info.renderModelId      = id;
	model_info.gltfExtensionCount = sizeof(gltf_exts) / sizeof(gltf_exts[0]);
	model_info.gltfExtensions     = gltf_exts;
	XrResult r = xrCreateRenderModelEXT(xr_session, &model_info, &source.render_model);
	if (XR_FAILED(r)) { log_warnf("%s [%s]", "xrCreateRenderModelEXT", openxr_string(r)); return false; }

	XrRenderModelPropertiesGetInfoEXT props_info       = { XR_TYPE_RENDER_MODEL_PROPERTIES_GET_INFO_EXT };
	XrRenderModelPropertiesEXT        props            = { XR_TYPE_RENDER_MODEL_PROPERTIES_EXT          };
	r = xrGetRenderModelPropertiesEXT(source.render_model, &props_info, &props);
	if (XR_FAILED(r)) { log_warnf("%s [%s]", "xrGetRenderModelPropertiesEXT", openxr_string(r)); xr_ext_render_model_source_destroy(&source); return false; }

	// Generate a unique asset name for this model, based on the cache ID.
	char name[128];
	XrUuidEXT uuid = props.cacheId;
	snprintf(name, sizeof(name), "sk/xr_model/%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x.glb",
		uuid.data[0 ], uuid.data[1 ], uuid.data[2 ], uuid.data[3 ],
		uuid.data[4 ], uuid.data[5 ], uuid.data[6 ], uuid.data[7 ],
		uuid.data[8 ], uuid.data[9 ], uuid.data[10], uuid.data[11],
		uuid.data[12], uuid.data[13], uuid.data[14], uuid.data[15]);
	source.model_asset = model_find(name); // Just in case it's loaded for some reason, this shouldn't really happen

	XrRenderModelAssetEXT            asset;
	XrRenderModelAssetCreateInfoEXT  asset_info = { XR_TYPE_RENDER_MODEL_ASSET_CREATE_INFO_EXT };
	asset_info.cacheId = props.cacheId;
	r = xrCreateRenderModelAssetEXT(xr_session, &asset_info, &asset);
	if (XR_FAILED(r)) { log_warnf("%s [%s]", "xrCreateRenderModelAssetEXT", openxr_string(r)); xr_ext_render_model_source_destroy(&source); return false; }

	// Load the GLTF itself
	if (source.model_asset == nullptr) {
		XrRenderModelAssetDataGetInfoEXT data_info = { XR_TYPE_RENDER_MODEL_ASSET_DATA_GET_INFO_EXT };
		XrRenderModelAssetDataEXT        data      = { XR_TYPE_RENDER_MODEL_ASSET_DATA_EXT };
		xrGetRenderModelAssetDataEXT(asset, &data_info, &data);
		data.bufferCapacityInput = data.bufferCountOutput;
		data.buffer              = sk_malloc_t(uint8_t, data.bufferCountOutput);
		r = xrGetRenderModelAssetDataEXT(asset, &data_info, &data);
		if (XR_FAILED(r)) { log_warnf("%s [%s]", "xrGetRenderModelAssetDataEXT", openxr_string(r)); xrDestroyRenderModelAssetEXT(asset); xr_ext_render_model_source_destroy(&source); return false; }

		source.model_asset = model_create_mem(name, data.buffer, data.bufferCountOutput);
		model_set_id(source.model_asset, name);

		sk_free(data.buffer);
	}

	// Find the animatable nodes on this model
	XrRenderModelAssetPropertiesGetInfoEXT asset_props_info = { XR_TYPE_RENDER_MODEL_ASSET_PROPERTIES_GET_INFO_EXT };
	XrRenderModelAssetPropertiesEXT        asset_props      = { XR_TYPE_RENDER_MODEL_ASSET_PROPERTIES_EXT };
	XrRenderModelAssetNodePropertiesEXT*   node_props       = sk_malloc_t(XrRenderModelAssetNodePropertiesEXT, props.animatableNodeCount);
	asset_props.nodePropertyCount = props.animatableNodeCount;
	asset_props.nodeProperties    = node_props;
	r = xrGetRenderModelAssetPropertiesEXT(asset, &asset_props_info, &asset_props);
	if (XR_FAILED(r)) { log_warnf("%s [%s]", "xrGetRenderModelAssetPropertiesEXT", openxr_string(r)); sk_free(node_props); xrDestroyRenderModelAssetEXT(asset); xr_ext_render_model_source_destroy(&source); return false; }
	source.anim_nodes      = sk_malloc_t(model_node_id, asset_props.nodePropertyCount);
	source.anim_node_count = asset_props.nodePropertyCount;
	for (uint32_t i = 0; i < asset_props.nodePropertyCount; i++) {
		source.anim_nodes[i] = model_node_find(source.model_asset, asset_props.nodeProperties[i].uniqueName);
	}

	sk_free(node_props);

	xrDestroyRenderModelAssetEXT(asset);

	int32_t source_idx = local.sources.add(source);

	// Now we've loaded the model, instantiate it for the user!
	return xr_ext_render_model_source_instantiate(source_idx, out_model);
}

///////////////////////////////////////////

XrRenderModelEXT xr_ext_render_model_get_model(const xr_render_model_t* model) {
	xr_render_model_source_t* source = &local.sources[model->asset_idx];
	return source->render_model;
}

///////////////////////////////////////////

void xr_ext_render_model_update(xr_render_model_t* ref_model) {
	xr_render_model_source_t* model = &local.sources[ref_model->asset_idx];

	XrRenderModelStateGetInfoEXT info = { XR_TYPE_RENDER_MODEL_STATE_GET_INFO_EXT };
	info.displayTime = xr_time;
	XrResult r = xrGetRenderModelStateEXT(model->render_model, &info, &ref_model->state_query);
	if (XR_FAILED(r)) { log_warnf("%s [%s]", "xrGetRenderModelStateEXT", openxr_string(r)); return; }

	for (uint32_t i = 0; i < ref_model->state_query.nodeStateCount; i++) {
		model_node_id node = model->anim_nodes[i];

		XrVector3f    p = ref_model->state_query.nodeStates[i].nodePose.position;
		XrQuaternionf r = ref_model->state_query.nodeStates[i].nodePose.orientation;
		model_node_set_transform_local(ref_model->model, node, matrix_trs({ p.x, p.y, p.z }, {r.x, r.y, r.z, r.w}));
		model_node_set_visible        (ref_model->model, node, ref_model->state_query.nodeStates[i].isVisible);
	}
}

///////////////////////////////////////////

void xr_ext_render_model_destroy(xr_render_model_t* ref_model) {
	model_release(ref_model->model);
	if (ref_model->space != XR_NULL_HANDLE) xrDestroySpace(ref_model->space);
	sk_free(ref_model->state_query.nodeStates);
	*ref_model = {};
}

///////////////////////////////////////////

void xr_ext_render_model_source_destroy(xr_render_model_source_t* ref_model) {
	model_release(ref_model->model_asset);
	if (ref_model->render_model != XR_NULL_HANDLE) xrDestroyRenderModelEXT(ref_model->render_model);
	sk_free(ref_model->anim_nodes);
	*ref_model = {};
}

///////////////////////////////////////////

bool xr_ext_render_model_source_instantiate(int32_t model_idx, xr_render_model_t* out_inst) {
	xr_render_model_source_t* model  = &local.sources[model_idx];
	xr_render_model_t  result = {};

	result.asset_idx = model_idx;
	result.model = model_copy(model->model_asset);

	XrRenderModelSpaceCreateInfoEXT space_info = { XR_TYPE_RENDER_MODEL_SPACE_CREATE_INFO_EXT };
	space_info.renderModel = model->render_model;
	XrResult r = xrCreateRenderModelSpaceEXT(xr_session, &space_info, &result.space);
	if (XR_FAILED(r)) { log_warnf("%s [%s]", "xrCreateRenderModelSpaceEXT", openxr_string(r)); xr_ext_render_model_destroy(&result); return false; }

	result.state_query = { XR_TYPE_RENDER_MODEL_STATE_EXT };
	result.state_query.nodeStateCount = model->anim_node_count;
	result.state_query.nodeStates     = sk_malloc_t(XrRenderModelNodeStateEXT, model->anim_node_count);

	*out_inst = result;
	return true;
}

} // namespace sk