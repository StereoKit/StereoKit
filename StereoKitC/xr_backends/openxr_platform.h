/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

// Include this file if you need access to platform specific OpenXR APIs! You
// should get all the OpenXR includes that you need from this one file.

#pragma once

#include "../platforms/platform.h"
#if defined(SK_XR_OPENXR)

#include "openxr.h"

// All platforms now use Vulkan
#define XR_USE_GRAPHICS_API_VULKAN

#if defined(SK_OS_ANDROID)
	#define XR_USE_PLATFORM_ANDROID
	#define XR_USE_TIMESPEC
	#include <jni.h>  // Required for OpenXR Android platform types

#elif defined(SK_OS_LINUX)
	#define XR_USE_TIMESPEC

#elif defined(SK_OS_WEB)
	#error OpenXR not supported on web

#elif defined(SK_OS_WINDOWS)
	#define XR_USE_PLATFORM_WIN32

#endif

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
	#include <sk_renderer.h>
	#define XR_GFX_EXTENSION XR_KHR_VULKAN_ENABLE_EXTENSION_NAME
	#define XrSwapchainImage XrSwapchainImageVulkanKHR
	#define XR_TYPE_SWAPCHAIN_IMAGE XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR
	#define XrGraphicsRequirements XrGraphicsRequirementsVulkanKHR
	#define XR_TYPE_GRAPHICS_REQUIREMENTS XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR
	#define xrGetGraphicsRequirementsKHR xrGetVulkanGraphicsRequirementsKHR
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

#endif