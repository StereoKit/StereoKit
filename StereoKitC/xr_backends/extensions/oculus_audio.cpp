/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#include "../openxr_platform.h"
#include "ext_management.h"
#include "oculus_audio.h"

#ifdef XR_USE_PLATFORM_WIN32

#include "../../systems/audio.h"

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

xr_system_ xr_ext_oculus_audio_init(void*);

///////////////////////////////////////////

void xr_ext_oculus_audio_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_OCULUS_AUDIO_DEVICE_GUID_EXTENSION_NAME;
	sys.evt_initialize = { xr_ext_oculus_audio_init };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_oculus_audio_init(void*) {
	// Check if we got our extension
	if (!backend_openxr_ext_enabled(XR_OCULUS_AUDIO_DEVICE_GUID_EXTENSION_NAME))
		return xr_system_fail;

	// Load all extension functions, we can do this locally since it's such a
	// simple extension!
	#define XR_EXT_FUNCTIONS( X )           \
		X(xrGetAudioOutputDeviceGuidOculus) \
		X(xrGetAudioInputDeviceGuidOculus )
	OPENXR_DEFINE_FN     (XR_EXT_FUNCTIONS);
	OPENXR_LOAD_FN_RETURN(XR_EXT_FUNCTIONS, xr_system_fail);

	// All we really need to do here is just find out what audio devices the XR
	// runtime recommends, and register that with our audio system!
	wchar_t device_guid[128];
	if (XR_SUCCEEDED(xrGetAudioOutputDeviceGuidOculus(xr_instance, device_guid))) audio_set_default_device_out(device_guid);
	if (XR_SUCCEEDED(xrGetAudioInputDeviceGuidOculus (xr_instance, device_guid))) audio_set_default_device_in (device_guid);

	return xr_system_succeed;
}

///////////////////////////////////////////

} // namespace sk

#else

namespace sk {
void xr_ext_oculus_audio_register() {}
}

#endif