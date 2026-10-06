namespace StereoKit
{
	/// <summary>Spatial is where you choose which kinds of scene
	/// understanding the device should run, like plane tracking, QR codes,
	/// or anchors, and how they're configured. Each of these is a
	/// `SpatialCapability`, and the things they discover show up as
	/// `SpatialEntity` objects.
	///
	/// Check `Capabilities` to see what the current device supports, then
	/// `Request` what you need. Capabilities start up asynchronously, and
	/// may need a permission first, so `Running` tells you which ones have
	/// actually started. Marker settings like `SetMarkerSize` and
	/// `ArucoDictionary` are best set before requesting, since changing
	/// them restarts that capability's tracking.</summary>
	public static class Spatial
	{
		/// <summary>The spatial capabilities the current device supports!
		/// This is None until an XR session with spatial entity support has
		/// initialized.</summary>
		public static SpatialCapability Capabilities => NativeAPI.spatial_capabilities();
		/// <summary>The capabilities that have been requested, via `Request`
		/// or by StereoKit systems like `Anchor`, minus any you've turned off
		/// with `Disable`.</summary>
		public static SpatialCapability Requested => NativeAPI.spatial_get_requested();
		/// <summary>The capabilities that have started up and are providing
		/// entities. This is a subset of `Requested`, since capabilities
		/// take a little time to start after being requested.</summary>
		public static SpatialCapability Running => NativeAPI.spatial_get_running();

		/// <summary>The components the device can provide on entities
		/// discovered by the given capability.</summary>
		/// <param name="capability">A single capability to look up.</param>
		/// <returns>All components the device supports for that capability.
		/// </returns>
		public static SpatialComponent ComponentsFor(SpatialCapability capability)
			=> NativeAPI.spatial_capability_components(capability);

		/// <summary>Does the current device support all of these
		/// capabilities? This is false until an XR session with spatial
		/// entity support has initialized.</summary>
		/// <param name="capabilities">One or more capabilities to check.</param>
		/// <returns>True if every capability given is supported.</returns>
		public static bool IsSupported(SpatialCapability capabilities)
			=> capabilities != SpatialCapability.None && (Capabilities & capabilities) == capabilities;

		/// <summary>Can this capability provide all of these components on
		/// this device? Handy for optional data like labels, which some
		/// devices don't provide.</summary>
		/// <param name="capability">A single capability to look up.</param>
		/// <param name="components">One or more components to check.</param>
		/// <returns>True if the capability can provide every component
		/// given.</returns>
		public static bool IsSupported(SpatialCapability capability, SpatialComponent components)
			=> components != SpatialComponent.None && (ComponentsFor(capability) & components) == components;

		/// <summary>Have all of these capabilities been requested, and not
		/// turned off with `Disable`? This doesn't mean they've started yet,
		/// see `IsRunning` for that.</summary>
		/// <param name="capabilities">One or more capabilities to check.</param>
		/// <returns>True if every capability given is requested.</returns>
		public static bool IsRequested(SpatialCapability capabilities)
			=> capabilities != SpatialCapability.None && (Requested & capabilities) == capabilities;

		/// <summary>Have all of these capabilities started up, and are they
		/// providing entities? Capabilities take a little time to start
		/// after being requested, and may wait on a permission first.</summary>
		/// <param name="capabilities">One or more capabilities to check.</param>
		/// <returns>True if every capability given is running.</returns>
		public static bool IsRunning(SpatialCapability capabilities)
			=> capabilities != SpatialCapability.None && (Running & capabilities) == capabilities;

		/// <summary>Request tracking for these capabilities, additively!
		/// This takes effect asynchronously, and entities will appear in the
		/// entity list as the system starts up and discovers them.
		/// Requesting or disabling one capability never disturbs entities
		/// belonging to another. If a capability needs a permission, this
		/// requests it automatically as a fallback, but requesting it
		/// yourself in advance via `Permission.Request` gives you control
		/// over when the user is asked, and lets you handle a denial. You can
		/// call this before `SK.Initialize`, and it takes effect once
		/// StereoKit starts.</summary>
		/// <param name="capabilities">One or more capabilities to request.
		/// Unsupported capabilities never start running.</param>
		public static void Request(SpatialCapability capabilities)
			=> NativeAPI.spatial_request(capabilities);

		/// <summary>Stop tracking these capabilities. Their entities leave
		/// the entity list, and any SpatialEntity identifiers you still
		/// hold stop resolving. Persisted entities are the exception, they
		/// wait with `Status` Pending, and come back if the capability is
		/// requested again. This also overrides StereoKit's own use of a
		/// capability. The `Anchor` system turns on
		/// `SpatialCapability.Anchor` the first time you use it, and calling
		/// this before `SK.Initialize` keeps `Anchor` from using spatial
		/// entities at all.</summary>
		/// <param name="capabilities">One or more capabilities to disable.</param>
		public static void Disable(SpatialCapability capabilities)
			=> NativeAPI.spatial_disable(capabilities);

		/// <summary>Tell the system how big your printed markers of this
		/// type are. This matters most for ArUco and AprilTags, where a
		/// known size can help a runtime judge marker distance, but it's
		/// only a hint, and runtimes may ignore it. Changing this while that
		/// marker type is being tracked restarts its tracking, so set it
		/// before `Request` when you can.</summary>
		/// <param name="type">The marker type this size applies to.</param>
		/// <param name="sizeMeters">The edge length of the marker's square,
		/// in meters. Use 0 if the sizes are mixed or unknown.</param>
		public static void SetMarkerSize(MarkerType type, float sizeMeters)
			=> NativeAPI.spatial_set_marker_size(type, sizeMeters);

		/// <summary>The physical marker size set via `SetMarkerSize`, 0 if
		/// unknown.</summary>
		/// <param name="type">The marker type to look up.</param>
		/// <returns>The marker's edge length in meters.</returns>
		public static float GetMarkerSize(MarkerType type)
			=> NativeAPI.spatial_get_marker_size(type);

		/// <summary>Tell the system whether markers of this type stay put,
		/// like a code taped to a wall, rather than being carried around.
		/// Stationary markers can have their pose refined over time instead
		/// of re-detected every frame. Changing this while that marker type
		/// is being tracked restarts its tracking, so set it before `Request`
		/// when you can. Not all devices use this, treat it as a hint.
		/// </summary>
		/// <param name="type">The marker type this applies to.</param>
		/// <param name="stationary">True if these markers never move.</param>
		public static void SetMarkerStationary(MarkerType type, bool stationary)
			=> NativeAPI.spatial_set_marker_stationary(type, stationary);

		/// <summary>Whether this marker type was marked as stationary via
		/// `SetMarkerStationary`.</summary>
		/// <param name="type">The marker type to look up.</param>
		/// <returns>True if these markers are expected to stay put.</returns>
		public static bool GetMarkerStationary(MarkerType type)
			=> NativeAPI.spatial_get_marker_stationary(type);

		/// <summary>Which family of ArUco markers `SpatialCapability.Aruco`
		/// looks for. Markers from other dictionaries aren't detected, so
		/// this must match the markers you printed! Default lets StereoKit
		/// pick. Changing it while ArUco markers are being tracked restarts
		/// that tracking.</summary>
		public static ArucoDict ArucoDictionary {
			get => NativeAPI.spatial_get_aruco_dictionary();
			set => NativeAPI.spatial_set_aruco_dictionary(value); }

		/// <summary>Which family of AprilTag markers
		/// `SpatialCapability.AprilTag` looks for. Markers from other
		/// dictionaries aren't detected, so this must match the markers you
		/// printed! Default lets StereoKit pick. Changing it while AprilTags
		/// are being tracked restarts that tracking.</summary>
		public static AprilTagDict AprilTagDictionary {
			get => NativeAPI.spatial_get_april_tag_dictionary();
			set => NativeAPI.spatial_set_april_tag_dictionary(value); }
	}
}
