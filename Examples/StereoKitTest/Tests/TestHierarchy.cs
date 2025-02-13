using StereoKit;

class TestHierarchy : ITest
{
	public void Initialize() {}
	public void Shutdown() {}

	public void Step()
	{
		// A simple test for verifying inheriting vs ignoring in transform
		// hierarchies.
		Hierarchy.Push(Matrix.TS(0,0,-0.5f, 0.5f));
			Mesh.Cube.Draw(Material.Default, Matrix.S(0.1f), Color.HSV(0,0.7f,1.0f));

			Hierarchy.Push(Matrix.TS(0.25f, 0, -0.5f, 0.5f), HierarchyParent.Inherit);
				Mesh.Cube.Draw(Material.Default, Matrix.S(0.1f), Color.HSV(0.3f, 0.7f, 1.0f));
				Tests.Test(TestInheritHierarchy);
			Hierarchy.Pop();

			Hierarchy.Push(Matrix.TS(0.25f,0, -0.5f, 0.5f), HierarchyParent.Ignore);
				Mesh.Cube.Draw(Material.Default, Matrix.S(0.1f), Color.HSV(0.6f, 0.7f, 1.0f));
				Tests.Test(TestIgnoreHierarchy);
			Hierarchy.Pop();

		Hierarchy.Pop();

		Tests.Screenshot("Tests/Hierarchy.jpg", 400, 200, 20, V.XYZ(0.125f, 0, 0), V.XYZ(0.125f, 0, -0.5f));
	}

	bool TestInheritHierarchy() => Vec3.Distance( Hierarchy.ToWorld(Vec3.Zero), new Vec3(0.125f, 0, -0.75f) ) < 0.00001f;
	bool TestIgnoreHierarchy () => Vec3.Distance( Hierarchy.ToWorld(Vec3.Zero), new Vec3(0.25f, 0, -0.5f) ) < 0.00001f;
}
