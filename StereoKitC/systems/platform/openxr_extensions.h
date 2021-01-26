#pragma once

#include "openxr.h"
#include "../../stereokit.h"
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#define FOR_EACH_EXTENSION_FUNCTION(_)           \
	_(xrCreateSpatialAnchorMSFT)                 \
	_(xrCreateSpatialAnchorSpaceMSFT)            \
	_(xrDestroySpatialAnchorMSFT)                \
	_(xrCreateSpatialAnchorFromPerceptionAnchorMSFT) \
	_(xrGetVisibilityMaskKHR)                    \
	_(xrCreateHandTrackerEXT)                    \
	_(xrDestroyHandTrackerEXT)                   \
	_(xrLocateHandJointsEXT)                     \
	_(xrCreateSpatialGraphNodeSpaceMSFT)         \
	_(xrCreateDebugUtilsMessengerEXT)            \
	_(xrDestroyDebugUtilsMessengerEXT)

#if defined(XR_USE_GRAPHICS_API_D3D11)
#define FOR_EACH_GRAPHICS_FUNCTION(_) _(xrGetD3D11GraphicsRequirementsKHR)
#elif defined(XR_USE_GRAPHICS_API_OPENGL)
#define FOR_EACH_GRAPHICS_FUNCTION(_) _(xrGetOpenGLGraphicsRequirementsKHR)
#elif defined(XR_USE_GRAPHICS_API_OPENGL_ES)
#define FOR_EACH_GRAPHICS_FUNCTION(_) _(xrGetOpenGLESGraphicsRequirementsKHR)
#elif defined(XR_USE_GRAPHICS_API_VULKAN)
#define FOR_EACH_GRAPHICS_FUNCTION(_) _(xrGetVulkanGraphicsRequirementsKHR)
#endif

#if defined(_WIN32)
#define FOR_EACH_PLATFORM_FUNCTION(_)            \
	_(xrConvertWin32PerformanceCounterToTimeKHR) \
	_(xrConvertTimeToWin32PerformanceCounterKHR) \

#else
#define FOR_EACH_PLATFORM_FUNCTION(_)  \
	_(xrConvertTimespecTimeToTimeKHR ) \
	_(xrConvertTimeToTimespecTimeKHR ) \

#endif

#define GET_INSTANCE_PROC_ADDRESS(name) (void)xrGetInstanceProcAddr(instance, #name, (PFN_xrVoidFunction*)((PFN_##name*)(&result.name)));
#define DEFINE_PROC_MEMBER(name) PFN_##name name;

struct XrExtTable {
	FOR_EACH_EXTENSION_FUNCTION(DEFINE_PROC_MEMBER);
	FOR_EACH_PLATFORM_FUNCTION(DEFINE_PROC_MEMBER);
	FOR_EACH_GRAPHICS_FUNCTION(DEFINE_PROC_MEMBER);
};

inline XrExtTable xrCreateExtensionTable(XrInstance instance) {
	XrExtTable result = {};
	FOR_EACH_EXTENSION_FUNCTION(GET_INSTANCE_PROC_ADDRESS);
	FOR_EACH_PLATFORM_FUNCTION(GET_INSTANCE_PROC_ADDRESS);
	FOR_EACH_GRAPHICS_FUNCTION(GET_INSTANCE_PROC_ADDRESS);
	return result;
}

#undef DEFINE_PROC_MEMBER
#undef GET_INSTANCE_PROC_ADDRESS
#undef FOR_EACH_EXTENSION_FUNCTION
#undef FOR_EACH_PLATFORM_FUNCTION
#undef FOR_EACH_GRAPHICS_FUNCTION