/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

// This implements XR_KHR_android_thread_settings
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_KHR_android_thread_settings

#include "android_thread.h"
#include "ext_management.h"
#include "../openxr_platform.h"

 ///////////////////////////////////////////

#ifdef XR_USE_PLATFORM_ANDROID

#include <unistd.h> // gettid

#define XR_EXT_FUNCTIONS( X ) \
	X(xrSetAndroidApplicationThreadKHR)
OPENXR_DEFINE_FN_STATIC(XR_EXT_FUNCTIONS);

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

xr_system_ xr_ext_android_thread_initialize(void*);
void       xr_ext_android_thread_shutdown  (void*);

///////////////////////////////////////////

void xr_ext_android_thread_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_KHR_ANDROID_THREAD_SETTINGS_EXTENSION_NAME;
	sys.evt_initialize = { xr_ext_android_thread_initialize };
	sys.evt_shutdown   = { xr_ext_android_thread_shutdown };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_android_thread_initialize(void*) {
	if (!backend_openxr_ext_enabled(XR_KHR_ANDROID_THREAD_SETTINGS_EXTENSION_NAME))
		return xr_system_fail;

	OPENXR_LOAD_FN_RETURN(XR_EXT_FUNCTIONS, xr_system_fail);

	return xr_system_succeed;
}

///////////////////////////////////////////

void xr_ext_android_thread_shutdown(void*) {
	OPENXR_CLEAR_FN(XR_EXT_FUNCTIONS);
}

///////////////////////////////////////////

void xr_ext_android_thread_set_type(xr_thread_type_ type) {
	if (!xrSetAndroidApplicationThreadKHR)
		return;
	XrAndroidThreadTypeKHR xr_type;
	switch (type) {
	case xr_thread_type_app_main:    xr_type = XR_ANDROID_THREAD_TYPE_APPLICATION_MAIN_KHR;   break;
	case xr_thread_type_app_work:    xr_type = XR_ANDROID_THREAD_TYPE_APPLICATION_WORKER_KHR; break;
	case xr_thread_type_render_main: xr_type = XR_ANDROID_THREAD_TYPE_RENDERER_MAIN_KHR;      break;
	case xr_thread_type_render_work: xr_type = XR_ANDROID_THREAD_TYPE_RENDERER_WORKER_KHR;    break;
	default: return;
	}
	xrSetAndroidApplicationThreadKHR(xr_session, xr_type, gettid());
}

} // namespace sk

#else

namespace sk {
void xr_ext_android_thread_register() {}
void xr_ext_android_thread_set_type(xr_thread_type_) {}
}

#endif