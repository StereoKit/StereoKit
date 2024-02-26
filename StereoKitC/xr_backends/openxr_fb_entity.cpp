/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2023-2024 Nick Klingensmith
 * Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
 */

#include "openxr_fb_entity.h"

#if defined(SK_XR_OPENXR)

#include "openxr_extensions.h"
#include <stdio.h>

///////////////////////////////////////////

using namespace sk;

enum entity_com_ {
	entity_com_locatable       = 1 << 0,
	entity_com_storable        = 1 << 1,
	entity_com_sharable        = 1 << 2,
	entity_com_bounded_2d      = 1 << 3,
	entity_com_bounded_3d      = 1 << 4,
	entity_com_semantic_labels = 1 << 5,
	entity_com_room_layout     = 1 << 6,
	entity_com_space_container = 1 << 7,
};
SK_MakeFlag(entity_com_);
const int entity_com_max = 8;

struct entity_evt_t {
	XrStructureType    type;
	XrAsyncRequestIdFB id;
	void*              context;
	void             (*callback)(const void* event_data, void* context);
};

struct entity_t {
	XrUuidEXT   uuid;
	XrSpace     space;
	entity_com_ components;
};

struct openxr_fb_entity_state_t {
	array_t<entity_evt_t> events;
};
static openxr_fb_entity_state_t* local = {};

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

entity_evt_t openxr_fb_entity_make_evt          (XrStructureType type, XrAsyncRequestIdFB id, void (*callback)(const void* event_data, void* context), void* context);
void         openxr_fb_entity_get_entities_async(void (*on_anchors)(const XrSpaceQueryResultFB* results, int32_t count));
void         openxr_fb_entity_on_event          (void* context, void* xrEventDataBuffer);

///////////////////////////////////////////

bool32_t openxr_fb_entity_init() {
	local = sk_malloc_zero_t(openxr_fb_entity_state_t, 1);
	if (!xr_ext_available.FB_spatial_entity) return false;

	// Spatial entity APIs pass a lot of messages back via this callback.
	backend_openxr_add_callback_poll_event(openxr_fb_entity_on_event, nullptr);

	log_diag("Requesting Anchors...");
	openxr_fb_entity_get_entities_async([](const XrSpaceQueryResultFB* anchors, int32_t count) {
		log_diagf("Received %d anchors", count);
		XrSpaceComponentTypeFB component_buffer[entity_com_max] = {};
		entity_t*              entities = sk_malloc_t(entity_t, count);

		for (int32_t i = 0; i < count; i++) {
			// format anchors[i].uuid.data as a 16 byte uuid, in the form of XXXX-XXXX-XXXX-XXXX
			log_diagf("Anchor %d: %08x-%08x-%08x-%08x", i,
				((uint32_t*)anchors[i].uuid.data)[0],
				((uint32_t*)anchors[i].uuid.data)[1],
				((uint32_t*)anchors[i].uuid.data)[2],
				((uint32_t*)anchors[i].uuid.data)[3]);

			uint32_t count  = 0;
			XrResult result = xr_extensions.xrEnumerateSpaceSupportedComponentsFB(anchors[i].space, entity_com_max, &count, component_buffer);
			if (XR_FAILED(result)) {
				log_warnf("%s failed: %s", "xrEnumerateSpaceSupportedComponentsFB", openxr_string(result));
				continue;
			}
			entity_com_ coms = (entity_com_)0;
			for (int32_t c = 0; c < count; c++) {
				switch (component_buffer[c]) {
					case XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB:       coms = coms | entity_com_locatable;       break;
					case XR_SPACE_COMPONENT_TYPE_STORABLE_FB:        coms = coms | entity_com_storable;        break;
					case XR_SPACE_COMPONENT_TYPE_SHARABLE_FB:        coms = coms | entity_com_sharable;        break;
					case XR_SPACE_COMPONENT_TYPE_BOUNDED_2D_FB:      coms = coms | entity_com_bounded_2d;      break;
					case XR_SPACE_COMPONENT_TYPE_BOUNDED_3D_FB:      coms = coms | entity_com_bounded_3d;      break;
					case XR_SPACE_COMPONENT_TYPE_SEMANTIC_LABELS_FB: coms = coms | entity_com_semantic_labels; break;
					case XR_SPACE_COMPONENT_TYPE_ROOM_LAYOUT_FB:     coms = coms | entity_com_room_layout;     break;
					case XR_SPACE_COMPONENT_TYPE_SPACE_CONTAINER_FB: coms = coms | entity_com_space_container; break;
				}
			}

			entities[i].uuid       = anchors[i].uuid;
			entities[i].space      = anchors[i].space;
			entities[i].components = coms;

			char caps[128] = {};
			if (coms & entity_com_locatable)       snprintf(caps, sizeof(caps), "%s loc", caps);
			if (coms & entity_com_storable)        snprintf(caps, sizeof(caps), "%s sto", caps);
			if (coms & entity_com_sharable)        snprintf(caps, sizeof(caps), "%s sha", caps);
			if (coms & entity_com_bounded_2d)      snprintf(caps, sizeof(caps), "%s b2d", caps);
			if (coms & entity_com_bounded_3d)      snprintf(caps, sizeof(caps), "%s b3d", caps);
			if (coms & entity_com_semantic_labels) snprintf(caps, sizeof(caps), "%s sem", caps);
			if (coms & entity_com_room_layout)     snprintf(caps, sizeof(caps), "%s rm", caps);
			if (coms & entity_com_space_container) snprintf(caps, sizeof(caps), "%s spc", caps);
			log_diagf("- %s", caps);
		}
	});

	return true;
}

