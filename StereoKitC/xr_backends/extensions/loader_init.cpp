/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

// This implements XR_KHR_loader_init and XR_KHR_loader_init_android
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_KHR_loader_init
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_KHR_loader_init_android

#include "loader_init.h"
#include "../openxr_platform.h"
#include "../../log.h"

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

xr_system_ xr_ext_loader_init() {
	// This handles both XR_KHR_loader_init and XR_KHR_loader_init_android.
	// While these extensions are separate, the spec mixes them up a bit! I
	// haven't seen it required on non-android systems, so currently we're
	// restricting this specifically to Android.

	// These extensions _must_ happen before normal extensions are loaded, so
	// this code is called outside the normal EXT system, and at the beginning
	// of OpenXR code.
#ifdef XR_USE_PLATFORM_ANDROID
	// xrInitializeLoaderKHR is somewhat unique in that it requires no
	// extension check, and can be loaded before OpenXR is initialized! The
	// only check required is if the function can successfully be loaded.
	PFN_xrInitializeLoaderKHR xrInitializeLoaderKHR;
	xrGetInstanceProcAddr(XR_NULL_HANDLE, "xrInitializeLoaderKHR", (PFN_xrVoidFunction*)(&xrInitializeLoaderKHR));
	if (xrInitializeLoaderKHR == NULL)
		return xr_system_fail; // Failure here means the ext is just not available

	XrLoaderInitInfoAndroidKHR init_android = { XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR };
	init_android.applicationVM      = backend_android_get_java_vm ();
	init_android.applicationContext = backend_android_get_activity();

	if (XR_FAILED(xrInitializeLoaderKHR((XrLoaderInitInfoBaseHeaderKHR*)&init_android))) {
		log_fail_reasonf(90, log_warning, "Failed to initialize OpenXR loader");
		return xr_system_fail_critical;
	}
	return xr_system_succeed;
#else
	return xr_system_fail;
#endif
}

} // namespace sk