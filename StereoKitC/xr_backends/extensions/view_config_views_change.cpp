/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 * Copyright (c) 2026 Qualcomm Technologies, Inc.
 */

// This implements XR_EXT_view_configuration_views_change
// https://registry.khronos.org/OpenXR/specs/1.1/html/xrspec.html#XR_EXT_view_configuration_views_change
//
// The runtime uses this to tell us that the recommended view configuration
// values (e.g. recommendedImageRectWidth) have changed - for example when the
// user adjusts a resolution slider in the runtime. We just flag the affected
// view configuration type as dirty; openxr_view.cpp consumes the flag and
// re-runs openxr_display_swapchain_update, which re-enumerates the views and
// reallocates swapchains only if the dimensions actually changed.

#include "view_config_views_change.h"
#include "ext_management.h"

///////////////////////////////////////////

namespace sk {

typedef struct xr_view_config_views_change_state_t {
	bool                    available;
	// The number of distinct view configuration types is tiny (primary plus a
	// handful of secondary configs), so a small fixed set is plenty.
	XrViewConfigurationType dirty[8];
	int32_t                 dirty_count;
} xr_view_config_views_change_state_t;
static xr_view_config_views_change_state_t local = { };

///////////////////////////////////////////

xr_system_ xr_ext_view_config_views_change_init(void*);
void       xr_ext_view_config_views_change_shutdown(void*);
void       xr_ext_view_config_views_change_poll(void*, XrEventDataBuffer* event);

///////////////////////////////////////////

void xr_ext_view_config_views_change_register() {
	local = {};

	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_EXT_VIEW_CONFIGURATION_VIEWS_CHANGE_EXTENSION_NAME;
	sys.evt_initialize = { xr_ext_view_config_views_change_init     };
	sys.evt_shutdown   = { xr_ext_view_config_views_change_shutdown };
	sys.evt_poll       = { (void (*)(void*, void*))xr_ext_view_config_views_change_poll };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_view_config_views_change_init(void*) {
	if (!backend_openxr_ext_enabled(XR_EXT_VIEW_CONFIGURATION_VIEWS_CHANGE_EXTENSION_NAME))
		return xr_system_fail;

	// No function pointers to load - this extension is purely an event plus a
	// relaxation of xrEnumerateViewConfigurationViews' constness.
	local.available = true;
	return xr_system_succeed;
}

///////////////////////////////////////////

void xr_ext_view_config_views_change_shutdown(void*) {
	local = {};
}

///////////////////////////////////////////

void xr_ext_view_config_views_change_poll(void*, XrEventDataBuffer* event) {
	if (event->type != XR_TYPE_EVENT_DATA_VIEW_CONFIGURATION_VIEWS_CHANGED_EXT)
		return;

	XrEventDataViewConfigurationViewsChangedEXT* changed = (XrEventDataViewConfigurationViewsChangedEXT*)event;

	// Only care about events for our system.
	if (changed->systemId != xr_system_id)
		return;

	// Flag this view configuration type as dirty, avoiding duplicates.
	for (int32_t i = 0; i < local.dirty_count; i++)
		if (local.dirty[i] == changed->viewConfigurationType) return;
	if (local.dirty_count < (int32_t)_countof(local.dirty))
		local.dirty[local.dirty_count++] = changed->viewConfigurationType;
}

///////////////////////////////////////////

bool xr_ext_view_config_views_change_consume(XrViewConfigurationType type) {
	if (!local.available) return false;

	for (int32_t i = 0; i < local.dirty_count; i++) {
		if (local.dirty[i] == type) {
			// Remove by swapping in the last entry.
			local.dirty[i] = local.dirty[local.dirty_count - 1];
			local.dirty_count--;
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////

} // namespace sk
