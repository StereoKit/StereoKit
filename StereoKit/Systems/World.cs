using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	public enum SpatialNodeType { Static, Dynamic }

	/// <summary>World contains information about the real world around the 
	/// user. This includes things like play boundaries, scene understanding,
	/// and other various things.</summary>
	public static class World
	{
		/// <summary>This refers to the play boundary, or guardian system
		/// that the system may have! Not all systems have this, so it's
		/// always a good idea to check this first!</summary>
		public static bool HasBounds  => NativeAPI.world_has_bounds();
		/// <summary>This is the size of a rectangle within the play
		/// boundary/guardian's space, in meters if one exists. Check
		/// `World.BoundsPose` for the center point and orientation of the
		/// boundary, and check `World.HasBounds` to see if it exists at all!
		/// </summary>
		public static Vec2 BoundsSize => NativeAPI.world_get_bounds_size();
		/// <summary>This is the orientation and center point of the system's
		/// boundary/guardian. This can be useful to find the floor height!
		/// Not all systems have a boundary, so be sure to check 
		/// `World.HasBounds` first.</summary>
		public static Pose BoundsPose => NativeAPI.world_get_bounds_pose();

		/// <summary>What information should StereoKit use to determine when
		/// the next world data refresh happens? See the `WorldRefresh` enum
		/// for details.</summary>
		public static WorldRefresh RefreshType { 
			get => NativeAPI.world_get_refresh_type(); 
			set => NativeAPI.world_set_refresh_type(value); }

		/// <summary>Radius, in meters, of the area that StereoKit should
		/// scan for world data. Default is 4. When using the 
		/// `WorldRefresh.Area` refresh type, the world data will refresh
		/// when the user has traveled half this radius from the center of
		/// where the most recent refresh occurred. </summary>
		public static float RefreshRadius {
			get => NativeAPI.world_get_refresh_radius();
			set => NativeAPI.world_set_refresh_radius(value); }

		/// <summary>The refresh interval speed, in seconds. This is only
		/// applicable when using `WorldRefresh.Timer` for the refresh type.
		/// Note that the system may not be able to refresh as fast as you
		/// wish, and in that case, StereoKit will always refresh as soon as 
		/// the previous refresh finishes.</summary>
		public static float RefreshInterval {
			get => NativeAPI.world_get_refresh_interval();
			set => NativeAPI.world_set_refresh_interval(value);
		}

		/// <summary>Converts a Windows Mirage spatial node GUID into a Pose
		/// based on its current position and rotation! Check
		/// SK.System.spatialBridgePresent to see if this is available to
		/// use. Currently only on HoloLens, good for use with the Windows
		/// QR code package.</summary>
		/// <param name="spatialNodeGuid">A Windows Mirage spatial node GUID
		/// acquired from a windows MR API call.</param>
		/// <param name="spatialNodeType">Type of spatial node to locate.</param>
		/// <param name="qpcTime">A windows performance counter timestamp at
		/// which the node should be located, obtained from another API or
		/// with System.Diagnostics.Stopwatch.GetTimestamp().</param>
		/// <returns>A Pose representing the current orientation of the
		/// spatial node.</returns>
		public static Pose FromSpatialNode(Guid spatialNodeGuid, SpatialNodeType spatialNodeType = SpatialNodeType.Static, long qpcTime = 0)
			=> NativeAPI.world_from_spatial_graph(spatialNodeGuid.ToByteArray(), spatialNodeType == SpatialNodeType.Dynamic ? 1 : 0, qpcTime);

		public static bool FromSpatialNode(Guid spatialNodeGuid, out Pose pose, SpatialNodeType spatialNodeType = SpatialNodeType.Static, long qpcTime = 0)
			=> NativeAPI.world_try_from_spatial_graph(spatialNodeGuid.ToByteArray(), spatialNodeType == SpatialNodeType.Dynamic ? 1 : 0, qpcTime, out pose) > 0;

		/// <summary>Converts a Windows.Perception.Spatial.SpatialAnchor's pose
		/// into SteroKit's coordinate system. This can be great for
		/// interacting with some of the UWP spatial APIs such as WorldAnchors.
		/// 
		/// This method only works on UWP platforms, check 
		/// SK.System.perceptionBridgePresent to see if this is available.
		/// </summary>
		/// <param name="perceptionSpatialAnchor">A valid
		/// Windows.Perception.Spatial.SpatialAnchor.</param>
		/// <returns>A Pose representing the current orientation of the
		/// SpatialAnchor.</returns>
		public static Pose FromPerceptionAnchor(object perceptionSpatialAnchor)
		{
			IntPtr unknown = Marshal.GetIUnknownForObject(perceptionSpatialAnchor);
			Pose   result  = NativeAPI.world_from_perception_anchor(unknown);
			Marshal.Release(unknown);
			return result;
		}

		public static bool FromPerceptionAnchor(object perceptionSpatialAnchor, out Pose pose)
		{
			IntPtr unknown = Marshal.GetIUnknownForObject(perceptionSpatialAnchor);
			int    result  = NativeAPI.world_try_from_perception_anchor(unknown, out pose);
			Marshal.Release(unknown);
			return result>0;
		}

		/// <summary>World.RaycastEnabled must be set to true first! 
		/// SK.System.worldRaycastPresent must also be true. This does a ray
		/// intersection with whatever represents the environment at the
		/// moment! In this case, it's a watertight collection of low
		/// resolution meshes calculated by the Scene Understanding
		/// extension, which is only provided by the Microsoft HoloLens
		/// runtime.</summary>
		/// <param name="ray">A world space ray that you'd like to try
		/// intersecting with the world mesh.</param>
		/// <param name="intersection">The location of the intersection, and
		/// direction of the world's surface at that point. This is only
		/// valid if the method returns true.</param>
		/// <returns>True if an intersection is detected, false if raycasting
		/// is disabled, or there was no intersection.</returns>
		public static bool Raycast(Ray ray, out Ray intersection)
			=> NativeAPI.world_raycast(ray, out intersection);

		/// <summary>Off by default. This tells StereoKit to load up and
		/// display an occlusion surface that allows the real world to
		/// occlude the application's digital content! Most systems may allow
		/// you to customize the visual appearance of this occlusion surface
		/// via the World.OcclusionMaterial.
		/// Check SK.System.worldOcclusionPresent to see if occlusion can be
		/// enabled. This will reset itself to false if occlusion isn't
		/// possible. Loading occlusion data is asynchronous, so occlusion
		/// may not occur immediately after setting this flag.</summary>
		public static bool OcclusionEnabled { 
			get => NativeAPI.world_get_occlusion_enabled();
			set => NativeAPI.world_set_occlusion_enabled(value); }

		/// <summary>Off by default. This tells StereoKit to load up 
		/// collision meshes for the environment, for use with World.Raycast.
		/// Check SK.System.worldRaycastPresent to see if raycasting can be
		/// enabled. This will reset itself to false if raycasting isn't
		/// possible. Loading raycasting data is asynchronous, so collision
		/// surfaces may not be available immediately after setting this
		/// flag.</summary>
		public static bool RaycastEnabled { 
			get => NativeAPI.world_get_raycast_enabled();
			set => NativeAPI.world_set_raycast_enabled(value); }

		/// <summary>By default, this is a black(0,0,0,0) opaque unlit
		/// material that will occlude geometry, but won't show up as visible
		/// anywhere. You can override this with whatever material you would
		/// like.</summary>
		public static Material OcclusionMaterial {
			get => new Material(NativeAPI.world_get_occlusion_material());
			set => NativeAPI.world_set_occlusion_material(value._inst); }


	}
}
