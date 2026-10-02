using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>This class exposes some of StereoKit's backend functionality.
	/// This allows for tighter integration with certain platforms, but also
	/// means your code becomes less portable. Everything in this class should
	/// be guarded by availability checks.</summary>
	public static class Backend
	{
		/// <summary>What technology is being used to drive StereoKit's XR
		/// functionality? OpenXR is the most likely candidate here, but if
		/// you're running the flatscreen Simulator, or running in the web with
		/// WebXR, then this will reflect that.</summary>
		public static BackendXRType XRType => NativeAPI.backend_xr_get_type();

		/// <summary>What kind of platform is StereoKit running on? This can be
		/// important to tell you what APIs or functionality is available to
		/// the app.</summary>
		public static BackendPlatform Platform => NativeAPI.backend_platform_get();

		/// <summary>This describes the graphics API that StereoKit is using for
		/// rendering. StereoKit is Vulkan-only, so this will report
		/// <see cref="BackendGraphics.Vulkan"/> on all supported platforms.</summary>
		public static BackendGraphics Graphics => NativeAPI.backend_graphics_get();

		/// <summary>This class is NOT of general interest, unless you are
		/// trying to add support for some unusual OpenXR extension! StereoKit
		/// should do all the OpenXR work that most people will need. If you
		/// find yourself here anyhow for something you feel StereoKit should
		/// support already, please add a feature request on GitHub!
		/// 
		/// This class contains handles and methods for working directly with
		/// OpenXR. This may allow you to activate or work with OpenXR
		/// extensions that StereoKit hasn't implemented or exposed yet. Check
		/// that Backend.XRType is OpenXR before using any of this.
		/// 
		/// These properties may best be used with some external OpenXR
		/// binding library, but you may get some limited mileage with the API
		/// as provided here.</summary>
		public static class OpenXR
		{
			/// <summary>Type: XrInstance. StereoKit's instance handle, valid
			/// after SK.Initialize.</summary>
			public static ulong Instance => NativeAPI.backend_openxr_get_instance();

			/// <summary>Type: XrSession. StereoKit's current session handle, 
			/// this will be valid after SK.Initialize, but the session may not
			/// be started quite so early.</summary>
			public static ulong Session => NativeAPI.backend_openxr_get_session();

			/// <summary>Type: XrSystemId. This is the id of the device
			/// StereoKit is currently using! This is the result of calling
			/// `xrGetSystem` with `XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY`.
			/// </summary>
			public static ulong SystemId => NativeAPI.backend_openxr_get_system_id();

			/// <summary>Type: XrSpace. StereoKit's primary coordinate space,
			/// valid after SK.Initialize, this will most likely be created
			/// from `XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT` or
			/// `XR_REFERENCE_SPACE_TYPE_LOCAL`.</summary>
			public static ulong Space => NativeAPI.backend_openxr_get_space();

			/// <summary>Type: XrSpace. StereoKit's head/view reference space,
			/// valid after SK.Initialize, this is created from
			/// `XR_REFERENCE_SPACE_TYPE_VIEW`.</summary>
			public static ulong HeadSpace => NativeAPI.backend_openxr_get_head_space();

			/// <summary>Type: XrTime. This is the OpenXR time for the current
			/// frame, and is available after SK.Initialize.</summary>
			public static long Time => NativeAPI.backend_openxr_get_time();

			/// <summary>Type: XrTime. This is the OpenXR time of the eye
			/// tracker sample associated with the current value of
			/// <see cref="Input.Eyes"/>.</summary>
			public static long EyesSampleTime => NativeAPI.backend_openxr_get_eyes_sample_time();

			/// <summary>Tells StereoKit to request only the extensions that
			/// are absolutely critical to StereoKit. You can still request
			/// extensions via `OpenXR.RequestExt`, and this can be used to
			/// opt-in to extensions that StereoKit would normally request
			/// automatically.</summary>
			public static bool UseMinimumExts { set { NativeAPI.backend_openxr_use_minimum_exts(value); } }

			/// <summary>This tells if an OpenXR extension has been requested
			/// and successfully loaded by the runtime. This MUST only be
			/// called after SK.Initialize.</summary>
			/// <param name="extensionName">The extension name as listed in the
			/// OpenXR spec. For example: "XR_EXT_hand_tracking".</param>
			/// <returns>If the extension is available to use.</returns>
			public static bool ExtEnabled(string extensionName) => NativeAPI.backend_openxr_ext_enabled(extensionName);

			/// <summary>This is basically `xrGetInstanceProcAddr` from OpenXR,
			/// you can use this to get and call functions from an extension
			/// you've loaded. You can use `Marshal.GetDelegateForFunctionPointer`
			/// to turn the result into a delegate that you can call.</summary>
			/// <param name="functionName"></param>
			/// <returns>A function pointer, or null on failure. You can use 
			/// `Marshal.GetDelegateForFunctionPointer` to turn this into a
			/// delegate that you can call.</returns>
			public static IntPtr GetFunctionPtr(string functionName) => NativeAPI.backend_openxr_get_function(functionName);

			/// <summary>This is basically `xrGetInstanceProcAddr` from OpenXR,
			/// you can use this to get and call functions from an extension
			/// you've loaded. This uses `Marshal.GetDelegateForFunctionPointer`
			/// to turn the result into a delegate that you can call.</summary>
			/// <param name="functionName"></param>
			/// <returns>A delegate, or null on failure.</returns>
			public static TDelegate GetFunction<TDelegate>(string functionName) {
				IntPtr fn = NativeAPI.backend_openxr_get_function(functionName);
				if (fn == IntPtr.Zero) return default;
				return Marshal.GetDelegateForFunctionPointer<TDelegate>(fn);
			}

			/// <summary>This sets a scaling value for joints provided by the
			/// articulated hand extension. Some systems just don't seem to get
			/// their joint sizes right!</summary>
			/// <param name="scaleFactor">1 being the default value, 2 being
			/// twice as large as normal, and 0.5 being half as big as normal.
			/// </param>
			public static void SetHandJointScale(float scaleFactor)
				=> NativeAPI.backend_openxr_set_hand_joint_scale(scaleFactor);

			/// <summary>Requests that OpenXR load a particular extension. This
			/// MUST be called before SK.Initialize. Note that it's entirely
			/// possible that your extension will not load on certain runtimes,
			/// so be sure to check ExtEnabled to see if it's available to use.
			/// </summary>
			/// <param name="extensionName">The extension name as listed in the
			/// OpenXR spec. For example: "XR_EXT_hand_tracking".</param>
			public static void RequestExt(string extensionName) => NativeAPI.backend_openxr_ext_request(extensionName);

			/// <summary>This ensures that StereoKit does not load a particular
			/// extension! StereoKit will behave as if the extension is not
			/// available on the device. It will also be excluded even if you
			/// explicitly requested it with `RequestExt` earlier, or
			/// afterwards. This MUST be called before SK.Initialize.</summary>
			/// <param name="extensionName">The extension name as listed in the
			/// OpenXR spec. For example: "XR_EXT_hand_tracking".</param>
			public static void ExcludeExt(string extensionName) => NativeAPI.backend_openxr_ext_exclude(extensionName);

			/// <summary>This allows you to add XrCompositionLayers to the list
			/// that StereoKit submits to xrEndFrame. You must call this every
			/// frame you wish the layer to be included.</summary>
			/// <typeparam name="T">This must be a serializable struct that
			/// follows the XrCompositionLayerBaseHeader data pattern.
			/// </typeparam>
			/// <param name="XrCompositionLayerX">A serializable
			/// XrCompositionLayer struct that follows the
			/// XrCompositionLayerBaseHeader data pattern. </param>
			/// <param name="sortOrder">An sort order value for sorting with
			/// other composition layers in the list. The primary projection
			/// layer that StereoKit renders to is at 0, -1 would be before it,
			/// and +1 would be after.</param>
			public static void AddCompositionLayer<T>(T XrCompositionLayerX, int sortOrder) where T : struct
			{
				int    size = Marshal.SizeOf<T>();
				IntPtr ptr  = Marshal.AllocHGlobal(size);
				Marshal.StructureToPtr(XrCompositionLayerX, ptr, false);
				NativeAPI.backend_openxr_composition_layer( ptr, size, sortOrder);
				Marshal.FreeHGlobal(ptr);
			}

			/// <summary>This adds an item to the chain of objects submitted to
			/// StereoKit's xrEndFrame call!</summary>
			/// <typeparam name="T">This must be a serializable struct that
			/// follows the OpenXR data struct pattern.</typeparam>
			/// <param name="XrBaseHeader">An OpenXR object that will be
			/// chained into the xrEndFrame call.</param>
			public static void AddEndFrameChain<T>(T XrBaseHeader) where T : struct
			{
				int    size = Marshal.SizeOf<T>();
				IntPtr ptr  = Marshal.AllocHGlobal(size);
				Marshal.StructureToPtr(XrBaseHeader, ptr, false);
				NativeAPI.backend_openxr_end_frame_chain(ptr, size);
				Marshal.FreeHGlobal(ptr);
			}

			private static event Action _onPreCreateSession;
			private static bool         _onPreCreateSessionRegistered = false;
			private static void _OnPreCreateSession(IntPtr context)
			{
				_onPreCreateSession();
				_onPreCreateSession           = null;
				_onPreCreateSessionRegistered = false;
			}

			/// <summary>This allows you to add callbacks that are invoked
			/// immediately before the OpenXR session is created, but after
			/// OpenXR has been initialized! This is only helpful when filled
			/// out _before_ calling `SK.Initialize`.</summary>
			public static event Action OnPreCreateSession {
				add {
					if (_onPreCreateSessionRegistered == false)
					{
						_onPreCreateSessionRegistered = true;
						NativeAPI.backend_openxr_add_callback_pre_session_create(_OnPreCreateSession, IntPtr.Zero);
					}
					_onPreCreateSession += value;
				}
				remove => _onPreCreateSession -= value;
			}

			internal static void CleanupInitialize()
			{
				// If OpenXR was not the backend, the callback events could
				// still contain callbacks with capture data! So we want to
				// free all those up.
				_onPreCreateSession           = null;
				_onPreCreateSessionRegistered = false;
			}

			private struct XRPollEventCallbackData
			{
				public Action<IntPtr>      action;
				public XRPollEventCallback callback;
			}

			private static List<XRPollEventCallbackData> _xrPollEventCallbacks;

			/// <summary>This event gets published each time xrPollEvent results in XR_SUCCESS.</summary>
			public static event Action<IntPtr> OnPollEvent
			{
				add
				{
					if (_xrPollEventCallbacks == null) _xrPollEventCallbacks = new List<XRPollEventCallbackData>();

					XRPollEventCallback callback = (_, XrEventDataBuffer) => { value(XrEventDataBuffer); };
					_xrPollEventCallbacks.Add(new XRPollEventCallbackData { action = value, callback = callback });

					NativeAPI.backend_openxr_add_callback_poll_event(callback, IntPtr.Zero);
				}
				remove
				{
					if (_xrPollEventCallbacks == null) throw new NullReferenceException();

					int i = _xrPollEventCallbacks.FindIndex(d => d.action == value);
					if (i < 0) throw new KeyNotFoundException();

					NativeAPI.backend_openxr_remove_callback_poll_event(_xrPollEventCallbacks[i].callback);
					_xrPollEventCallbacks.RemoveAt(i);
				}
			}
		}

		/// <summary>This class contains variables that may be useful for
		/// interop with the Android operating system, or other Android
		/// libraries.</summary>
		public static class Android
		{
			/// <summary>This is the `JavaVM*` object that StereoKit uses on
			/// Android. This is only valid after SK.Initialize, on Android
			/// systems.</summary>
			public static IntPtr JavaVM => NativeAPI.backend_android_get_java_vm();
			/// <summary>This is the `jobject` activity that StereoKit uses on
			/// Android. This is only valid after SK.Initialize, on Android
			/// systems.</summary>
			public static IntPtr Activity => NativeAPI.backend_android_get_activity();
			/// <summary>This is the `JNIEnv*` object that StereoKit uses on
			/// Android. This is only valid after SK.Initialize, on Android
			/// systems.</summary>
			public static IntPtr JNIEnvironment => NativeAPI.backend_android_get_jni_env();
		}

		/// <summary>When using Direct3D11 for rendering, this contains a
		/// number of variables that may be useful for doing advanced rendering
		/// tasks. This is the default rendering backend on Windows.</summary>
		[Obsolete("StereoKit is now Vulkan-only; the D3D11 backend is no longer supported. Use Backend.Vulkan instead.")]
		public static class D3D11
		{
			/// <summary>This is the main `ID3D11Device*` StereoKit uses for
			/// rendering. (No longer supported, always returns IntPtr.Zero)</summary>
			public static IntPtr D3DDevice  => default;
			/// <summary>This is the main `ID3D11DeviceContext*` StereoKit uses
			/// for rendering. (No longer supported, always returns IntPtr.Zero)</summary>
			public static IntPtr D3DContext => default;
		}

		/// <summary>When using OpenGL with the WGL loader for rendering, this
		/// contains a number of variables that may be useful for doing
		/// advanced rendering tasks. This is Windows only, and requires
		/// gloabally defining SKG_FORCE_OPENGL when building the core
		/// StereoKitC library.</summary>
		[Obsolete("StereoKit is now Vulkan-only; the OpenGL/WGL backend is no longer supported. Use Backend.Vulkan instead.")]
		public static class OpenGL_WGL
		{
			/// <summary>This is the Handle to Device Context `HDC` StereoKit
			/// uses with `wglMakeCurrent`. (No longer supported, always returns
			/// IntPtr.Zero)</summary>
			public static IntPtr HDC => default;
			/// <summary>This is the Handle to an OpenGL Rendering Context
			/// `HGLRC` StereoKit uses with `wglMakeCurrent`. (No longer
			/// supported, always returns IntPtr.Zero)</summary>
			public static IntPtr HGLRC => default;
		}

		/// <summary>When using OpenGL with the GLX loader for rendering, this
		/// contains a number of variables that may be useful for doing
		/// advanced rendering tasks. This is the default rendering backend for
		/// Linux.</summary>
		[Obsolete("StereoKit is now Vulkan-only; the OpenGL/GLX backend is no longer supported. Use Backend.Vulkan instead.")]
		public static class OpenGL_GLX
		{
			/// <summary>This is the `Display*` from X used to create the GLX
			/// context. (No longer supported, always returns IntPtr.Zero)</summary>
			public static IntPtr Display  => default;
			/// <summary>This is the `GLXContext` that StereoKit uses with
			/// `glXMakeCurrent` (No longer supported, always returns IntPtr.Zero)</summary>
			public static IntPtr Context  => default;
			/// <summary>This is the `GLXDrawable` that StereoKit uses with
			/// `glXMakeCurrent`. (No longer supported, always returns IntPtr.Zero)</summary>
			public static IntPtr Drawable => default;
		}

		/// <summary>When using OpenGL ES with the EGL loader for rendering,
		/// this contains a number of variables that may be useful for doing
		/// advanced rendering tasks. This is the default rendering backend for
		/// Android, and Linux builds can be configured to use this with the
		/// SK_LINUX_EGL cmake option when building the core StereoKitC
		/// library.</summary>
		[Obsolete("StereoKit is now Vulkan-only; the OpenGL ES/EGL backend is no longer supported. Use Backend.Vulkan instead.")]
		public static class OpenGLES_EGL
		{
			/// <summary>This is the `EGLDisplay` StereoKit receives from
			/// `eglGetDisplay` (No longer supported, always returns IntPtr.Zero)</summary>
			public static IntPtr Display => default;
			/// <summary>This is the `EGLContext` StereoKit receives from
			/// `eglCreateContext`. (No longer supported, always returns IntPtr.Zero)</summary>
			public static IntPtr Context => default;
		}

		/// <summary>When using Vulkan for rendering, this contains a number
		/// of variables that may be useful for doing advanced rendering
		/// tasks. Vulkan is StereoKit's only rendering backend, so these are
		/// valid on all supported platforms after SK.Initialize.</summary>
		public static class Vulkan
		{
			/// <summary>The `VkInstance` StereoKit created (or was given, when
			/// running under OpenXR) for rendering. Valid after SK.Initialize.
			/// </summary>
			public static IntPtr Instance => NativeAPI.backend_vulkan_get_instance();
			/// <summary>The `VkPhysicalDevice` StereoKit is rendering with.
			/// Valid after SK.Initialize.</summary>
			public static IntPtr PhysicalDevice => NativeAPI.backend_vulkan_get_physical_device();
			/// <summary>The `VkDevice` StereoKit created (or was given, when
			/// running under OpenXR) for rendering. Valid after SK.Initialize.
			/// </summary>
			public static IntPtr Device => NativeAPI.backend_vulkan_get_device();

			/// <summary>Gets the `VkQueue` StereoKit uses for the given queue
			/// family. Currently only <see cref="BackendVulkanQueue.Graphics"/>
			/// has a handle available; the others return IntPtr.Zero until
			/// StereoKit makes real use of them. If you submit work to this
			/// queue, you MUST guard it with <see cref="QueueLock"/> /
			/// <see cref="QueueUnlock"/>, since StereoKit shares it across
			/// threads.</summary>
			/// <param name="queue">Which queue family to retrieve the queue
			/// for.</param>
			/// <returns>A `VkQueue` handle, or IntPtr.Zero if no queue handle is
			/// available for that family.</returns>
			public static IntPtr Queue(BackendVulkanQueue queue)
				=> NativeAPI.backend_vulkan_get_queue(queue);

			/// <summary>Gets the queue family index StereoKit uses for the given
			/// queue family. This is the value you'd use when creating command
			/// pools or performing queue family ownership transfers.</summary>
			/// <param name="queue">Which queue family to look up.</param>
			/// <returns>The Vulkan queue family index, or uint.MaxValue if that
			/// family is not available on this device (for example, video
			/// decode).</returns>
			public static uint QueueFamilyIndex(BackendVulkanQueue queue)
				=> NativeAPI.backend_vulkan_get_queue_family_index(queue);

			/// <summary>Locks the mutex StereoKit uses to guard the given queue
			/// family, so you can safely submit work to a queue StereoKit also
			/// uses. Always pair this with <see cref="QueueUnlock"/>. Note that
			/// queue families that resolve to the same index share a single
			/// lock, so don't nest locks across two families that may
			/// alias.</summary>
			/// <param name="queue">Which queue family's lock to acquire.</param>
			public static void QueueLock(BackendVulkanQueue queue)
				=> NativeAPI.backend_vulkan_queue_lock(queue);

			/// <summary>Releases the queue family lock acquired via
			/// <see cref="QueueLock"/>.</summary>
			/// <param name="queue">Which queue family's lock to release.</param>
			public static void QueueUnlock(BackendVulkanQueue queue)
				=> NativeAPI.backend_vulkan_queue_unlock(queue);

			/// <summary>Returns a sync file descriptor for the most recently
			/// submitted frame's GPU work! Waiting on it (e.g. via
			/// EGL_ANDROID_native_fence_sync) guarantees all rendering
			/// submitted up to the last frame end has completed. Call from
			/// StereoKit's main thread. The caller owns the descriptor and
			/// must close it. Only functional on platforms and devices
			/// supporting external fence export.</summary>
			/// <returns>A sync file descriptor, or -1 when unsupported or no
			/// frame has been submitted yet.</returns>
			public static int GetFrameFenceFd()
				=> NativeAPI.backend_vulkan_get_frame_fence_fd();

			/// <summary>Registers a request for Vulkan instance/device
			/// extensions and device features. This MUST be called before
			/// SK.Initialize. A request enables atomically: only when all of
			/// its extensions are present, and every requested feature bit is
			/// supported. If <see cref="BackendVulkanRequest.required"/> is true
			/// and the request can't be satisfied, SK.Initialize will fail!
			/// After initialization, check the result with
			/// <see cref="RequestEnabled"/> (by name) or <see cref="ExtEnabled"/>
			/// (by extension name).</summary>
			/// <param name="request">The extensions and features to request. Its
			/// arrays and feature struct pointers only need to remain valid for
			/// the duration of this call - StereoKit copies everything it needs.
			/// </param>
			public static void Request(BackendVulkanRequest request)
			{
				IntPtr nameStr = Marshal.StringToCoTaskMemUTF8(request.name);
				IntPtr instArr = StrArrToNative(request.instanceExtensions, out int instCount);
				IntPtr devArr  = StrArrToNative(request.deviceExtensions,   out int devCount);

				int    featCount = request.features == null ? 0 : request.features.Length;
				IntPtr featArr   = IntPtr.Zero;
				if (featCount > 0)
				{
					int size = Marshal.SizeOf<BackendVulkanFeature>();
					featArr  = Marshal.AllocHGlobal(size * featCount);
					for (int i = 0; i < featCount; i++)
						Marshal.StructureToPtr(request.features[i], featArr + i * size, false);
				}

				BackendVulkanRequestT native = new BackendVulkanRequestT {
					name                   = nameStr,
					required               = request.required,
					instanceExtensions     = instArr,
					instanceExtensionCount = instCount,
					deviceExtensions       = devArr,
					deviceExtensionCount   = devCount,
					features               = featArr,
					featureCount           = featCount,
				};
				NativeAPI.backend_vulkan_request(native);

				// StereoKit copies the request, so temporaries can be freed now.
				FreeStrArr(instArr, instCount);
				FreeStrArr(devArr,  devCount);
				if (featArr != IntPtr.Zero) Marshal.FreeHGlobal   (featArr);
				Marshal.FreeCoTaskMem(nameStr);
			}

			/// <summary>Checks if a named request registered via
			/// <see cref="Request"/> was successfully enabled. This MUST only be
			/// called after SK.Initialize.</summary>
			/// <param name="name">The name given to the BackendVulkanRequest.
			/// </param>
			/// <returns>If the request's extensions and features were all
			/// enabled.</returns>
			public static bool RequestEnabled(string name)
				=> NativeAPI.backend_vulkan_request_enabled(name);

			/// <summary>Checks if a Vulkan extension was enabled at init,
			/// regardless of which request asked for it. This MUST only be
			/// called after SK.Initialize.</summary>
			/// <param name="extensionName">The extension name, for example
			/// "VK_KHR_swapchain".</param>
			/// <returns>If the extension is available to use.</returns>
			public static bool ExtEnabled(string extensionName)
				=> NativeAPI.backend_vulkan_ext_enabled(extensionName);

			/// <summary>Resolves a Vulkan function pointer, using
			/// `vkGetDeviceProcAddr` with a `vkGetInstanceProcAddr` fallback.
			/// Use this to call into extensions you've enabled via
			/// <see cref="Request"/>. You can use
			/// `Marshal.GetDelegateForFunctionPointer` to turn the result into a
			/// callable delegate.</summary>
			/// <param name="functionName">The Vulkan function name, for example
			/// "vkCmdBeginRenderingKHR".</param>
			/// <returns>A function pointer, or IntPtr.Zero on failure.</returns>
			public static IntPtr GetFunctionPtr(string functionName)
				=> NativeAPI.backend_vulkan_get_function(functionName);

			/// <summary>Resolves a Vulkan function pointer and wraps it as a
			/// delegate, using `vkGetDeviceProcAddr` with a
			/// `vkGetInstanceProcAddr` fallback. Use this to call into extensions
			/// you've enabled via <see cref="Request"/>.</summary>
			/// <param name="functionName">The Vulkan function name, for example
			/// "vkCmdBeginRenderingKHR".</param>
			/// <returns>A delegate, or null on failure.</returns>
			public static TDelegate GetFunction<TDelegate>(string functionName)
			{
				IntPtr fn = NativeAPI.backend_vulkan_get_function(functionName);
				if (fn == IntPtr.Zero) return default;
				return Marshal.GetDelegateForFunctionPointer<TDelegate>(fn);
			}

			static IntPtr StrArrToNative(string[] strings, out int count)
			{
				count = strings == null ? 0 : strings.Length;
				if (count == 0) return IntPtr.Zero;
				IntPtr block = Marshal.AllocHGlobal(IntPtr.Size * count);
				for (int i = 0; i < count; i++)
					Marshal.WriteIntPtr(block, i * IntPtr.Size, Marshal.StringToCoTaskMemUTF8(strings[i]));
				return block;
			}

			static void FreeStrArr(IntPtr block, int count)
			{
				if (block == IntPtr.Zero) return;
				for (int i = 0; i < count; i++)
					Marshal.FreeCoTaskMem(Marshal.ReadIntPtr(block, i * IntPtr.Size));
				Marshal.FreeHGlobal(block);
			}
		}

	}

	/// <summary>A single Vulkan feature struct to request as part of a
	/// <see cref="BackendVulkanRequest"/>. See
	/// <see cref="Backend.Vulkan.Request"/> for details.</summary>
	public struct BackendVulkanFeature
	{
		/// <summary>A pointer to a pinned VkPhysicalDevice*Features struct with
		/// its sType set, and the feature bits you want enabled set to VK_TRUE.
		/// This must NOT be a VkPhysicalDeviceFeatures2. The pointer only needs
		/// to remain valid for the duration of the Backend.Vulkan.Request call.
		/// </summary>
		public IntPtr vkStruct;
		/// <summary>The size of the struct vkStruct points at, in bytes.
		/// </summary>
		public int    size;

		/// <summary>Creates a feature request from a pointer to a pinned
		/// VkPhysicalDevice*Features struct and its size in bytes.</summary>
		/// <param name="vkStruct">A pointer to a pinned VkPhysicalDevice*Features
		/// struct, with its sType and desired VK_TRUE bits set.</param>
		/// <param name="size">The size of the struct vkStruct points at, in
		/// bytes.</param>
		public BackendVulkanFeature(IntPtr vkStruct, int size)
		{
			this.vkStruct = vkStruct;
			this.size     = size;
		}
	}

	/// <summary>A request for Vulkan instance/device extensions and device
	/// features, registered via <see cref="Backend.Vulkan.Request"/> before
	/// SK.Initialize.</summary>
	public struct BackendVulkanRequest
	{
		/// <summary>An optional name used as a handle for
		/// <see cref="Backend.Vulkan.RequestEnabled"/>. null makes the request
		/// anonymous - it still contributes its extensions and features, but
		/// can't be queried by name.</summary>
		public string   name;
		/// <summary>If true, SK.Initialize will fail should this request go
		/// unsatisfied. If false, an unmet request is simply left disabled.
		/// </summary>
		public bool     required;
		/// <summary>Vulkan instance extension names this request needs.</summary>
		public string[] instanceExtensions;
		/// <summary>Vulkan device extension names this request needs.</summary>
		public string[] deviceExtensions;
		/// <summary>Vulkan device features this request needs. Their bits are
		/// queried for support before being enabled.</summary>
		public BackendVulkanFeature[] features;
	}

	// Native ABI mirror of backend_vulkan_request_t, filled and freed by
	// Backend.Vulkan.Request.
	[StructLayout(LayoutKind.Sequential)]
	internal struct BackendVulkanRequestT
	{
		public IntPtr name;
		[MarshalAs(UnmanagedType.Bool)] public bool required;
		public IntPtr instanceExtensions;
		public int    instanceExtensionCount;
		public IntPtr deviceExtensions;
		public int    deviceExtensionCount;
		public IntPtr features;
		public int    featureCount;
	}
}
