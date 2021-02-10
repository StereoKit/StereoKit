using StereoKit;

class DocEyes : ITest
{
	public void Initialize() { }
	public void Shutdown  () { }

	public void Update()
	{
		/// :CodeSample: Input.EyesTracked Input.Eyes
		if (Input.EyesTracked.IsActive())
		{
			// Intersect the eye Ray with a floor plane
			Plane plane = new Plane(Vec3.Zero, Vec3.Up);
			if (Input.Eyes.Ray.Intersect(plane, out Vec3 at))
			{
				Default.MeshSphere.Draw(Default.Material, Matrix.TS(at, .05f));
			}
		}
		/// :End:
	}
}
