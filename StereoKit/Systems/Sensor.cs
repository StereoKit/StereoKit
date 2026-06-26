using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>The Sensor class provides access to device sensor data.
	/// Currently this includes depth sensing, with color camera support
	/// planned for the future.</summary>
	public static class Sensor
	{
		/// <summary>Provides access to real-time environment depth
		/// sensing from the device, if available, with per-frame metadata
		/// including each eye's pose and field of view. Depending on the
		/// backend, depth is delivered either as a GPU texture or as CPU
		/// buffers; prefer <see cref="Texture"/> on GPU-native backends and
		/// <see cref="TryGetLatestData"/> on CPU-native backends (either
		/// accessor works, but the off-native path incurs an upload or a
		/// readback). A backend may expose raw and/or smooth depth plus
		/// their confidence images (see <see cref="SensorDepthImage"/>).
		/// View poses are reprojected to display time (not the raw capture
		/// pose), so mind that when registering depth against another
		/// sensor such as a color camera. If no depth provider is
		/// available, calls gracefully return false or no-op.</summary>
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
			/// hand removal, raw/smooth depth, or confidence.</param>
			/// <param name="resolution">Preferred resolution (pixels per eye), chosen from
			/// <see cref="GetResolutions"/>; default (0x0) = the runtime's highest. Ignored
			/// where not selectable.</param>
			/// <returns>True on success.</returns>
			public static bool Start(SensorDepthCaps capabilities = SensorDepthCaps.None, SensorDepthResolution resolution = default)
				=> NativeAPI.sensor_depth_start(capabilities, resolution);

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

			/// <summary>Retrieves the latest CPU-accessible data for a specific depth
			/// image (raw/smooth depth or their confidence; default smooth depth) with
			/// matching per-eye metadata, when present this frame (see
			/// SensorDepthFrame.availableImages). The readback pipeline starts
			/// automatically on the first call and runs asynchronously, so the first few
			/// calls may return false, and the data may be 1-2 frames behind the GPU
			/// texture.</summary>
			/// <typeparam name="T">Element type: float for depth images (meters or ndc
			/// per depthFormat), byte for confidence images.</typeparam>
			/// <param name="info">The per-frame metadata matching this data.</param>
			/// <param name="data">An array filled with the data; reallocated if null or
			/// the wrong size.</param>
			/// <param name="viewIndex">-1 for all views (default), 0 left, 1 right.</param>
			/// <param name="image">Which image to fetch (default smooth depth).</param>
			/// <returns>True if the image was available.</returns>
			public static bool TryGetLatestData<T>(out SensorDepthFrame info, ref T[] data, int viewIndex = -1, SensorDepthImage image = SensorDepthImage.SmoothDepth) where T : struct
			{
				info = default;
				if (!TryGetLatestFrame(out SensorDepthFrame meta)) return false;
				if ((meta.availableImages & (1u << (int)image)) == 0) return false;

				int bpp = ImageBytesPerPixel(image, meta.depthFormat);
				if (bpp == 0) return false;
				int layers     = viewIndex < 0 ? (int)meta.viewCount : 1;
				int dataLength = (int)meta.width * (int)meta.height * bpp * layers / Marshal.SizeOf<T>();
				if (data == null || data.Length != dataLength)
					data = new T[dataLength];

				GCHandle pinned = GCHandle.Alloc(data, GCHandleType.Pinned);
				bool result = NativeAPI.sensor_depth_try_get_latest_data(out info, pinned.AddrOfPinnedObject(), out _, viewIndex, image);
				pinned.Free();
				return result;
			}

			/// <summary>The depth resolutions (pixels per eye) the current backend
			/// advertises, highest first. Empty where not selectable.</summary>
			/// <returns>Supported pixel sizes per eye.</returns>
			public static SensorDepthResolution[] GetResolutions()
			{
				NativeAPI.sensor_depth_get_resolutions(out IntPtr ptr, out int count);
				if (ptr == IntPtr.Zero || count <= 0) return Array.Empty<SensorDepthResolution>();
				SensorDepthResolution[] resolutions = new SensorDepthResolution[count];
				int stride = Marshal.SizeOf<SensorDepthResolution>();
				for (int i = 0; i < count; i++)
					resolutions[i] = Marshal.PtrToStructure<SensorDepthResolution>(ptr + i * stride);
				return resolutions;
			}

			private static int ImageBytesPerPixel(SensorDepthImage image, SensorDepthFormat format) =>
				image == SensorDepthImage.SmoothConfidence || image == SensorDepthImage.RawConfidence
					? 1
					: DepthFormatBytesPerPixel(format);

			private static int DepthFormatBytesPerPixel(SensorDepthFormat format) => format switch
			{
				SensorDepthFormat.NdcD16    => 2,
				SensorDepthFormat.MetersR32 => 4,
				_                           => 0,
			};
		}
	}
}
