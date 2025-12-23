/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

// This implements XR_KHR_vulkan_enable for OpenXR Vulkan graphics binding
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_KHR_vulkan_enable

#include "../openxr_platform.h"
#include "ext_management.h"
#include "graphics.h"

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

	// Load our function manually since recursive macros get ugly, and we only
	// have the one!
	if (XR_FAILED(xrGetInstanceProcAddr(xr_instance, NAME_xrGetGraphicsRequirementsKHR, (PFN_xrVoidFunction*)((PFN_xrGetGraphicsRequirementsKHR)(&xrGetGraphicsRequirementsKHR))))) {
		return xr_system_fail_critical;
	}

	// OpenXR wants to ensure apps are using the correct graphics device
	XrGraphicsRequirements requirement = { XR_TYPE_GRAPHICS_REQUIREMENTS };
	XrResult result = xrGetGraphicsRequirementsKHR(xr_instance, xr_system_id, &requirement);
	if (XR_FAILED(result)) {
		log_infof("%s [%s]", "xrGetGraphicsRequirementsKHR", openxr_string(result));
		return xr_system_fail_critical;
	}

	log_diagf("OpenXR requires Vulkan v%d.%d.%d - v%d.%d.%d",
		XR_VERSION_MAJOR(requirement.minApiVersionSupported), XR_VERSION_MINOR(requirement.minApiVersionSupported), XR_VERSION_PATCH(requirement.minApiVersionSupported),
		XR_VERSION_MAJOR(requirement.maxApiVersionSupported), XR_VERSION_MINOR(requirement.maxApiVersionSupported), XR_VERSION_PATCH(requirement.maxApiVersionSupported));

	// Set up Vulkan graphics binding using sk_renderer
	local.gfx_binding = { XR_TYPE_GRAPHICS_BINDING };
	local.gfx_binding.instance         = skr_get_vk_instance();
	local.gfx_binding.physicalDevice   = skr_get_vk_physical_device();
	local.gfx_binding.device           = skr_get_vk_device();
	local.gfx_binding.queueFamilyIndex = skr_get_vk_graphics_queue_family();
	local.gfx_binding.queueIndex       = 0;
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
	// Vulkan doesn't use LUID like D3D11
	return nullptr;
}

} // namespace sk