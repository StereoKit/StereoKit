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

		/// <summary>This describes the graphics API thatStereoKit is using for
		/// rendering. StereoKit uses D3D11 for Windows platforms, and a flavor
		/// of OpenGL for Linux, Android, and Web.</summary>
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
			public static bool UseMinimumExts { set => NativeAPI.backend_openxr_use_minimum_exts(value?1:0); }

			/// <summary>This tells if an OpenXR extension has been requested
			/// and successfully loaded by the runtime. This MUST only be
			/// called after SK.Initialize.</summary>
			/// <param name="extensionName">The extension name as listed in the
			/// OpenXR spec. For example: "XR_EXT_hand_tracking".</param>
			/// <returns>If the extension is available to use.</returns>
			public static bool ExtEnabled(string extensionName) => NativeAPI.backend_openxr_ext_enabled(extensionName)>0;

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

			/// <summary>Requests that OpenXR load a particular extension. This
			/// MUST be called before SK.Initialize. Note that it's entirely
			/// possible that your extension will not load on certain runtimes,
			/// so be sure to check ExtEnabled to see if it's available to use.
			/// </summary>
			/// <param name="extensionName">The extension name as listed in the
			/// OpenXR spec. For example: "XR_EXT_hand_tracking".</param>
			public  static void RequestExt (string extensionName) { NativeLib.Load(); _RequestExt(extensionName); }
			private static void _RequestExt(string extensionName) => NativeAPI.backend_openxr_ext_request(extensionName);

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
				int size = Marshal.SizeOf<T>();
				IntPtr ptr = Marshal.AllocHGlobal(size);
				Marshal.StructureToPtr(XrCompositionLayerX, ptr, false);
				NativeAPI.backend_openxr_composition_layer( ptr, size, sortOrder);
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
		public static class D3D11
		{
			/// <summary>This is the main `ID3D11Device*` StereoKit uses for
			/// rendering.</summary>
			public static IntPtr D3DDevice  => NativeAPI.backend_d3d11_get_d3d_device();
			/// <summary>This is the main `ID3D11DeviceContext*` StereoKit uses
			/// for rendering.</summary>
			public static IntPtr D3DContext => NativeAPI.backend_d3d11_get_d3d_context();
		}

		/// <summary>When using OpenGL with the WGL loader for rendering, this
		/// contains a number of variables that may be useful for doing
		/// advanced rendering tasks. This is Windows only, and requires
		/// gloabally defining SKG_FORCE_OPENGL when building the core
		/// StereoKitC library.</summary>
		public static class OpenGL_WGL
		{
			/// <summary>This is the Handle to Device Context `HDC` StereoKit
			/// uses with `wglMakeCurrent`.</summary>
			public static IntPtr HDC => NativeAPI.backend_opengl_wgl_get_hdc();
			/// <summary>This is the Handle to an OpenGL Rendering Context
			/// `HGLRC` StereoKit uses with `wglMakeCurrent`.</summary>
			public static IntPtr HGLRC => NativeAPI.backend_opengl_wgl_get_hglrc();
		}

		/// <summary>When using OpenGL with the GLX loader for rendering, this
		/// contains a number of variables that may be useful for doing
		/// advanced rendering tasks. This is the default rendering backend for
		/// Linux.</summary>
		public static class OpenGL_GLX
		{
			/// <summary>This is the `Display*` from X used to create the GLX
			/// context.</summary>
			public static IntPtr Display  => NativeAPI.backend_opengl_glx_get_display();
			/// <summary>This is the `GLXContext` that StereoKit uses with
			/// `glXMakeCurrent`</summary>
			public static IntPtr Context  => NativeAPI.backend_opengl_glx_get_context();
			/// <summary>This is the `GLXDrawable` that StereoKit uses with
			/// `glXMakeCurrent`.</summary>
			public static IntPtr Drawable => NativeAPI.backend_opengl_glx_get_drawable();
		}

		/// <summary>When using OpenGL ES with the EGL loader for rendering,
		/// this contains a number of variables that may be useful for doing
		/// advanced rendering tasks. This is the default rendering backend for
		/// Android, and Linux builds can be configured to use this with the
		/// SK_LINUX_EGL cmake option when building the core StereoKitC
		/// library.</summary>
		public static class OpenGLES_EGL
		{
			/// <summary>This is the `EGLDisplay` StereoKit receives from
			/// `eglGetDisplay`</summary>
			public static IntPtr Display => NativeAPI.backend_opengl_egl_get_display();
			/// <summary>This is the `EGLContext` StereoKit receives from
			/// `eglCreateContext`.</summary>
			public static IntPtr Context => NativeAPI.backend_opengl_egl_get_context();
		}
		
	}
}
