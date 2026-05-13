/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 * Copyright (c) 2026 Qualcomm Technologies, Inc.
 */

// This implements XR_FB_haptic_pcm and XR_FB_haptic_amplitude_envelope, which
// provide higher-fidelity haptic playback than the core XrHapticVibration:
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_FB_haptic_pcm
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_FB_haptic_amplitude_envelope

#include "fb_haptic.h"
#include "ext_management.h"

///////////////////////////////////////////

#define XR_EXT_FUNCTIONS( X ) \
	X(xrGetDeviceSampleRateFB)
OPENXR_DEFINE_FN_STATIC(XR_EXT_FUNCTIONS);

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

typedef struct xr_fb_haptic_state_t {
	bool pcm_available;
	bool envelope_available;
} xr_fb_haptic_state_t;
static xr_fb_haptic_state_t local = { };

///////////////////////////////////////////

xr_system_ xr_fb_haptic_initialize(void*);
void       xr_fb_haptic_shutdown  (void*);

///////////////////////////////////////////

void xr_fb_haptic_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_FB_HAPTIC_PCM_EXTENSION_NAME;
	sys.request_exts[sys.request_ext_count++] = XR_FB_HAPTIC_AMPLITUDE_ENVELOPE_EXTENSION_NAME;
	sys.evt_initialize = { xr_fb_haptic_initialize };
	sys.evt_shutdown   = { xr_fb_haptic_shutdown };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_fb_haptic_initialize(void*) {
	local.pcm_available      = backend_openxr_ext_enabled(XR_FB_HAPTIC_PCM_EXTENSION_NAME);
	local.envelope_available = backend_openxr_ext_enabled(XR_FB_HAPTIC_AMPLITUDE_ENVELOPE_EXTENSION_NAME);

	if (local.pcm_available) {
		// xrGetDeviceSampleRateFB is the only new function from XR_FB_haptic_pcm.
		// Both extensions chain into the existing core xrApplyHapticFeedback via
		// XrHapticBaseHeader, so they don't add other entry points.
		OPENXR_LOAD_FN_RETURN(XR_EXT_FUNCTIONS, xr_system_fail);
	}

	return (local.pcm_available || local.envelope_available)
		? xr_system_succeed
		: xr_system_fail;
}

///////////////////////////////////////////

void xr_fb_haptic_shutdown(void*) {
	OPENXR_CLEAR_FN(XR_EXT_FUNCTIONS);
	local = {};
}

///////////////////////////////////////////

bool xr_fb_haptic_pcm_available     () { return local.pcm_available; }
bool xr_fb_haptic_envelope_available() { return local.envelope_available; }

///////////////////////////////////////////

float xr_fb_haptic_pcm_get_sample_rate(const XrHapticActionInfo* info) {
	if (!local.pcm_available) return 0;
	XrDevicePcmSampleRateGetInfoFB rate = { XR_TYPE_DEVICE_PCM_SAMPLE_RATE_GET_INFO_FB };
	if (XR_FAILED(xrGetDeviceSampleRateFB(xr_session, info, &rate))) return 0;
	return rate.sampleRate;
}

} // namespace sk
