using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>The Sensor class provides access to device sensor data.
	/// Currently this includes depth sensing, with color camera support
	/// planned for the future.</summary>
	public static class Sensor
	{
		/// <summary>Provides access to real-time depth sensing from the
		/// device, if available. Depth data is provided as a GPU texture
		/// with per-frame metadata including camera intrinsics and
		/// view/projection information for each eye.
		///
		/// This is currently backed by XR_META_environment_depth on OpenXR
		/// backends. If no depth provider is available, calls will
		/// gracefully return false or no-op.</summary>
		public static class Depth
		{
			/// <summary>True when the depth system is available on the current
			/// device and backend.</summary>
			public static bool IsAvailable
				=> NativeAPI.sensor_depth_available();

			/// <summary>True while the depth provider is actively running.</summary>
			public static bool IsRunning
				=> NativeAPI.sensor_depth_running();

			/// <summary>The system-managed depth texture. Available once the
			/// depth sensor has been started. Dimensions and format are
			/// valid after the first frame.</summary>
			public static Tex Texture
			{
				get {
					IntPtr ptr = NativeAPI.sensor_depth_get_texture();
					if (ptr == IntPtr.Zero) return null;
					NativeAPI.tex_addref(ptr);
					return new Tex(ptr);
				}
			}

			/// <summary>Returns a bitmask of SensorDepthCaps indicating
			/// which optional features are supported on the current platform
			/// and backend.</summary>
			/// <returns>Supported capability capabilities.</returns>
			public static SensorDepthCaps GetCapabilities()
				=> NativeAPI.sensor_depth_get_capabilities();

			/// <summary>Starts the depth provider with the given capabilities.
			/// Unsupported capabilities for the current platform are silently
			/// ignored. Must be called before TryGetLatestFrame will return
			/// data.</summary>
			/// <param name="capabilities">Optional capabilities to configure features like
			/// hand removal or CPU data readback.</param>
			/// <returns>True on success.</returns>
			public static bool Start(SensorDepthCaps capabilities = SensorDepthCaps.None)
				=> NativeAPI.sensor_depth_start(capabilities);

			/// <summary>Stops the depth provider.</summary>
			public static void Stop()
				=> NativeAPI.sensor_depth_stop();

			/// <summary>Updates the active capabilities while the sensor is running.
			/// Can enable or disable features like hand removal or CPU
			/// readback at runtime. Unsupported capabilities are silently ignored.
			/// </summary>
			/// <param name="capabilities">New set of capabilities to apply.</param>
			/// <returns>True if the sensor is running and capabilities were applied.</returns>
			public static bool SetCapabilities(SensorDepthCaps capabilities)
				=> NativeAPI.sensor_depth_set_capabilities(capabilities);

			/// <summary>Retrieves the latest per-frame depth metadata.
			/// </summary>
			/// <param name="info">The latest per-frame metadata.</param>
			/// <returns>True if a frame was available.</returns>
			public static bool TryGetLatestFrame(out SensorDepthFrame info)
				=> NativeAPI.sensor_depth_try_get_latest_frame(out info);

			/// <summary>Retrieves the latest CPU-accessible depth data
			/// with matching per-eye metadata. The readback pipeline
			/// starts automatically on the first call and runs
			/// asynchronously, so the first few calls may return false.
			/// The returned data is typically 1-2 frames behind the GPU
			/// texture, but can be more under heavy GPU load.</summary>
			/// <typeparam name="T">The struct type to interpret the depth
			/// data as. For depth16 format, use ushort.</typeparam>
			/// <param name="info">The per-frame metadata matching this data.</param>
			/// <param name="data">An array that will be filled with the
			/// depth data. If null or the wrong size, it will be
			/// reallocated.</param>
			/// <param name="viewIndex">Which view to read back: -1 for
			/// all views (default), 0 for the first view, 1 for the
			/// second view.</param>
			/// <returns>True if data was available.</returns>
			public static bool TryGetLatestData<T>(out SensorDepthFrame info, ref T[] data, int viewIndex = -1) where T : struct
			{
				info = default;
				Tex tex = Texture;
				if (tex == null) return false;

				int bpp = FormatBytesPerPixel(tex.Format);
				if (bpp == 0)
				{
					Log.Err($"The texture format '{tex.Format}' is not a supported depth format.");
					return false;
				}

				int viewCount  = viewIndex < 0 ? 2 : 1;
				int dataBytes  = tex.Width * tex.Height * bpp * viewCount;
				int dataLength = dataBytes / Marshal.SizeOf<T>();
				if (data == null || data.Length != dataLength)
					data = new T[dataLength];

				GCHandle pinned = GCHandle.Alloc(data, GCHandleType.Pinned);
				bool result = NativeAPI.sensor_depth_try_get_latest_data(out info, pinned.AddrOfPinnedObject(), out _, viewIndex);
				pinned.Free();

				return result;
			}

			private static int FormatBytesPerPixel(TexFormat format) => format switch
			{
				TexFormat.Depth16   => 2,
				TexFormat.Depth32   => 4,
				TexFormat.Depth24s8 => 4,
				TexFormat.Depth32s8 => 8,
				TexFormat.Depth16s8 => 4,
				_                   => 0,
			};
		}
	}
}
