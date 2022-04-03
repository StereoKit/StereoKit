using StereoKit;

class DocBounds : ITest
{
	public void Initialize()
	{
		/// :CodeSample: Bounds Bounds.FromCorner Bounds.FromCorners Bounds.Intersect
		/// ### General Usage
		/// 
		// All these create bounds for a 1x1x1m cube around the origin!
		Bounds bounds = new Bounds(Vec3.One);
		bounds = Bounds.FromCorner(new Vec3(-0.5f, -0.5f, -0.5f), Vec3.One);
		bounds = Bounds.FromCorners(
			new Vec3(-0.5f, -0.5f, -0.5f),
			new Vec3( 0.5f,  0.5f,  0.5f));

		// Note that positions must be in a coordinate space relative to 
		// the bounds!
		if (bounds.Contains(new Vec3(0,0.25f,0)))
			Log.Info("Super easy to check if something's in it!");

		// Casting a ray at a bounds is trivial too, again, ensure 
		// coordinates are in the same space!
		Ray ray = new Ray(Vec3.Up, -Vec3.Up);
		if (bounds.Intersect(ray, out Vec3 at))
			Log.Info("Bounds intersection at " + at); // <0, 0.5f, 0>

		// You can also scale a Bounds using the '*' operator overload, 
		// this is really useful if you're working with the Bounds of a
		// Model that you've scaled. It will scale the center as well as
		// the size!
		bounds = bounds * 0.5f;

		// Scale the current bounds reference using 'Scale'
		bounds.Scale(0.5f);

		// Scale the bounds by a Vec3
		bounds = bounds * new Vec3(1, 10, 0.5f);
		/// :End:
	}

	public void Shutdown() { }
	public void Update() { }
}