using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>For use with World.FromSpatialNode, this indicates the type of
	/// node that's being bridged with OpenXR.</summary>
	public enum SpatialNodeType 
	{
		/// <summary>Static spatial nodes track the pose of a fixed location in
		/// the world relative to reference spaces. The tracking of static
		/// nodes may slowly adjust the pose over time for better accuracy but
		/// the pose is relatively stable in the short term, such as between
		/// rendering frames. For example, a QR code tracking library can use a
		/// static node to represent the location of the tracked QR code.
		/// </summary>
		Static,
		/// <summary>Dynamic spatial nodes track the pose of a physical object
		/// that moves continuously relative to reference spaces. The pose of
		/// dynamic spatial nodes can be very different within the duration of
		/// a rendering frame. It is important for the application to use the
		/// correct timestamp to query the space location. For example, a color
		/// camera mounted in front of a HMD is also tracked by the HMD so a
		/// web camera library can use a dynamic node to represent the camera
		/// location.</summary>
		Dynamic
	}

	/// <summary>World contains information about the real world around the 
	/// user. This includes things like play boundaries, scene understanding,
	/// and other various things.</summary>
	public static class World
	{
		/// <summary>This refers to the play boundary, or guardian system
		/// that the system may have! Not all systems have this, so it's
		/// always a good idea to check this first!</summary>
		public static bool HasBounds  => NB.Bool(NativeAPI.world_has_bounds());
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

		/// <summary>The mode or "reference space" that StereoKit uses for
		/// determining its base origin. This is determined by the initial
		/// value provided in SKSettings.origin, as well as by support from the
		/// underlying runtime. The mode reported here will _not_ necessarily
		/// be the one requested in initialization, as fallbacks are
		/// implemented using different available modes.</summary>
		public static OriginMode OriginMode => NativeAPI.world_get_origin_mode();

		/// <summary>This reports the status of the device's positional
		/// tracking. If the room is too dark, or a hand is covering tracking
		/// sensors, or some other similar 6dof tracking failure, this would
		/// report as not tracked.
		/// 
		/// Note that this does not factor in the status of rotational
		/// tracking. Rotation is typically done via gyroscopes/accelerometers,
		/// which don't really fail the same way positional tracking system
		/// can.</summary>
		public static BtnState Tracked => NativeAPI.world_get_tracked();

		/// <summary>This is relative to the base reference point and is NOT
		/// in world space! The origin StereoKit uses is actually a base
		/// reference point combined with an offset! You can use this to read
		/// or set the offset from the OriginMode reference point. </summary>
		public static Pose OriginOffset
		{
			get => NativeAPI.world_get_origin_offset();
			set => NativeAPI.world_set_origin_offset(value);
		}

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
			=> NativeAPI.world_from_spatial_graph(spatialNodeGuid.ToByteArray(), NB.Int(spatialNodeType == SpatialNodeType.Dynamic), qpcTime);

		/// <summary>Converts a Windows Mirage spatial node GUID into a Pose
		/// based on its current position and rotation! Check
		/// SK.System.spatialBridgePresent to see if this is available to
		/// use. Currently only on HoloLens, good for use with the Windows
		/// QR code package.</summary>
		/// <param name="spatialNodeGuid">A Windows Mirage spatial node GUID
		/// acquired from a windows MR API call.</param>
		/// <param name="pose">A resulting Pose representing the current
		/// orientation of the spatial node.</param>
		/// <param name="spatialNodeType">Type of spatial node to locate.</param>
		/// <param name="qpcTime">A windows performance counter timestamp at
		/// which the node should be located, obtained from another API or
		/// with System.Diagnostics.Stopwatch.GetTimestamp().</param>
		/// <returns>True if FromSpatialNode succeeded, and false if it failed.
		/// </returns>
		public static bool FromSpatialNode(Guid spatialNodeGuid, out Pose pose, SpatialNodeType spatialNodeType = SpatialNodeType.Static, long qpcTime = 0)
		{ unsafe {
			fixed (Pose* posePtr = &pose)
			return NB.Bool(NativeAPI.world_try_from_spatial_graph(spatialNodeGuid.ToByteArray(), NB.Int(spatialNodeType == SpatialNodeType.Dynamic), qpcTime, posePtr));
		} }

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
			if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
			{
				unsafe
				{
					IntPtr unknown = Marshal.GetIUnknownForObject(perceptionSpatialAnchor);
					Pose   result  = NativeAPI.world_from_perception_anchor((void*)unknown);
					Marshal.Release(unknown);
					return result;
				}
			}
			else
			{
				return Pose.Identity;
			}
		}

		/// <summary>Converts a Windows.Perception.Spatial.SpatialAnchor's pose
		/// into SteroKit's coordinate system. This can be great for
		/// interacting with some of the UWP spatial APIs such as WorldAnchors.
		/// 
		/// This method only works on UWP platforms, check 
		/// SK.System.perceptionBridgePresent to see if this is available.
		/// </summary>
		/// <param name="perceptionSpatialAnchor">A valid
		/// Windows.Perception.Spatial.SpatialAnchor.</param>
		/// <param name="pose">A resulting Pose representing the current
		/// orientation of the spatial node.</param>
		/// <returns>A Pose representing the current orientation of the
		/// SpatialAnchor.</returns>
		public static bool FromPerceptionAnchor(object perceptionSpatialAnchor, out Pose pose)
		{ 
			if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
			{
				unsafe
				{
					IntPtr unknown = Marshal.GetIUnknownForObject(perceptionSpatialAnchor);
					bool   result;
					fixed (Pose* posePtr = &pose)
					result = NB.Bool(NativeAPI.world_try_from_perception_anchor((void*)unknown, posePtr));
					Marshal.Release(unknown);
					return result;
				}
			}
			else
			{
				pose = Pose.Identity;
				return false;
			}
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
		{ unsafe {
			fixed (Ray*intersectionPtr = &intersection)
			return NB.Bool(NativeAPI.world_raycast(ray, intersectionPtr));
		} }

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
			get => NB.Bool(NativeAPI.world_get_occlusion_enabled());
			set => NativeAPI.world_set_occlusion_enabled(NB.Int(value)); }

		/// <summary>Off by default. This tells StereoKit to load up 
		/// collision meshes for the environment, for use with World.Raycast.
		/// Check SK.System.worldRaycastPresent to see if raycasting can be
		/// enabled. This will reset itself to false if raycasting isn't
		/// possible. Loading raycasting data is asynchronous, so collision
		/// surfaces may not be available immediately after setting this
		/// flag.</summary>
		public static bool RaycastEnabled { 
			get => NB.Bool(NativeAPI.world_get_raycast_enabled());
			set => NativeAPI.world_set_raycast_enabled(NB.Int(value)); }

		/// <summary>By default, this is a black(0,0,0,0) opaque unlit
		/// material that will occlude geometry, but won't show up as visible
		/// anywhere. You can override this with whatever material you would
		/// like.</summary>
		public static Material OcclusionMaterial {
			get => new Material(NativeAPI.world_get_occlusion_material());
			set => NativeAPI.world_set_occlusion_material(value._inst); }


	}
}
