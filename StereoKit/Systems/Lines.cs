namespace StereoKit
{
	/// <summary>A line drawing class! This is an easy way to visualize lines
	/// or relationships between objects. The current implementation uses a
	/// quad strip that always faces the user, via vertex shader
	/// manipulation.</summary>
	public static class Lines
	{
		/// <summary>Adds a line to the environment for the current frame.
		/// </summary>
		/// <param name="start">Starting point of the line.</param>
		/// <param name="end">End point of the line.</param>
		/// <param name="color">Color for the line, this is embedded in the
		/// vertex color of the line.</param>
		/// <param name="thickness">Thickness of the line in meters.</param>
		public static void Add(Vec3 start, Vec3 end, Color32 color, float thickness)
			=>NativeAPI.line_add(start, end, color, color, thickness);

		/// <summary>Adds a line to the environment for the current frame.
		/// </summary>
		/// <param name="start">Starting point of the line.</param>
		/// <param name="end">End point of the line.</param>
		/// <param name="colorStart">Color for the start of the line, this is
		/// embedded in the vertex color of the line.</param>
		/// <param name="colorEnd">Color for the end of the line, this is
		/// embedded in the vertex color of the line.</param>
		/// <param name="thickness">Thickness of the line in meters.</param>
		public static void Add(Vec3 start, Vec3 end, Color32 colorStart, Color32 colorEnd, float thickness)
			=> NativeAPI.line_add(start, end, colorStart, colorEnd, thickness);

		/// <summary>Adds a line based on a ray to the environment for the
		/// current frame.</summary>
		/// <param name="ray">The ray we want to visualize!</param>
		/// <param name="length">How long should the ray be? Actual length
		/// will be ray.direction.Magnitude * length.</param>
		/// <param name="color">Color for the line, this is embedded in the
		/// vertex color of the line.</param>
		/// <param name="thickness">Thickness of the line in meters.</param>
		public static void Add(Ray ray, float length, Color32 color, float thickness)
			=> NativeAPI.line_add(ray.position, ray.At(length), color, color, thickness);

		/// <summary>Adds a line from a list of line points to the
		/// environment. This does not close the path, so if you want it
		/// closed, you'll have to add an extra point or two at the end
		/// yourself!</summary>
		/// <param name="points">An array of line points.</param>
		public static void Add(in LinePoint[] points)
			=> NativeAPI.line_add_listv(points, points.Length);

		/// <inheritdoc cref="AddAxis(Pose, float)"/>
		/// <param name="thickness">How thick should the lines be, in meters?
		/// </param>
		public static void AddAxis(Pose atPose, float size, float thickness)
		{
			Add(atPose.position, atPose.position + Vec3.UnitX   * atPose.orientation * size,        new Color32(255,0,  0,  255), thickness);
			Add(atPose.position, atPose.position + Vec3.UnitY   * atPose.orientation * size,        new Color32(0,  255,0,  255), thickness);
			Add(atPose.position, atPose.position + Vec3.UnitZ   * atPose.orientation * size,        new Color32(0,  0,  255,255), thickness);
			Add(atPose.position, atPose.position + Vec3.Forward * atPose.orientation * size * 0.5f, new Color32(255,255,255,255), thickness);
		}

		/// <summary>Displays an RGB/XYZ axis widget at the pose! Each line
		/// is extended along the positive direction of each axis, so the red
		/// line is +X, green is +Y, and blue is +Z. A white line is drawn
		/// along -Z to indicate the Forward vector of the pose (-Z is 
		/// forward in StereoKit).</summary>
		/// <param name="atPose">What position and orientation do we want
		/// this axis widget at?</param>
		/// <param name="size">How long should the widget lines be, in
		/// meters?</param>
		public static void AddAxis(Pose atPose, float size = U.cm)
			=> AddAxis(atPose, size, size * 0.1f);
	}
}
