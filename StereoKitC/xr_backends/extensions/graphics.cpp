/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

// This implements XR_KHR_d3d11_enable, XR_KHR_opengl_enable,
// XR_KHR_opengl_es_enable, and XR_MNDX_egl_enable
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_KHR_D3D11_enable
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_KHR_opengl_enable
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_KHR_opengl_es_enable
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_MNDX_egl_enable

#include "../openxr_platform.h"
#include "ext_management.h"
#include "graphics.h"
#include "sk_gpu.h"

// Define our function manually since recursive macros get ugly, and we only
// have the one!
PFN_xrGetGraphicsRequirementsKHR xrGetGraphicsRequirementsKHR;

typedef struct xr_graphics_state_t {
	XrGraphicsRequirements luid_requirement;
	XrGraphicsBinding      gfx_binding;
} xr_graphics_state_t;
static xr_graphics_state_t local = { };

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

xr_system_ xr_ext_graphics_pre_session(void*, XrBaseHeader *ref_session_info);
void       xr_ext_graphics_shutdown   (void*);

///////////////////////////////////////////

void xr_ext_graphics_register() {
	xr_system_t sys = {};
	sys.required = true;
	sys.request_exts[sys.request_ext_count++] = XR_GFX_EXTENSION;
#ifdef XR_USE_PLATFORM_EGL
	sys.request_exts[sys.request_ext_count++] = XR_MNDX_EGL_ENABLE_EXTENSION_NAME;
#endif
	sys.evt_pre_session = { xr_ext_graphics_pre_session };
	sys.evt_shutdown    = { xr_ext_graphics_shutdown    };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_graphics_pre_session(void*, XrBaseHeader* ref_session_info) {
	// Check if we got our extension
	if (!backend_openxr_ext_enabled(XR_GFX_EXTENSION)) {
		log_errf("Couldn't load required extension [%s]", XR_GFX_EXTENSION);
		return xr_system_fail_critical;
	}

#ifdef XR_USE_PLATFORM_EGL
	if (!backend_openxr_ext_enabled(XR_MNDX_EGL_ENABLE_EXTENSION_NAME)) {
		log_errf("StereoKit requires the XR_MNDX_egl_enable extension for GL on Linux!");
		return xr_system_fail_critical;
	}
#endif

	// Load our function manually since recursive macros get ugly, and we only
	// have the one!
	if (XR_FAILED(xrGetInstanceProcAddr(xr_instance, NAME_xrGetGraphicsRequirementsKHR, (PFN_xrVoidFunction*)((PFN_xrGetGraphicsRequirementsKHR)(&xrGetGraphicsRequirementsKHR))))) {
		return xr_system_fail_critical;
	}


	// OpenXR wants to ensure apps are using the correct LUID, so this MUST be
	// called before xrCreateSession. StereoKit needs the LUID much earlier
	// for graphics initialization though, so this request isn't doing much.
	XrGraphicsRequirements requirement = { XR_TYPE_GRAPHICS_REQUIREMENTS };
	XrResult result = xrGetGraphicsRequirementsKHR(xr_instance, xr_system_id, &requirement);
	if (XR_FAILED(result)) {
		log_infof("%s [%s]", "xrGetGraphicsRequirementsKHR", openxr_string(result));
		return xr_system_fail_critical;
	}

	void *luid = nullptr;
#ifdef XR_USE_GRAPHICS_API_D3D11
	luid = (void *)&requirement.adapterLuid;
#elif defined XR_USE_GRAPHICS_API_OPENGL_ES
	log_diagf("OpenXR requires GLES v%d.%d.%d - v%d.%d.%d",
		XR_VERSION_MAJOR(requirement.minApiVersionSupported), XR_VERSION_MINOR(requirement.minApiVersionSupported), XR_VERSION_PATCH(requirement.minApiVersionSupported),
		XR_VERSION_MAJOR(requirement.maxApiVersionSupported), XR_VERSION_MINOR(requirement.maxApiVersionSupported), XR_VERSION_PATCH(requirement.maxApiVersionSupported));
#endif

	// gfx_binding is part of the local state so its lifetime lasts until
	// session initialization.
	local.gfx_binding = { XR_TYPE_GRAPHICS_BINDING };
	skg_platform_data_t platform = skg_get_platform_data();
#ifdef XR_USE_PLATFORM_XLIB
	local.gfx_binding.xDisplay    =  (Display*     )platform._x_display;
	local.gfx_binding.visualid    = *(uint32_t*    )platform._visual_id;
	local.gfx_binding.glxFBConfig =  (GLXFBConfig  )platform._glx_fb_config;
	local.gfx_binding.glxDrawable = *((GLXDrawable*)platform._glx_drawable);
	local.gfx_binding.glxContext  =  (GLXContext   )platform._glx_context;
#elif defined XR_USE_GRAPHICS_API_OPENGL
	local.gfx_binding.hDC         = (HDC  )platform._gl_hdc;
	local.gfx_binding.hGLRC       = (HGLRC)platform._gl_hrc;
#elif defined XR_USE_GRAPHICS_API_OPENGL_ES
	#ifdef XR_USE_PLATFORM_EGL
		local.gfx_binding.getProcAddress = eglGetProcAddress;
	#endif
	local.gfx_binding.display     = (EGLDisplay)platform._egl_display;
	local.gfx_binding.config      = (EGLConfig )platform._egl_config;
	local.gfx_binding.context     = (EGLContext)platform._egl_context;
#elif defined XR_USE_GRAPHICS_API_D3D11
	local.gfx_binding.device = (ID3D11Device*)platform._d3d11_device;
#endif
	xr_insert_next(ref_session_info, (XrBaseHeader*)&local.gfx_binding);

	return xr_system_succeed;
}

///////////////////////////////////////////

void xr_ext_graphics_shutdown(void*) {
	local = {};

	xrGetGraphicsRequirementsKHR = nullptr;
}

///////////////////////////////////////////

void* xr_ext_graphics_get_luid() {
	local.luid_requirement = { XR_TYPE_GRAPHICS_REQUIREMENTS };
#ifdef XR_USE_GRAPHICS_API_D3D11
	// Get an extension function, and check it for our requirements
	if (XR_FAILED(xrGetInstanceProcAddr(xr_instance, "xrGetD3D11GraphicsRequirementsKHR", (PFN_xrVoidFunction*)(&xrGetGraphicsRequirementsKHR))) ||
		XR_FAILED(xrGetGraphicsRequirementsKHR(xr_instance, xr_system_id, &local.luid_requirement))) {
		xrDestroyInstance(xr_instance);
		return nullptr;
	}

	return (void*)&local.luid_requirement.adapterLuid;
#else
	return nullptr;
#endif
}

} // namespace sk