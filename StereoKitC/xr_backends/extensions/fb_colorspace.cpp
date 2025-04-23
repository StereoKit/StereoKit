/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#include "fb_colorspace.h"
#include "ext_management.h"

 ///////////////////////////////////////////

#define XR_EXT_FUNCTIONS( X ) \
	X(xrSetColorSpaceFB)
OPENXR_DEFINE_FN_STATIC(XR_EXT_FUNCTIONS);

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

typedef struct xr_fb_colorspace_state_t {
	bool available;
} xr_fb_colorspace_state_t;
static xr_fb_colorspace_state_t local = { };

///////////////////////////////////////////

xr_system_ xr_ext_fb_colorspace_initialize(void*);
void       xr_ext_fb_colorspace_shutdown  (void*);

///////////////////////////////////////////

void xr_ext_fb_colorspace_register() {
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_FB_COLOR_SPACE_EXTENSION_NAME;
	sys.evt_initialize = { xr_ext_fb_colorspace_initialize };
	sys.evt_shutdown   = { xr_ext_fb_colorspace_shutdown };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_fb_colorspace_initialize(void*) {
	if (!backend_openxr_ext_enabled(XR_FB_COLOR_SPACE_EXTENSION_NAME))
		return xr_system_fail;

	OPENXR_LOAD_FN_RETURN(XR_EXT_FUNCTIONS, xr_system_fail);

	local.available = true;

	return xr_system_succeed;
}

///////////////////////////////////////////

void xr_ext_fb_colorspace_shutdown(void*) {
	OPENXR_CLEAR_FN(XR_EXT_FUNCTIONS);
}

///////////////////////////////////////////

void xr_ext_fb_colorspace_set_for(tex_format_ format) {
	if (!local.available) return;

	const char*    colorspace_str = "XR_COLOR_SPACE_REC709_FB";
	XrColorSpaceFB colorspace     =  XR_COLOR_SPACE_REC709_FB;

	// Maybe?
	if (format != tex_format_bgra32 && format != tex_format_rgba32) {
		colorspace_str = "XR_COLOR_SPACE_REC2020_FB";
		colorspace     =  XR_COLOR_SPACE_REC2020_FB;
	}

	if (XR_SUCCEEDED(xrSetColorSpaceFB(xr_session, colorspace)))
		log_diagf("Set color space to <~grn>%s<~clr>", colorspace_str);
}

} // namespace sk