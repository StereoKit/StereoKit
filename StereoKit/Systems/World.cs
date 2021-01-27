using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>World contains information about the real world around the 
	/// user. This includes things like play boundaries, scene understanding,
	/// and other various things.</summary>
	public static class World
	{
		public static bool HasBounds  => NativeAPI.world_has_bounds();
		public static Vec2 BoundsSize => NativeAPI.world_get_bounds_size();
		public static Pose BoundsPose => NativeAPI.world_get_bounds_pose();

		/// <summary>Converts a Windows Mirage spatial node GUID into a Pose
		/// based on its current position and rotation! Check 
		/// StereoKitApp.System.spatialBridge to see if this is available to
		/// use. Currently only on HoloLens, good for use with the Windows
		/// QR code package.</summary>
		/// <param name="spatialNodeGuid">A Windows Mirage spatial node GUID
		/// aquired from a windows MR API call.</param>
		/// <returns>A Pose representing the current orientation of the
		/// spatial node.</returns>
		public static Pose FromSpatialNode(Guid spatialNodeGuid)
			=> NativeAPI.world_from_spatial_graph(spatialNodeGuid.ToByteArray());

		public static Pose FromPerceptionAnchor(object perceptionSpatialAnchor)
		{
			IntPtr unknown = Marshal.GetIUnknownForObject(perceptionSpatialAnchor);
			Pose   result  = NativeAPI.world_from_perception_anchor(unknown);
			Marshal.Release(unknown);
			return result;
		}
	}
}
