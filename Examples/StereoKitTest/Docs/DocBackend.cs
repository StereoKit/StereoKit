// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

using System;
using System.Runtime.InteropServices;
using StereoKit;
using StereoKit.Framework;

class DocBackend : ITest
{
	public void Initialize() { }
	public void Shutdown  () { }
	public void Step      () { }
}

/// :CodeSample: OpenXR.RequestExt OpenXR.ExtEnabled Backend.XRType OpenXR.GetFunction OpenXR.Instance OpenXR.Time IStepper
/// ### Implementing OpenXR Extensions
/// 
/// Using the `Backend.OpenXR` class, it's possible to implement OpenXR
/// extensions without directly modifying StereoKit! Here's a simple
/// example of how to do this, implemented via an `IStepper`.
class Win32PerformanceCounterExt : IStepper
{
	// Start by defining C# equivalents of OpenXR's function signatures and
	// types. This can be a bit involved, see PassthroughFBExt.cs in the SK
	// repository for a more extensive sample.
	delegate uint XR_xrConvertTimeToWin32PerformanceCounterKHR(ulong instance, long time, out long performanceCounter);
	static        XR_xrConvertTimeToWin32PerformanceCounterKHR xrConvertTimeToWin32PerformanceCounterKHR;
	const string timeExt = "XR_KHR_win32_convert_performance_counter_time";

	public bool Enabled { get; private set; }

	public Win32PerformanceCounterExt()
	{
		// OpenXR extensions must be requested before initializing StereoKit,
		// so this IStepper needs to be added _before_ `SK.Initialize`.
		if (SK.IsInitialized)
			Log.Err("OpenXR extensions must be constructed before StereoKit is initialized!");

		// At this point, we don't even know if the app will have access to
		// OpenXR, so this should _not_ be be guarded by a check for OpenXR.
		Backend.OpenXR.RequestExt(timeExt);
	}

	public bool Initialize()
	{
		// Check if we're running OpenXR, the extension is present, and all of
		// our extension functions bound properly.
		Enabled =
			Backend.XRType == BackendXRType.OpenXR &&
			Backend.OpenXR.ExtEnabled(timeExt)     &&
			LoadBindings();

		// Test it out!
		if (Enabled)
		{
			xrConvertTimeToWin32PerformanceCounterKHR(Backend.OpenXR.Instance, Backend.OpenXR.Time, out long counter);
			Log.Info($"XrTime: {counter}");
		}

		return Enabled;
	}

	// In this method, we load any functions from the extension that we care
	// about, and then report if they were loaded successfully.
	private bool LoadBindings()
	{
		xrConvertTimeToWin32PerformanceCounterKHR =
			Backend.OpenXR.GetFunction<XR_xrConvertTimeToWin32PerformanceCounterKHR>("xrConvertTimeToWin32PerformanceCounterKHR");

		return xrConvertTimeToWin32PerformanceCounterKHR != null;
	}

	// A more complicated extension might use these, but this EXT does not
	// require any actions on-Step.
	public void Shutdown() { }
	public void Step() { }
}
/// :End:

/// :CodeSample: Backend.Vulkan Backend.Graphics BackendVulkanRequest IStepper
/// ### Requesting Vulkan Extensions
///
/// StereoKit renders with Vulkan, and `Backend.Vulkan` lets you opt into extra
/// Vulkan instance/device extensions and device features that StereoKit
/// wouldn't normally request. Register a `BackendVulkanRequest` _before_
/// `SK.Initialize`, then after initialization check whether it enabled and
/// resolve any functions it provides.
///
/// A good practical use is `VK_EXT_debug_utils`, which lets you attach
/// human-readable names to Vulkan objects so they show up nicely in tools like
/// RenderDoc or the validation layers. Here we request the (instance)
/// extension, then use it to name the `VkDevice` StereoKit is rendering with.
/// This is implemented via an `IStepper`, so it must be added before
/// `SK.Initialize`.
class VulkanDebugNamesExt : IStepper
{
	const string debugUtilsExt = "VK_EXT_debug_utils";

	// A C# equivalent of vkSetDebugUtilsObjectNameEXT and the struct it takes.
	[StructLayout(LayoutKind.Sequential)]
	struct VkDebugUtilsObjectNameInfoEXT
	{
		public int    sType;
		public IntPtr pNext;
		public int    objectType;
		public ulong  objectHandle;
		[MarshalAs(UnmanagedType.LPUTF8Str)] public string pObjectName;
	}
	delegate int VkSetDebugUtilsObjectNameEXT(IntPtr device, in VkDebugUtilsObjectNameInfoEXT nameInfo);
	static        VkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;

	public bool Enabled { get; private set; }

	public VulkanDebugNamesExt()
	{
		// Vulkan requests must happen before StereoKit initializes, so this
		// IStepper needs to be added _before_ `SK.Initialize`.
		if (SK.IsInitialized)
			Log.Err("Vulkan extensions must be requested before StereoKit is initialized!");

		// debug_utils is an _instance_ extension. A named request lets us query
		// it later, and leaving `required` false means SK still starts up if
		// the extension isn't available.
		Backend.Vulkan.Request(new BackendVulkanRequest {
			name               = "debug_utils",
			required           = false,
			instanceExtensions = new string[] { debugUtilsExt },
		});
	}

	public bool Initialize()
	{
		// Confirm we're on Vulkan, the extension enabled, and our function
		// bound. RequestEnabled("debug_utils") would also work here.
		Enabled =
			Backend.Graphics == BackendGraphics.Vulkan &&
			Backend.Vulkan.ExtEnabled(debugUtilsExt)   &&
			LoadBindings();

		if (Enabled)
		{
			// Give the VkDevice a friendly name for debugging tools.
			VkDebugUtilsObjectNameInfoEXT info = new VkDebugUtilsObjectNameInfoEXT {
				sType        = 1000128000, // VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT
				objectType   = 3,          // VK_OBJECT_TYPE_DEVICE
				objectHandle = (ulong)Backend.Vulkan.Device,
				pObjectName  = "StereoKit Device",
			};
			vkSetDebugUtilsObjectNameEXT(Backend.Vulkan.Device, info);
		}
		return Enabled;
	}

	private bool LoadBindings()
	{
		vkSetDebugUtilsObjectNameEXT = Backend.Vulkan.GetFunction<VkSetDebugUtilsObjectNameEXT>("vkSetDebugUtilsObjectNameEXT");
		return vkSetDebugUtilsObjectNameEXT != null;
	}

	public void Shutdown() { }
	public void Step() { }
}
/// :End: