using System.Numerics;

namespace StereoKit
{
	/// <summary>World contains information about the real world around the 
	/// user. This includes things like play boundaries, scene understanding,
	/// and other various things.</summary>
	public static class World
	{
		public static bool    HasBounds => NativeAPI.world_has_bounds();
		public static Vector2 BoundsSize => NativeAPI.world_get_bounds_size();
		public static Pose    BoundsPose => NativeAPI.world_get_bounds_pose();
	}
}
