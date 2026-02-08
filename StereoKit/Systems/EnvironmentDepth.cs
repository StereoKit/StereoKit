using System;

namespace StereoKit
{
	/// <summary>EnvironmentDepth provides access to real-time depth sensing
	/// from the device, if available. Depth data is provided as a GPU
	/// texture with per-frame metadata including camera intrinsics and
	/// view/projection information for each eye.
	/// 
	/// This is currently backed by XR_META_environment_depth on OpenXR
	/// backends. If no depth provider is available, calls will gracefully
	/// return false or no-op.</summary>
	public static class EnvironmentDepth
	{
		/// <summary>A snapshot of environment depth data for the current
		/// frame. Contains a GPU texture with packed stereo depth, along
		/// with per-eye camera metadata needed to unproject the depth
		/// values into 3D positions.</summary>
		public struct Frame
		{
			/// <summary>Stereo depth texture as a 2D array with 2 layers
			/// (layer 0 = left eye, layer 1 = right eye) in D16_UNORM format.</summary>
			public Tex    Texture;
			/// <summary>Predicted display time used during acquisition,
			/// in OpenXR time units (nanoseconds).</summary>
			public long   DisplayTime;
			/// <summary>Width of a single eye's depth image, in pixels.</summary>
			public uint   Width;
			/// <summary>Height of a single eye's depth image, in pixels.</summary>
			public uint   Height;
			/// <summary>Near clip plane of the depth projection, in meters.</summary>
			public float  NearZ;
			/// <summary>Far clip plane of the depth projection, in meters.</summary>
			public float  FarZ;
			/// <summary>Depth camera metadata for the left eye.</summary>
			public View   Left;
			/// <summary>Depth camera metadata for the right eye.</summary>
			public View   Right;
		}

		/// <summary>Per-eye view metadata for a depth frame, providing the
		/// camera pose and field of view used to capture that eye's depth.</summary>
		public struct View
		{
			/// <summary>The pose of this eye's depth camera in world space.</summary>
			public Pose    Pose;
			/// <summary>The field of view of this eye's depth camera, in degrees.</summary>
			public FovInfo Fov;
		}

		/// <summary>True when the depth system is available on the current
		/// device and backend.</summary>
		public static bool IsAvailable
			=> NativeAPI.environment_depth_available();

		/// <summary>True while the depth provider is actively running.</summary>
		public static bool IsRunning
			=> NativeAPI.environment_depth_running();

		/// <summary>True if the current runtime supports depth hand removal configuration.</summary>
		public static bool SupportsHandRemoval
			=> NativeAPI.environment_depth_supports_hand_removal();

		/// <summary>Starts the depth provider. Use SetHandRemoval to
		/// configure hand removal separately before or after starting.</summary>
		/// <returns>True on success.</returns>
		public static bool Start()
			=> NativeAPI.environment_depth_start();

		/// <summary>Stops the depth provider.</summary>
		public static void Stop()
			=> NativeAPI.environment_depth_stop();

		/// <summary>Enables or disables hand removal when supported by the runtime.</summary>
		/// <param name="enabled">True to enable hand removal.</param>
		/// <returns>True on success.</returns>
		public static bool SetHandRemoval(bool enabled)
			=> NativeAPI.environment_depth_set_hand_removal(enabled);

		/// <summary>Retrieves the latest environment depth frame.</summary>
		/// <param name="frame">The latest depth frame data.</param>
		/// <returns>True if a frame was available.</returns>
		public static bool TryGetLatestFrame(out Frame frame)
		{
			frame = default;
			if (!NativeAPI.environment_depth_try_get_latest_frame(out EnvironmentDepthFrame native))
				return false;

			frame = new Frame
			{
				Texture     = native.texture != IntPtr.Zero ? new Tex(native.texture) : null,
				DisplayTime = native.displayTime,
				Width       = native.width,
				Height      = native.height,
				NearZ       = native.nearZ,
				FarZ        = native.farZ,
				Left = new View
				{
					Pose = native.left.pose,
					Fov  = native.left.fov,
				},
				Right = new View
				{
					Pose = native.right.pose,
					Fov  = native.right.fov,
				},
			};
			return true;
		}
	}
}