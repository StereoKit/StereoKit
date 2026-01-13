// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2025-2026 Nick Klingensmith
// Copyright (c) 2025-2026 Qualcomm Technologies, Inc.

// This implements OpenXR Vulkan graphics binding with two separate paths:
// - XR_KHR_vulkan_enable2 (preferred) - OpenXR creates VkInstance/VkDevice
// - XR_KHR_vulkan_enable (fallback)   - Application creates VkInstance/VkDevice
//
// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_KHR_vulkan_enable
// https://registry.khronos.org/OpenXR/specs/1.1/html/xrspec.html#XR_KHR_vulkan_enable2

#include "../openxr_platform.h"
#include "ext_management.h"
#include "vulkan_enable.h"
#include "../../sk_memory.h"
#include "../../libraries/stref.h"
#include "../../libraries/array.h"

///////////////////////////////////////////
// Function pointers
///////////////////////////////////////////

#define XR_VULKAN_ENABLE_FN(X)              \
	X(xrGetVulkanInstanceExtensionsKHR)     \
	X(xrGetVulkanDeviceExtensionsKHR)       \
	X(xrGetVulkanGraphicsDeviceKHR)         \
	X(xrGetVulkanGraphicsRequirementsKHR)
OPENXR_DEFINE_FN_STATIC(XR_VULKAN_ENABLE_FN);

#define XR_VULKAN_ENABLE2_FN(X)             \
	X(xrCreateVulkanInstanceKHR)            \
	X(xrCreateVulkanDeviceKHR)              \
	X(xrGetVulkanGraphicsDevice2KHR)        \
	X(xrGetVulkanGraphicsRequirements2KHR)
OPENXR_DEFINE_FN_STATIC(XR_VULKAN_ENABLE2_FN);

///////////////////////////////////////////
// State
///////////////////////////////////////////

typedef struct xr_vulkan_enable_state_t {
	XrGraphicsBindingVulkanKHR      gfx_binding;

	char*                           instance_ext_storage;
	array_t<const char*>            instance_extensions;
	char*                           device_ext_storage;
	array_t<const char*>            device_extensions;
} xr_vulkan_enable_state_t;
static xr_vulkan_enable_state_t local = {};

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////

static bool                 xr_vulkan_enable_init              ();
static skr_device_request_t xr_vulkan_enable_device_callback   (void* vk_instance, void* user_data);
static bool                 xr_vulkan_enable2_init             ();
static void*                xr_vulkan_enable2_instance_callback(skr_instance_create_info_t* create_info, void* user_data);
static skr_device_request_t xr_vulkan_enable2_device_callback  (void* vk_instance, void* user_data);
static xr_system_           xr_ext_vulkan_enable_pre_session   (void*, XrBaseHeader* ref_session_info);
static void                 xr_ext_vulkan_enable_shutdown      (void*);

///////////////////////////////////////////
// Public API
///////////////////////////////////////////

