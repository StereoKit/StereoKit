using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace StereoKit
{
    static class NativeCallbacks
    {
        static bool _isWeb = false;
        static ReadOnlyDictionary<string, CallbackInfo> _webCallbacks;

        static NativeCallbacks()
        {
            _isWeb = RuntimeInformation.OSDescription == "Browser";

            if (_isWeb)
            {
                InitWebCallbacks();
            }
        }

        internal static TDelegate GetNativeCallback<TDelegate>(TDelegate action, [CallerMemberName] string nativeMethodName = "", string nativeArgName = null) where TDelegate : Delegate
        {
            if (_isWeb)
            {
                var callbackKey = nativeMethodName;
                if(nativeArgName != null)
                {
                    callbackKey += "_" + nativeArgName;
                }

                if(!_webCallbacks.TryGetValue(callbackKey, out var callback))
                {
                    throw new InvalidOperationException($"Native callback {callbackKey} not found");
                }

                callback.ManagedCallback = action;
                return (TDelegate)callback.NativeCallback;
            }

            return action;
        }

        private static void InitWebCallbacks()
        {
            var webCallbacks = new Dictionary<string, CallbackInfo>();
            var methods = typeof(NativeCallbacks)
                .GetMethods(BindingFlags.NonPublic | BindingFlags.Static)
                .Where(m => m.GetCustomAttributes(typeof(MonoPInvokeCallbackAttribute), true).Any());

            foreach (var method in methods)
            {
                var monoPInvokeCallback = method.GetCustomAttribute<MonoPInvokeCallbackAttribute>();
                webCallbacks.Add(method.Name, new CallbackInfo { NativeCallback = Delegate.CreateDelegate(monoPInvokeCallback.CallbackType, method) });
            }

            _webCallbacks = new ReadOnlyDictionary<string, CallbackInfo> (webCallbacks);
        }

        private static object ProcessCallback([CallerMemberName] string callbackKey = "", params object[] args)
        {
            if (_webCallbacks.TryGetValue(callbackKey, out var callback) && callback.ManagedCallback != null)
            {
                return callback.ManagedCallback.DynamicInvoke(args);
            }

            return default(object);
        }

        private class CallbackInfo
        {
            internal Delegate NativeCallback { get; set; }
            internal Delegate ManagedCallback { get; set; }
        }

        // Build will fail with Parameter types of pinvoke callback method 'System.Void log_subscribe(System.IntPtr, StereoKit.LogLevel, System.String)' needs to be blittable.
        // This means Blazor does not yet support a string parameter in a PInvoke callback method.
        //[MonoPInvokeCallback(typeof(LogCallbackData))] private static void log_subscribe(IntPtr context, LogLevel level, string text) => ProcessCallback(nameof(log_subscribe), context, level, string.Empty);
        [MonoPInvokeCallback(typeof(Action))]  private static void sk_step() => ProcessCallback();
        [MonoPInvokeCallback(typeof(Action))]  private static void sk_run_app_update() => ProcessCallback();
        [MonoPInvokeCallback(typeof(Action))] private static void sk_run_app_shutdown() => ProcessCallback();
        [MonoPInvokeCallback(typeof(RenderOnScreenshotCallback))] private static void render_screenshot_capture(IntPtr data, int width, int height, IntPtr context) => ProcessCallback(nameof(render_screenshot_capture), data, width, height, context);
        [MonoPInvokeCallback(typeof(RenderOnScreenshotCallback))] private static void render_screenshot_viewpoint(IntPtr data, int width, int height, IntPtr context) => ProcessCallback(nameof(render_screenshot_viewpoint), data, width, height, context);
        [MonoPInvokeCallback(typeof(AudioGenerator))]  private static float sound_generate(float time) => (float)ProcessCallback(nameof(sound_generate), time); 
        [MonoPInvokeCallback(typeof(PickerCallback))]  private static void platform_file_picker_sz(IntPtr callback_data, int confirmed, IntPtr filename, int filename_length) => ProcessCallback(nameof(platform_file_picker_sz), callback_data, confirmed, filename, filename_length);

        
        
        sealed class MonoPInvokeCallbackAttribute : Attribute
        {
            public Type CallbackType { get; private set; }
            public MonoPInvokeCallbackAttribute(Type t) { CallbackType = t; }
        }
    }
}
