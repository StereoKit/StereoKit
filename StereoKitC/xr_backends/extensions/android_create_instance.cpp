/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#include "android_create_instance.h"
#include "ext_management.h"
#include "../openxr_platform.h"
#include "../../log.h"

///////////////////////////////////////////

#ifdef XR_USE_PLATFORM_ANDROID

typedef struct android_create_instance_state_t {
	XrInstanceCreateInfoAndroidKHR android_info;
} android_create_instance_state_t;
static android_create_instance_state_t local = { };

namespace sk {

xr_system_ xr_ext_android_create_instance_pre_instance(void*, XrBaseHeader* ref_instance_create_info);

///////////////////////////////////////////

void xr_ext_android_create_instance_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_KHR_ANDROID_CREATE_INSTANCE_EXTENSION_NAME;
	sys.evt_pre_instance = { xr_ext_android_create_instance_pre_instance };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_android_create_instance_pre_instance(void*, XrBaseHeader* ref_instance_create_info) {
	// create_info must be declared in a long lasting scope! The memory must
	// remain alive at least until the instance is created!
	local.android_info = { XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR };
	local.android_info.applicationVM       = backend_android_get_java_vm ();
	local.android_info.applicationActivity = backend_android_get_activity();
	xr_insert_next(ref_instance_create_info, (XrBaseHeader*)&local.android_info);
	return xr_system_succeed;
}

} // namespace sk

#else

namespace sk {
void xr_ext_android_create_instance_register() {}
}

#endif