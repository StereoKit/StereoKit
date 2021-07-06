#pragma once

#include "../../stereokit.h"
#include "../../libraries/sk_gpu.h"
#include "../platform/platform_utils.h"

#if defined(SK_OS_ANDROID)
	#include <android/native_activity.h>
	#include <time.h>
	#define XR_USE_PLATFORM_ANDROID
	#define XR_USE_TIMESPEC
	#define XR_TIME_EXTENSION XR_KHR_CONVERT_TIMESPEC_TIME_EXTENSION_NAME
	#define XR_USE_GRAPHICS_API_OPENGL_ES
#elif defined(SK_OS_LINUX)
	#include <time.h>

	#if defined(SKG_LINUX_EGL)
		#define XR_USE_PLATFORM_EGL
		#define XR_USE_GRAPHICS_API_OPENGL_ES
	#else
		#define XR_USE_PLATFORM_XLIB
		#define XR_USE_GRAPHICS_API_OPENGL
	#endif

	#define XR_USE_TIMESPEC
	#define XR_TIME_EXTENSION XR_KHR_CONVERT_TIMESPEC_TIME_EXTENSION_NAME
#elif defined(SK_OS_WEB)
	#define XR_USE_TIMESPEC
	#define XR_TIME_EXTENSION XR_KHR_CONVERT_TIMESPEC_TIME_EXTENSION_NAME
	#define XR_USE_GRAPHICS_API_OPENGL
#elif defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
	#define XR_USE_PLATFORM_WIN32
	#define XR_TIME_EXTENSION XR_KHR_WIN32_CONVERT_PERFORMANCE_COUNTER_TIME_EXTENSION_NAME
	#if defined(SKG_OPENGL)
		#define XR_USE_GRAPHICS_API_OPENGL
	#elif defined(SKG_DIRECT3D11)
		#define XR_USE_GRAPHICS_API_D3D11
	#endif
#endif

#if defined(XR_USE_GRAPHICS_API_D3D11)
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

#define XrGraphicsBinding XrGraphicsBindingOpenGLESAndroidKHR
#define XR_TYPE_GRAPHICS_BINDING XR_TYPE_GRAPHICS_BINDING_OPENGL_ES_ANDROID_KHR

#elif defined(SK_OS_LINUX)

#define XrGraphicsBinding XrGraphicsBindingEGLMNDX
#define XR_TYPE_GRAPHICS_BINDING XR_TYPE_GRAPHICS_BINDING_EGL_MNDX

#endif

#endif

#include <openxr/openxr.h>
#include "openxr_extensions.h"

#include <stdint.h>

#define xr_check(xResult, message) {XrResult xr_call_result = xResult; if (XR_FAILED(xr_call_result)) {log_infof(message, openxr_string(xr_call_result)); return false;}}

namespace sk {

bool openxr_init          ();
void openxr_shutdown      ();
void openxr_step_begin    ();
void openxr_step_end      ();
void openxr_poll_events   ();
bool openxr_render_frame  ();
void openxr_poll_actions  ();

void       *openxr_get_luid ();
int64_t     openxr_get_time ();
bool32_t    openxr_get_space(XrSpace space, pose_t *out_pose, XrTime time = 0);
const char* openxr_string   (XrResult result);

extern XrSpace    xrc_space_grip[2];
extern XrSpace    xr_app_space;
extern XrInstance xr_instance;
extern XrSession  xr_session;
extern XrSessionState xr_session_state;
extern XrSystemId xr_system_id;
extern bool       xr_has_articulated_hands;
extern bool       xr_has_hand_meshes;
extern bool       xr_has_depth_lsr;
extern bool       xr_has_bounds;
extern XrTime     xr_time;
extern XrSpace    xrc_space_grip[2];
extern vec2       xr_bounds_size;
extern pose_t     xr_bounds_pose;

} // namespace sk
