/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 */

#include "openxr_fb_entity.h"

#if defined(SK_XR_OPENXR)

#include "openxr_extensions.h"

///////////////////////////////////////////

using namespace sk;

struct entity_evt_t {
	XrStructureType    type;
	XrAsyncRequestIdFB id;
	void*              context;
	void             (*callback)(const void* event_data, void* context);
};

struct openxr_fb_entity_state_t {
	array_t<entity_evt_t> events;
};
static openxr_fb_entity_state_t* local = {};

///////////////////////////////////////////

entity_evt_t _make_evt(XrStructureType type, XrAsyncRequestIdFB id, void (*callback)(const void* event_data, void* context), void* context = nullptr) {
	entity_evt_t result = {};
	result.type     = type;
	result.id       = id;
	result.context  = context;
	result.callback = callback;
	return result;
}
void openxr_fb_entity_get_anchors_async(void (*on_anchors)(const XrSpaceQueryResultFB* results, int32_t count));

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

bool32_t openxr_fb_entity_init() {
	local = sk_malloc_t(openxr_fb_entity_state_t, 1);
	if (!xr_ext_available.FB_spatial_entity) return false;

	// Spatial entity APIs pass a lot of messages back via this callback.
	backend_openxr_add_callback_poll_event(openxr_fb_entity_on_event, nullptr);

	openxr_fb_entity_get_anchors_async([](const XrSpaceQueryResultFB* anchors, int32_t count) {
		for (int32_t i = 0; i < count; i++) {
			log_diagf("Anchor %d: %s", i, anchors[i].uuid.data);
		}
	});

	return true;
}

///////////////////////////////////////////

void openxr_fb_entity_shutdown() {
	backend_openxr_remove_callback_poll_event(openxr_fb_entity_on_event);
	sk_free(local);
}

///////////////////////////////////////////

void openxr_fb_entity_get_anchors_async(void (*on_anchors)(const XrSpaceQueryResultFB *results, int32_t count)) {
	if (!xr_ext_available.FB_spatial_entity_query) return;

	XrSpaceQueryInfoBaseHeaderFB query  = { XR_TYPE_SPACE_QUERY_INFO_FB };
	XrAsyncRequestIdFB           req_id = {};
	XrResult result = xr_extensions.xrQuerySpacesFB(xr_session, &query, &req_id);
	if (XR_FAILED(result)) {
		log_warnf("xrQuerySpacesFB failed: %s", openxr_string(result));
		return;
	}

	local->events.add(_make_evt(XR_TYPE_EVENT_DATA_SPACE_QUERY_RESULTS_AVAILABLE_FB, req_id,
	[](const void* evt_data, void* context) {
		const XrEventDataSpaceQueryResultsAvailableFB* data = (const XrEventDataSpaceQueryResultsAvailableFB*)evt_data;

		XrSpaceQueryResultsFB space_query = { XR_TYPE_SPACE_QUERY_RESULTS_FB };
		xr_extensions.xrRetrieveSpaceQueryResultsFB(xr_session, data->requestId, &space_query);

		space_query.results             = sk_malloc_t(XrSpaceQueryResultFB, space_query.resultCountOutput);
		space_query.resultCapacityInput = space_query.resultCountOutput;
		xr_extensions.xrRetrieveSpaceQueryResultsFB(xr_session, data->requestId, &space_query);

		void (*callback)(const XrSpaceQueryResultFB*, int32_t) = (void (*)(const XrSpaceQueryResultFB *, int32_t))context;
		callback(space_query.results, space_query.resultCountOutput);

		sk_free(space_query.results);
	}, on_anchors));
}

///////////////////////////////////////////

void openxr_fb_entity_on_event(void* context, void* xrEventDataBuffer) {
	XrEventDataBuffer *evt = (XrEventDataBuffer*)xrEventDataBuffer;

	for (size_t i = 0; i < local->events.count; i++)
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

}
#endif