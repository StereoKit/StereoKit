#pragma once

#include "../platforms/platform.h"
#if defined(SK_XR_OPENXR)

#include "openxr.h"
#include "../stereokit.h"
#include "../libraries/array.h"
#include "../libraries/stref.h"

#if defined(XR_USE_GRAPHICS_API_D3D11)
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
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
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
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
	#include<GL/glx.h>
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
#if defined(SK_DEBUG)
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
	_(EXT_hand_tracking_data_source,     true) \
	_(EXT_hand_interaction,              true) \
	_(EXT_palm_pose,                     true) \
	_(EXT_eye_gaze_interaction,          true) \
	_(EXT_local_floor,                   true) \
	_(FB_color_space,                    true) \
	_(FB_spatial_entity,                 true) \
	_(OCULUS_audio_device_guid,          true) \
	_(MSFT_unbounded_reference_space,    true) \
	_(MSFT_hand_tracking_mesh,           true) \
	_(MSFT_hand_interaction,             true) \
	_(MSFT_spatial_anchor,               true) \
	_(MSFT_spatial_anchor_persistence,   true) \
	_(MSFT_spatial_graph_bridge,         true) \
	_(MSFT_secondary_view_configuration, true) \
	_(MSFT_first_person_observer,        true) \
	_(MSFT_scene_understanding,          true) \
	_(BD_controller_interaction,         true) \
	_(EXT_hp_mixed_reality_controller,   true) \
	_(EXTX_overlay,                      true)

// UWP platform only
#define FOR_EACH_EXT_UWP(_) \
	_(MSFT_perception_anchor_interop, EXT_AVAILABLE_UWP)

// Android platform only
#define FOR_EACH_EXT_ANDROID(_) \
	_(KHR_android_create_instance, EXT_AVAILABLE_ANDROID) \
	_(KHR_android_thread_settings, EXT_AVAILABLE_ANDROID)

// Linux platform only
#define FOR_EACH_EXT_LINUX(_) \
	_(MNDX_egl_enable, EXT_AVAILABLE_LINUX)

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
	_(xrCreateSpatialAnchorStoreConnectionMSFT)  \
	_(xrDestroySpatialAnchorStoreConnectionMSFT) \
	_(xrPersistSpatialAnchorMSFT)                \
	_(xrEnumeratePersistedSpatialAnchorNamesMSFT)\
	_(xrCreateSpatialAnchorFromPersistedNameMSFT)\
	_(xrUnpersistSpatialAnchorMSFT)              \
	_(xrClearSpatialAnchorStoreMSFT)             \
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
	_(xrTryGetPerceptionAnchorFromSpatialAnchorMSFT) \
	_(xrGetAudioOutputDeviceGuidOculus)              \
	_(xrGetAudioInputDeviceGuidOculus)
#elif defined(SK_OS_ANDROID)
#define FOR_EACH_PLATFORM_FUNCTION(_)  \
	_(xrConvertTimespecTimeToTimeKHR ) \
	_(xrConvertTimeToTimespecTimeKHR ) \
	_(xrSetAndroidApplicationThreadKHR)
#else
#define FOR_EACH_PLATFORM_FUNCTION(_)  \
	_(xrConvertTimespecTimeToTimeKHR ) \
	_(xrConvertTimeToTimespecTimeKHR )
#endif

///////////////////////////////////////////

#define DEFINE_PROC_MEMBER(name) PFN_##name name;
struct xr_ext_table_t {
	FOR_EACH_EXTENSION_FUNCTION(DEFINE_PROC_MEMBER);
	FOR_EACH_PLATFORM_FUNCTION(DEFINE_PROC_MEMBER);
	PFN_xrGetGraphicsRequirementsKHR xrGetGraphicsRequirementsKHR;
};
extern xr_ext_table_t xr_extensions;