void xr_ext_vulkan_enable_register() {
	bool has_enable  = ext_management_ext_available(XR_KHR_VULKAN_ENABLE_EXTENSION_NAME);
	bool has_enable2 = ext_management_ext_available(XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME);

	xr_system_t sys = {};
	sys.required        = true;
	sys.evt_pre_session = { xr_ext_vulkan_enable_pre_session };
	sys.evt_shutdown    = { xr_ext_vulkan_enable_shutdown    };

	// Prefer enable2 when available, fall back to enable, default to enable2
	if (has_enable && !has_enable2) { sys.request_exts[sys.request_ext_count++] = XR_KHR_VULKAN_ENABLE_EXTENSION_NAME; }
	else                            { sys.request_exts[sys.request_ext_count++] = XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME; }
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

bool xr_ext_vulkan_enable_setup_skr(skr_settings_t* ref_settings, const char*** out_instance_exts, uint32_t* out_instance_ext_count) {
	if (backend_openxr_ext_enabled(XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME)) { 
		if (!xr_vulkan_enable2_init()) return false;

		ref_settings->instance_create_callback  = xr_vulkan_enable2_instance_callback;
		ref_settings->instance_create_user_data = &local;
		ref_settings->device_init_callback      = xr_vulkan_enable2_device_callback;
		ref_settings->device_init_user_data     = &local;

	} else if (backend_openxr_ext_enabled(XR_KHR_VULKAN_ENABLE_EXTENSION_NAME )) {
		if (!xr_vulkan_enable_init ()) return false;

		ref_settings->device_init_callback      = xr_vulkan_enable_device_callback;
		ref_settings->device_init_user_data     = &local;

	} else {
		log_errf("No Vulkan graphics binding extension available! Need %s or %s", XR_KHR_VULKAN_ENABLE_EXTENSION_NAME, XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME);
		return false;
	}

	// Return required instance extensions
	*out_instance_exts      = local.instance_extensions.data;
	*out_instance_ext_count = (uint32_t)local.instance_extensions.count;
	return true;
}

///////////////////////////////////////////
// Helpers
///////////////////////////////////////////

static void parse_extension_string(const char* ext_str, array_t<const char*>* out_array) {
	if (!ext_str || !ext_str[0]) return;

	const char* start = ext_str;
	const char* curr  = ext_str;
	while (*curr) {
		if (*curr == ' ') {
			if (curr > start) out_array->add(start);
			while (*curr == ' ') curr++;
			start = curr;
		} else {
			curr++;
		}
	}
	if (curr > start) out_array->add(start);

	// Null-terminate each extension by replacing spaces with nulls
	char* mutable_str = (char*)ext_str;
	while (*mutable_str) {
		if (*mutable_str == ' ') *mutable_str = '\0';
		mutable_str++;
	}
}

///////////////////////////////////////////

static void log_vulkan_requirements(const XrGraphicsRequirementsVulkanKHR* requirements) {
	log_diagf("OpenXR requires Vulkan v%d.%d.%d - v%d.%d.%d",
		XR_VERSION_MAJOR(requirements->minApiVersionSupported),
		XR_VERSION_MINOR(requirements->minApiVersionSupported),
		XR_VERSION_PATCH(requirements->minApiVersionSupported),
		XR_VERSION_MAJOR(requirements->maxApiVersionSupported),
		XR_VERSION_MINOR(requirements->maxApiVersionSupported),
		XR_VERSION_PATCH(requirements->maxApiVersionSupported));
}

///////////////////////////////////////////
// XR_KHR_vulkan_enable path
///////////////////////////////////////////

static bool xr_vulkan_enable_init() {
	OPENXR_LOAD_FN_RETURN(XR_VULKAN_ENABLE_FN, false);

	XrGraphicsRequirementsVulkanKHR vulkan_requirements = { XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR };
	xr_check(xrGetVulkanGraphicsRequirementsKHR(xr_instance, xr_system_id, &vulkan_requirements),
		"xrGetVulkanGraphicsRequirementsKHR");
	log_vulkan_requirements(&vulkan_requirements);

	// Query required Vulkan instance extensions
	uint32_t ext_size = 0;
	xrGetVulkanInstanceExtensionsKHR(xr_instance, xr_system_id, 0, &ext_size, nullptr);
	if (ext_size > 0) {
		local.instance_ext_storage = sk_malloc_t(char, ext_size);
		xrGetVulkanInstanceExtensionsKHR(xr_instance, xr_system_id, ext_size, &ext_size, local.instance_ext_storage);
		parse_extension_string(local.instance_ext_storage, &local.instance_extensions);

		log_diagf("OpenXR requires %d Vulkan instance extension(s)", local.instance_extensions.count);
		for (int32_t i = 0; i < local.instance_extensions.count; i++) {
			log_diagf("  - %s", local.instance_extensions[i]);
		}
	}

	return true;
}

///////////////////////////////////////////

static skr_device_request_t xr_vulkan_enable_device_callback(void* vk_instance, void* user_data) {
	skr_device_request_t request = {};

	VkPhysicalDevice phys_device = VK_NULL_HANDLE;
	XrResult result = xrGetVulkanGraphicsDeviceKHR(xr_instance, xr_system_id, (VkInstance)vk_instance, &phys_device);
	if (XR_FAILED(result)) {
		log_errf("xrGetVulkanGraphicsDeviceKHR failed [%s]", openxr_string(result));
		return request;
	}

	request.physical_device = phys_device;

	// Query required Vulkan device extensions
	uint32_t ext_size = 0;
	xrGetVulkanDeviceExtensionsKHR(xr_instance, xr_system_id, 0, &ext_size, nullptr);
	if (ext_size > 0) {
		local.device_ext_storage = sk_malloc_t(char, ext_size);
		xrGetVulkanDeviceExtensionsKHR(xr_instance, xr_system_id, ext_size, &ext_size, local.device_ext_storage);
		parse_extension_string(local.device_ext_storage, &local.device_extensions);

		request.required_device_extensions      = local.device_extensions.data;
		request.required_device_extension_count = (uint32_t)local.device_extensions.count;

		log_diagf("OpenXR requires %d Vulkan device extension(s)", local.device_extensions.count);
		for (int32_t i = 0; i < local.device_extensions.count; i++) {
			log_diagf("  - %s", local.device_extensions[i]);
		}
	}

	return request;
}

///////////////////////////////////////////
// XR_KHR_vulkan_enable2 path
///////////////////////////////////////////

static bool xr_vulkan_enable2_init() {
	OPENXR_LOAD_FN_RETURN(XR_VULKAN_ENABLE2_FN, false);

	XrGraphicsRequirementsVulkanKHR vulkan_requirements = { XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR };
	xr_check(xrGetVulkanGraphicsRequirements2KHR(xr_instance, xr_system_id, &vulkan_requirements),
		"xrGetVulkanGraphicsRequirements2KHR");
	log_vulkan_requirements(&vulkan_requirements);

	// enable2 doesn't use xrGetVulkanInstanceExtensionsKHR - OpenXR adds required
	// extensions internally when xrCreateVulkanInstanceKHR is called
	return true;
}

///////////////////////////////////////////

static void* xr_vulkan_enable2_instance_callback(skr_instance_create_info_t* create_info, void* user_data) {
	XrVulkanInstanceCreateInfoKHR xr_create_info = { XR_TYPE_VULKAN_INSTANCE_CREATE_INFO_KHR };
	xr_create_info.systemId               = xr_system_id;
	xr_create_info.pfnGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)create_info->get_instance_proc_addr;
	xr_create_info.vulkanCreateInfo       = (const VkInstanceCreateInfo*)create_info->instance_create_info;
	xr_create_info.vulkanAllocator        = nullptr;

	VkInstance vk_instance = VK_NULL_HANDLE;
	VkResult   vk_result   = VK_SUCCESS;
	xr_check_ret(xrCreateVulkanInstanceKHR(xr_instance, &xr_create_info, &vk_instance, &vk_result),
		"xrCreateVulkanInstanceKHR", nullptr);

	if (vk_result != VK_SUCCESS) {
		log_errf("xrCreateVulkanInstanceKHR: Vulkan instance creation failed [0x%X]", vk_result);
		return nullptr;
	}

	log_diag("VkInstance created via xrCreateVulkanInstanceKHR");
	return vk_instance;
}