///////////////////////////////////////////

void openxr_fb_entity_shutdown() {
	backend_openxr_remove_callback_poll_event(openxr_fb_entity_on_event);
	local->events.free();
	sk_free(local);
}

///////////////////////////////////////////

void openxr_fb_entity_get_entities_async(void (*on_anchors)(const XrSpaceQueryResultFB* results, int32_t count)) {
	if (!xr_ext_available.FB_spatial_entity_query) return;

	XrSpaceQueryInfoFB query = { XR_TYPE_SPACE_QUERY_INFO_FB };
	query.queryAction    = XR_SPACE_QUERY_ACTION_LOAD_FB;
	query.maxResultCount = 20;
	XrAsyncRequestIdFB req_id = {};
	XrResult           result = xr_extensions.xrQuerySpacesFB(xr_session, (XrSpaceQueryInfoBaseHeaderFB*)&query, &req_id);
	if (XR_FAILED(result)) {
		log_warnf("%s failed: %s", "xrQuerySpacesFB", openxr_string(result));
		return;
	}

	local->events.add(openxr_fb_entity_make_evt(XR_TYPE_EVENT_DATA_SPACE_QUERY_RESULTS_AVAILABLE_FB, req_id,
	[](const void* evt_data, void* context) {
		const XrEventDataSpaceQueryResultsAvailableFB* data = (const XrEventDataSpaceQueryResultsAvailableFB*)evt_data;

		XrSpaceQueryResultsFB space_query = { XR_TYPE_SPACE_QUERY_RESULTS_FB };
		XrResult result = xr_extensions.xrRetrieveSpaceQueryResultsFB(xr_session, data->requestId, &space_query);
		if (XR_FAILED(result)) {
			log_warnf("%s failed: %s", "xrRetrieveSpaceQueryResultsFB", openxr_string(result));
			return;
		}

		space_query.results             = sk_malloc_t(XrSpaceQueryResultFB, space_query.resultCountOutput);
		space_query.resultCapacityInput = space_query.resultCountOutput;
		result = xr_extensions.xrRetrieveSpaceQueryResultsFB(xr_session, data->requestId, &space_query);
		if (XR_FAILED(result)) {
			log_warnf("%s failed: %s", "xrRetrieveSpaceQueryResultsFB", openxr_string(result));
			return;
		}

		void (*callback)(const XrSpaceQueryResultFB*, int32_t) = (void (*)(const XrSpaceQueryResultFB *, int32_t))context;
		callback(space_query.results, space_query.resultCountOutput);

		sk_free(space_query.results);
	}, on_anchors));
}

///////////////////////////////////////////

void openxr_fb_entity_on_event(void* context, void* xrEventDataBuffer) {
	XrEventDataBuffer *evt = (XrEventDataBuffer*)xrEventDataBuffer;

	for (int32_t i = 0; i < local->events.count; i++)
	{
		if (evt->type != local->events[i].type) continue;

		const XrEventDataSpaceQueryResultsAvailableFB* data =
			(XrEventDataSpaceQueryResultsAvailableFB*)evt;
		if (data->requestId == local->events[i].id) {
			local->events[i].callback(evt, local->events[i].context);
			local->events.remove(i);
			i--;
		}
	}
}

///////////////////////////////////////////

entity_evt_t openxr_fb_entity_make_evt(XrStructureType type, XrAsyncRequestIdFB id, void (*callback)(const void* event_data, void* context), void* context) {
	entity_evt_t result = {};
	result.type     = type;
	result.id       = id;
	result.context  = context;
	result.callback = callback;
	return result;
}

///////////////////////////////////////////

}
#endif