#define GET_INSTANCE_PROC_ADDRESS(name) (void)xrGetInstanceProcAddr(instance, #name, (PFN_xrVoidFunction*)((PFN_##name*)(&result.name)));
inline xr_ext_table_t openxr_create_extension_table(XrInstance instance) {
	xr_ext_table_t result = {};
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

typedef enum xr_state_ {
	// EXT is not available on this system
	xr_ext_unavailable =  0,
	// EXT is available and used by StereoKit
	xr_ext_active      =  1,
	// EXT is available, but rejected by StereoKit due to some issue
	xr_ext_rejected    = -1,
	// EXT is available, but has reported that it won't work for this session
	xr_ext_disabled    = -2,
} xr_state_;

#define DEFINE_EXT_INFO(name, available) xr_state_ name;
typedef struct xr_ext_info_t {
	xr_state_ gfx_extension;
	xr_state_ time_extension;
	FOR_EACH_EXT_ALL    (DEFINE_EXT_INFO);
	FOR_EACH_EXT_UWP    (DEFINE_EXT_INFO);
	FOR_EACH_EXT_ANDROID(DEFINE_EXT_INFO);
	FOR_EACH_EXT_LINUX  (DEFINE_EXT_INFO);
	FOR_EACH_EXT_DEBUG  (DEFINE_EXT_INFO);
} xr_ext_info_t;
extern xr_ext_info_t xr_ext;

inline bool openxr_list_extensions(array_t<const char*> extra_exts, array_t<const char*> exclude_exts, bool minimum_exts, array_t<const char*>* ref_all_available_exts, array_t<const char*>* ref_request_exts) {
	// Enumerate the list of extensions available on the system
	uint32_t ext_count = 0;
	if (XR_FAILED(xrEnumerateInstanceExtensionProperties(nullptr, 0, &ext_count, nullptr)))
		return false;
	XrExtensionProperties* exts = sk_malloc_t(XrExtensionProperties, ext_count);
	for (uint32_t i = 0; i < ext_count; i++) exts[i] = { XR_TYPE_EXTENSION_PROPERTIES };
	xrEnumerateInstanceExtensionProperties(nullptr, ext_count, &ext_count, exts);

	qsort(exts, ext_count, sizeof(XrExtensionProperties), [](const void* a, const void* b) {
		return strcmp(((XrExtensionProperties*)a)->extensionName, ((XrExtensionProperties*)b)->extensionName); });

	// See which of the available extensions we want to use
	for (uint32_t i = 0; i < ext_count; i++) {
		// These extensions are required for StereoKit to function
		if (strcmp(exts[i].extensionName, XR_GFX_EXTENSION)  == 0) { ref_request_exts->add(XR_GFX_EXTENSION);  continue; }
		if (strcmp(exts[i].extensionName, XR_TIME_EXTENSION) == 0) { ref_request_exts->add(XR_TIME_EXTENSION); continue; }
#if defined(SK_OS_ANDROID)
		if (strcmp(exts[i].extensionName, "XR_KHR_android_create_instance") == 0) { ref_request_exts->add("XR_KHR_android_create_instance"); continue; }
#endif

		// Skip this extension if it's a specifically excluded one
		bool skip = false;
		for (int32_t e = 0; e < exclude_exts.count; e++) {
			if (strcmp(exts[i].extensionName, exclude_exts[e]) == 0) {
				skip = true;
				break;
			}
		}
		if (skip) continue;

		// Check if the current extension is a user requested extra, and if
		// so, add it
		bool found = false;
		for (int32_t e = 0; e < extra_exts.count; e++) {
			if (strcmp(exts[i].extensionName, extra_exts[e]) == 0) {
				ref_request_exts->add(extra_exts[e]);
				found = true;
				break;
			}
		}
		if (found) continue;

		// We're only interested required extensions if we're using minimum
		// extension mode
		if (minimum_exts) continue;

		// Check if this extension matches any extensions that StereoKit wants
#define ADD_NAME(name, available) else if (available && strcmp("XR_"#name, exts[i].extensionName) == 0) {ref_request_exts->add("XR_"#name);}
		if (false) {}
		FOR_EACH_EXT_ALL    (ADD_NAME)
		FOR_EACH_EXT_UWP    (ADD_NAME)
		FOR_EACH_EXT_ANDROID(ADD_NAME)
		FOR_EACH_EXT_LINUX  (ADD_NAME)
		FOR_EACH_EXT_DEBUG  (ADD_NAME)
		else {
			// We got to the end, and no-one loves this extension.
			ref_all_available_exts->add(string_copy(exts[i].extensionName));
		}
#undef ADD_NAME
	}

	// Mark each extension that made it to this point as available in the
	// xr_ext struct
	for (int32_t i = 0; i < ref_request_exts->count; i++) {
#define CHECK_EXT(name, available) else if (strcmp("XR_"#name, ref_request_exts->get(i)) == 0) {xr_ext.name = xr_ext_active;}
		if (false) {}
		FOR_EACH_EXT_ALL    (CHECK_EXT)
		FOR_EACH_EXT_UWP    (CHECK_EXT)
		FOR_EACH_EXT_ANDROID(CHECK_EXT)
		FOR_EACH_EXT_LINUX  (CHECK_EXT)
		FOR_EACH_EXT_DEBUG  (CHECK_EXT)
#undef CHECK_EXT
	}
	
	sk_free(exts);
	return true;
}

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