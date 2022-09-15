#pragma once

#include "../platforms/platform_utils.h"
#if defined(SK_XR_OPENXR)

#include "openxr.h"
#include "../stereokit.h"
#include "../libraries/array.h"

#if defined(XR_USE_GRAPHICS_API_D3D11)
	#define WIN32_LEAN_AND_MEAN
	#include <d3d11.h>
	#define XR_GFX_EXTENSION XR_KHR_D3D11_ENABLE_EXTENSION_NAME
	#define XrSwapchainImage XrSwapchainImageD3D11KHR
	#define XR_TYPE_SWAPCHAIN_IMAGE XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR
	#define XrGraphicsRequirements XrGraphicsRequirementsD3D11KHR
	#define XR_TYPE_GRAPHICS_REQUIREMENTS XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR
	#define xrGetGraphicsRequirementsKHR xrGetD3D11GraphicsRequirementsKHR
	#define PFN_xrGetGraphicsRequirementsKHR PFN_xrGetD3D11GraphicsRequirementsKHR
	#define NAME_xrGetGraphicsRequirementsKHR "xrGetD3D11GraphicsRequirementsKHR"
	#define XrGraphicsBinding XrGraphicsBindingD3D11KHR
	#define XR_TYPE_GRAPHICS_BINDING XR_TYPE_GRAPHICS_BINDING_D3D11_KHR

#elif defined(XR_USE_PLATFORM_WIN32) && defined(XR_USE_GRAPHICS_API_OPENGL)
	#include <windows.h>
	#include <unknwn.h>
	#define XR_GFX_EXTENSION XR_KHR_OPENGL_ENABLE_EXTENSION_NAME
	#define XrSwapchainImage XrSwapchainImageOpenGLKHR
	#define XR_TYPE_SWAPCHAIN_IMAGE XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR
	#define XrGraphicsRequirements XrGraphicsRequirementsOpenGLKHR
	#define XR_TYPE_GRAPHICS_REQUIREMENTS XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR
	#define xrGetGraphicsRequirementsKHR xrGetOpenGLGraphicsRequirementsKHR
	#define PFN_xrGetGraphicsRequirementsKHR PFN_xrGetOpenGLGraphicsRequirementsKHR
	#define NAME_xrGetGraphicsRequirementsKHR "xrGetOpenGLGraphicsRequirementsKHR"
	#define XrGraphicsBinding XrGraphicsBindingOpenGLWin32KHR
	#define XR_TYPE_GRAPHICS_BINDING XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR

#elif defined(XR_USE_PLATFORM_XLIB) && defined(XR_USE_GRAPHICS_API_OPENGL)
	#include<X11/X.h>
	#include<X11/Xlib.h>
	#include<GL/gl.h>
	#include<GL/glx.h>
	#include<GL/glu.h>
	#define XR_GFX_EXTENSION XR_KHR_OPENGL_ENABLE_EXTENSION_NAME
	#define XrSwapchainImage XrSwapchainImageOpenGLKHR
	#define XR_TYPE_SWAPCHAIN_IMAGE XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR
	#define XrGraphicsRequirements XrGraphicsRequirementsOpenGLKHR
	#define XR_TYPE_GRAPHICS_REQUIREMENTS XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR
	#define xrGetGraphicsRequirementsKHR xrGetOpenGLGraphicsRequirementsKHR
	#define PFN_xrGetGraphicsRequirementsKHR PFN_xrGetOpenGLGraphicsRequirementsKHR
	#define NAME_xrGetGraphicsRequirementsKHR "xrGetOpenGLGraphicsRequirementsKHR"
	#define XrGraphicsBinding XrGraphicsBindingOpenGLXlibKHR
	#define XR_TYPE_GRAPHICS_BINDING XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR

#elif defined(XR_USE_GRAPHICS_API_VULKAN)
	#define XR_GFX_EXTENSION XR_KHR_VULKAN_ENABLE_EXTENSION_NAME
	#define XrSwapchainImage XrSwapchainImageVulkanKHR
	#define XR_TYPE_SWAPCHAIN_IMAGE XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR
	#define XrGraphicsRequirements XrGraphicsRequirementsVulkanKHR
	#define XR_TYPE_GRAPHICS_REQUIREMENTS XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR
	#define PFN_xrGetGraphicsRequirementsKHR PFN_xrGetVulkanGraphicsRequirementsKHR
	#define NAME_xrGetGraphicsRequirementsKHR "xrGetVulkanGraphicsRequirementsKHR"
	#define XrGraphicsBinding XrGraphicsBindingVulkanKHR
	#define XR_TYPE_GRAPHICS_BINDING XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR

#elif defined(XR_USE_GRAPHICS_API_OPENGL_ES)
	#include <EGL/egl.h>
	#define XR_GFX_EXTENSION XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME
	#define XrSwapchainImage XrSwapchainImageOpenGLESKHR
	#define XR_TYPE_SWAPCHAIN_IMAGE XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR
	#define XrGraphicsRequirements XrGraphicsRequirementsOpenGLESKHR
	#define XR_TYPE_GRAPHICS_REQUIREMENTS XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_ES_KHR
	#define xrGetGraphicsRequirementsKHR xrGetOpenGLESGraphicsRequirementsKHR
	#define PFN_xrGetGraphicsRequirementsKHR PFN_xrGetOpenGLESGraphicsRequirementsKHR
	#define NAME_xrGetGraphicsRequirementsKHR "xrGetOpenGLESGraphicsRequirementsKHR"

	#if defined(SK_OS_ANDROID)
		#include <android/native_activity.h>
		#define XrGraphicsBinding XrGraphicsBindingOpenGLESAndroidKHR
		#define XR_TYPE_GRAPHICS_BINDING XR_TYPE_GRAPHICS_BINDING_OPENGL_ES_ANDROID_KHR

	#elif defined(SK_OS_LINUX)
		#define XrGraphicsBinding XrGraphicsBindingEGLMNDX
		#define XR_TYPE_GRAPHICS_BINDING XR_TYPE_GRAPHICS_BINDING_EGL_MNDX

	#endif

#endif

#include <openxr/openxr_platform.h>

