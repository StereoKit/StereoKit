/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

 // This implements XR_EXT_future
 // https://registry.khronos.org/OpenXR/specs/1.1/html/xrspec.html#XR_EXT_future

#include "ext_management.h"
#include "../openxr.h"

#define XR_EXT_FUNCTIONS( X ) \
	X(xrPollFutureEXT)        \
	X(xrCancelFutureEXT)
OPENXR_DEFINE_FN_STATIC(XR_EXT_FUNCTIONS);

struct xr_future_callback_t {
	XrFutureEXT future;
	void*       context;
	void      (*on_finish)(void* context, XrFutureEXT future);
};
struct xr_future_state_t {
	bool available;
	array_t<xr_future_callback_t> callbacks;
};
static xr_future_state_t local = { };

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

xr_system_ xr_ext_future_initialize(void*);
void       xr_ext_future_shutdown  (void*);
void       xr_ext_future_step_begin(void*);

///////////////////////////////////////////

void xr_ext_future_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_EXT_FUTURE_EXTENSION_NAME;
	sys.evt_initialize = { xr_ext_future_initialize };
	sys.evt_shutdown   = { xr_ext_future_shutdown   };
	sys.evt_step_begin = { xr_ext_future_step_begin };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_future_initialize(void*) {
	// Check if we got our extension
	if (!backend_openxr_ext_enabled(XR_EXT_FUTURE_EXTENSION_NAME))
		return xr_system_fail;

	// Load all extension functions
	OPENXR_LOAD_FN_RETURN(XR_EXT_FUNCTIONS, xr_system_fail);

	local.available = true;
	return xr_system_succeed;
}

///////////////////////////////////////////

void xr_ext_future_shutdown(void*) {
	local.callbacks.free();
	local = {};

	OPENXR_CLEAR_FN(XR_EXT_FUNCTIONS);
}

///////////////////////////////////////////

void xr_ext_future_step_begin(void*) {
	XrFuturePollInfoEXT   info   = { XR_TYPE_FUTURE_POLL_INFO_EXT };
	XrFuturePollResultEXT result = { XR_TYPE_FUTURE_POLL_RESULT_EXT };
	for (int32_t i = 0; i < local.callbacks.count; i++) {
		const xr_future_callback_t* cb = &local.callbacks[i];
		info.future = cb->future;
		if (xrPollFutureEXT(xr_instance, &info, &result) == XR_SUCCESS && result.state == XR_FUTURE_STATE_READY_EXT) {
			cb->on_finish(cb->context, cb->future);
			local.callbacks.remove(i);
			i--;
		}
	}
}

///////////////////////////////////////////

void xr_ext_future_on_finish(XrFutureEXT future, void(*on_finish)(void* context, XrFutureEXT future), void* context) {
	if (!local.available)
		return;
	local.callbacks.add({ future, context, on_finish });
}

///////////////////////////////////////////

bool xr_ext_future_check(XrFutureEXT future) {
	if (!local.available || future == XR_NULL_HANDLE)
		return false;

	XrFuturePollInfoEXT   info   = { XR_TYPE_FUTURE_POLL_INFO_EXT   };
	XrFuturePollResultEXT result = { XR_TYPE_FUTURE_POLL_RESULT_EXT };
	info.future = future;
	return
		xrPollFutureEXT(xr_instance, &info, &result) == XR_SUCCESS &&
		result.state == XR_FUTURE_STATE_READY_EXT;
}

}