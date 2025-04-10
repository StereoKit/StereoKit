/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#include "../openxr_platform.h"
#include "ext_management.h"
#include "time.h"

// There are two different time conversion extensions, one for Windows, and one
// for everything else. We're using some macros here to make the code a bit
// more linear.
#ifdef XR_USE_TIMESPEC
	#include <time.h>
	#define XR_TIME_EXTENSION XR_KHR_CONVERT_TIMESPEC_TIME_EXTENSION_NAME
	#define XR_EXT_FUNCTIONS( X )         \
		X(xrConvertTimespecTimeToTimeKHR) \
		X(xrConvertTimeToTimespecTimeKHR)
#else
	#define XR_TIME_EXTENSION XR_KHR_WIN32_CONVERT_PERFORMANCE_COUNTER_TIME_EXTENSION_NAME
	#define XR_EXT_FUNCTIONS( X )                    \
		X(xrConvertWin32PerformanceCounterToTimeKHR) \
		X(xrConvertTimeToWin32PerformanceCounterKHR )
#endif
OPENXR_DEFINE_FN_STATIC(XR_EXT_FUNCTIONS);

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

xr_system_ xr_ext_time_init    (void*);
void       xr_ext_time_shutdown(void*);

///////////////////////////////////////////

void xr_ext_time_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_TIME_EXTENSION;
	sys.func_initialize = { xr_ext_time_init };
	sys.func_shutdown   = { xr_ext_time_shutdown };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_time_init(void*) {
	// This extension is CRITICAL for SK's code, so if we fail to get
	// everything we need to work, we'll fail initialization!
	if (!backend_openxr_ext_enabled(XR_TIME_EXTENSION))
		return xr_system_fail_critical;

	// Load all extension functions
	OPENXR_LOAD_FN(XR_EXT_FUNCTIONS, xr_system_fail_critical);

	// A number of items use the xr_time, so lets get this ready as soon as we're
	// able.
	xr_time = xr_ext_time_acquire_time();

	return xr_system_succeed;
}

///////////////////////////////////////////

void xr_ext_time_shutdown(void*) {
	OPENXR_CLEAR_FN(XR_EXT_FUNCTIONS);
}

///////////////////////////////////////////

XrTime xr_ext_time_acquire_time() {
	XrTime result = {};
#ifdef XR_USE_TIMESPEC
	struct timespec time;
	if (clock_gettime(CLOCK_MONOTONIC, &time) != 0 ||
		XR_FAILED(xrConvertTimespecTimeToTimeKHR(xr_instance, &time, &result))) {
		log_warn("openxr_acquire_time failed to get current time!");
	}
#else
	LARGE_INTEGER time;
	if (!QueryPerformanceCounter(&time) ||
		XR_FAILED(xrConvertWin32PerformanceCounterToTimeKHR(xr_instance, &time, &result))) {
		log_warn("openxr_acquire_time failed to get current time!");
	}
#endif
	return result;
}

} // namespace sk