namespace sk {

#pragma warning( push )
#pragma warning( disable: 4127 ) // conditional expression is constant

///////////////////////////////////////////

#if defined(SK_OS_WINDOWS_UWP)
#define EXT_AVAILABLE_UWP true
#else
#define EXT_AVAILABLE_UWP false
#endif
#if defined(SK__DEBUG)
#define EXT_AVAILABLE_DEBUG true
#else
#define EXT_AVAILABLE_DEBUG false
#endif
#if defined(SK_OS_ANDROID)
#define EXT_AVAILABLE_ANDROID true
#else
#define EXT_AVAILABLE_ANDROID false
#endif
#if defined(SK_OS_LINUX)
#define EXT_AVAILABLE_LINUX true
#else
#define EXT_AVAILABLE_LINUX false
#endif

///////////////////////////////////////////
// Extension availability
///////////////////////////////////////////

// Extensions that are available for all supported platforms
#define FOR_EACH_EXT_ALL(_) \
	_(KHR_composition_layer_depth,       true) \
	_(EXT_hand_tracking,                 true) \
	_(EXT_palm_pose,                     true) \
	_(EXT_eye_gaze_interaction,          true) \
	_(FB_color_space,                    true) \
	_(OCULUS_audio_device_guid,          true) \
	_(MSFT_unbounded_reference_space,    true) \
	_(MSFT_hand_interaction,             true) \
	_(MSFT_hand_tracking_mesh,           true) \
	_(MSFT_spatial_anchor,               true) \
	_(MSFT_spatial_graph_bridge,         true) \
	_(MSFT_secondary_view_configuration, true) \
	_(MSFT_first_person_observer,        true) \
	_(MSFT_scene_understanding,          true) \
	_(EXT_hp_mixed_reality_controller,   true) \
	_(EXTX_overlay,                      true)

// UWP platform only
#define FOR_EACH_EXT_UWP(_) \
	_(MSFT_perception_anchor_interop, EXT_AVAILABLE_UWP)

// Android platform only
#define FOR_EACH_EXT_ANDROID(_) \
	_(KHR_android_create_instance, EXT_AVAILABLE_ANDROID)

#if defined(SKG_LINUX_EGL)
// Linux platform only
#define FOR_EACH_EXT_LINUX(_) \
	_(MNDX_egl_enable, EXT_AVAILABLE_LINUX)
#else
#define FOR_EACH_EXT_LINUX(_)
#endif

// Debug builds only
#define FOR_EACH_EXT_DEBUG(_) \
	_(EXT_debug_utils, EXT_AVAILABLE_DEBUG)

///////////////////////////////////////////
// Extension functions
///////////////////////////////////////////

#define FOR_EACH_EXTENSION_FUNCTION(_)           \
	_(xrCreateSpatialAnchorMSFT)                 \
	_(xrCreateSpatialAnchorSpaceMSFT)            \
	_(xrDestroySpatialAnchorMSFT)                \
	_(xrCreateSceneObserverMSFT)                 \
	_(xrDestroySceneObserverMSFT)                \
	_(xrCreateSceneMSFT)                         \
	_(xrDestroySceneMSFT)                        \
	_(xrComputeNewSceneMSFT)                     \
	_(xrGetSceneComputeStateMSFT)                \
	_(xrGetSceneComponentsMSFT)                  \
	_(xrLocateSceneComponentsMSFT)               \
	_(xrEnumerateSceneComputeFeaturesMSFT)       \
	_(xrGetSceneMeshBuffersMSFT)                 \
	_(xrGetVisibilityMaskKHR)                    \
	_(xrCreateHandTrackerEXT)                    \
	_(xrDestroyHandTrackerEXT)                   \
	_(xrLocateHandJointsEXT)                     \
	_(xrCreateHandMeshSpaceMSFT)                 \
	_(xrUpdateHandMeshMSFT)                      \
	_(xrEnumerateColorSpacesFB)                  \
	_(xrSetColorSpaceFB)                         \
	_(xrCreateSpatialGraphNodeSpaceMSFT)         \
	_(xrCreateDebugUtilsMessengerEXT)            \
	_(xrDestroyDebugUtilsMessengerEXT)

#if defined(_WIN32)
#define FOR_EACH_PLATFORM_FUNCTION(_)                \
	_(xrConvertWin32PerformanceCounterToTimeKHR)     \
	_(xrConvertTimeToWin32PerformanceCounterKHR)     \
	_(xrCreateSpatialAnchorFromPerceptionAnchorMSFT) \
	_(xrGetAudioOutputDeviceGuidOculus)              \
	_(xrGetAudioInputDeviceGuidOculus)
#else
#define FOR_EACH_PLATFORM_FUNCTION(_)  \
	_(xrConvertTimespecTimeToTimeKHR ) \
	_(xrConvertTimeToTimespecTimeKHR )
#endif

///////////////////////////////////////////

#define DEFINE_PROC_MEMBER(name) PFN_##name name;
struct XrExtTable {
	FOR_EACH_EXTENSION_FUNCTION(DEFINE_PROC_MEMBER);
	FOR_EACH_PLATFORM_FUNCTION(DEFINE_PROC_MEMBER);
	PFN_xrGetGraphicsRequirementsKHR xrGetGraphicsRequirementsKHR;
};
extern XrExtTable xr_extensions;

#define GET_INSTANCE_PROC_ADDRESS(name) (void)xrGetInstanceProcAddr(instance, #name, (PFN_xrVoidFunction*)((PFN_##name*)(&result.name)));
inline XrExtTable xrCreateExtensionTable(XrInstance instance) {
	XrExtTable result = {};
	FOR_EACH_EXTENSION_FUNCTION(GET_INSTANCE_PROC_ADDRESS);
	FOR_EACH_PLATFORM_FUNCTION(GET_INSTANCE_PROC_ADDRESS);
	(void)xrGetInstanceProcAddr(instance, NAME_xrGetGraphicsRequirementsKHR, (PFN_xrVoidFunction*)((PFN_xrGetGraphicsRequirementsKHR)(&result.xrGetGraphicsRequirementsKHR)));
	return result;
}

#undef DEFINE_PROC_MEMBER
#undef GET_INSTANCE_PROC_ADDRESS
#undef FOR_EACH_EXTENSION_FUNCTION
#undef FOR_EACH_PLATFORM_FUNCTION
#undef FOR_EACH_GRAPHICS_FUNCTION

///////////////////////////////////////////

#define DEFINE_EXT_INFO(name, available) bool name;
typedef struct XrExtInfo {
	bool gfx_extension;
	bool time_extension;
	FOR_EACH_EXT_ALL    (DEFINE_EXT_INFO);
	FOR_EACH_EXT_UWP    (DEFINE_EXT_INFO);
	FOR_EACH_EXT_ANDROID(DEFINE_EXT_INFO);
	FOR_EACH_EXT_LINUX  (DEFINE_EXT_INFO);
	FOR_EACH_EXT_DEBUG  (DEFINE_EXT_INFO);
} XrExtInfo;
extern XrExtInfo xr_ext_available;

#define CHECK_EXT(name, available) else if (!minimum_exts && available && strcmp("XR_"#name, exts[i].extensionName) == 0) {xr_ext_available.name = true; result.add("XR_"#name);}
#define CHECK_NAME(name, available) else if (strcmp("XR_"#name, exts[i].extensionName) == 0) {xr_ext_available.name = true;}
inline array_t<const char *> openxr_list_extensions(array_t<const char*> extra_exts, bool minimum_exts, void (*on_available)(const char *name)) {
	array_t<const char *> result = {};

	// Enumerate the list of extensions available on the system
	uint32_t ext_count = 0;
	if (XR_FAILED(xrEnumerateInstanceExtensionProperties(nullptr, 0, &ext_count, nullptr)))
		return result;
	XrExtensionProperties* exts = sk_malloc_t(XrExtensionProperties, ext_count);
	for (uint32_t i = 0; i < ext_count; i++) exts[i] = { XR_TYPE_EXTENSION_PROPERTIES };
	xrEnumerateInstanceExtensionProperties(nullptr, ext_count, &ext_count, exts);

	// Identify which of these we want, mark them as available, and put them
	// in the final list.
	for (uint32_t i = 0; i < ext_count; i++) {
		if      (strcmp(XR_GFX_EXTENSION,  exts[i].extensionName) == 0) { xr_ext_available.gfx_extension  = true; result.add(XR_GFX_EXTENSION);  }
		else if (strcmp(XR_TIME_EXTENSION, exts[i].extensionName) == 0) { xr_ext_available.time_extension = true; result.add(XR_TIME_EXTENSION); }
		FOR_EACH_EXT_ALL    (CHECK_EXT)
		FOR_EACH_EXT_UWP    (CHECK_EXT)
		FOR_EACH_EXT_ANDROID(CHECK_EXT)
		FOR_EACH_EXT_LINUX  (CHECK_EXT)
		FOR_EACH_EXT_DEBUG  (CHECK_EXT)
		else {
			bool found = false;
			for (int32_t e = 0; e < extra_exts.count; e++) {
				if (strcmp(extra_exts[e], exts[i].extensionName) == 0) {
					if (false) {}
					FOR_EACH_EXT_ALL    (CHECK_NAME)
					FOR_EACH_EXT_UWP    (CHECK_NAME)
					FOR_EACH_EXT_ANDROID(CHECK_NAME)
					FOR_EACH_EXT_LINUX  (CHECK_NAME)
					FOR_EACH_EXT_DEBUG  (CHECK_NAME)
					result.add(extra_exts[e]);
					found = true;
					break;
				}
			}
			if (!found && on_available != nullptr) { on_available(exts[i].extensionName); }
		}
	}

	sk_free(exts);
	return result;
}

#undef CHECK_EXT
#undef CHECK_NAME
#undef DEFINE_EXT_INFO
#undef EXT_AVAILABLE_UWP
#undef EXT_AVAILABLE_ANDROID
#undef EXT_AVAILABLE_DEBUG
#undef FOR_EACH_EXT_ALL
#undef FOR_EACH_EXT_UWP
#undef FOR_EACH_EXT_ANDROID
#undef FOR_EACH_EXT_LINUX
#undef FOR_EACH_EXT_DEBUG

#pragma warning( pop )
}

#endif