///////////////////////////////////////////

static skr_device_request_t xr_vulkan_enable2_device_callback(void* vk_instance, void* user_data) {
	skr_device_request_t request = {};

	XrVulkanGraphicsDeviceGetInfoKHR get_info = { XR_TYPE_VULKAN_GRAPHICS_DEVICE_GET_INFO_KHR };
	get_info.systemId       = xr_system_id;
	get_info.vulkanInstance = (VkInstance)vk_instance;

	VkPhysicalDevice phys_device = VK_NULL_HANDLE;
	XrResult result = xrGetVulkanGraphicsDevice2KHR(xr_instance, &get_info, &phys_device);
	xr_check_ret(result, "xrGetVulkanGraphicsDevice2KHR", request);

	request.physical_device = phys_device;
	return request;
}

///////////////////////////////////////////
// Lifecycle
///////////////////////////////////////////

static xr_system_ xr_ext_vulkan_enable_pre_session(void*, XrBaseHeader* ref_session_info) {
	// Set up Vulkan graphics binding
	local.gfx_binding = { XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR };
	local.gfx_binding.instance         = skr_get_vk_instance();
	local.gfx_binding.physicalDevice   = skr_get_vk_physical_device();
	local.gfx_binding.device           = skr_get_vk_device();
	local.gfx_binding.queueFamilyIndex = skr_get_vk_graphics_queue_family();
	local.gfx_binding.queueIndex       = 0;
	xr_insert_next(ref_session_info, (XrBaseHeader*)&local.gfx_binding);

	return xr_system_succeed;
}

///////////////////////////////////////////

static void xr_ext_vulkan_enable_shutdown(void*) {
	sk_free(local.instance_ext_storage);
	sk_free(local.device_ext_storage);
	local.instance_extensions.free();
	local.device_extensions.free();
	local = {};

	OPENXR_CLEAR_FN(XR_VULKAN_ENABLE_FN);
	OPENXR_CLEAR_FN(XR_VULKAN_ENABLE2_FN);
}

///////////////////////////////////////////

} // namespace sk
