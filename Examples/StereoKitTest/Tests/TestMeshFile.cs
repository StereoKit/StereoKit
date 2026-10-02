using StereoKit;

class TestMeshFile : ITest
{
	Mesh  svg;
	Mesh  svgMem;
	Mesh  stl;
	Mesh  ply;
	Model model;
	bool  tested;

	const string plyText =
		"ply\nformat ascii 1.0\n" +
		"element vertex 3\nproperty float x\nproperty float y\nproperty float z\n" +
		"element face 1\nproperty list uchar uint vertex_indices\nend_header\n" +
		"0 0 0\n1 0 0\n0 1 0\n3 0 1 2\n";

	public void Initialize()
	{
		svg    = Mesh .FromFile  ("StereoKitWideLight.svg");
		svgMem = Mesh .FromMemory("StereoKitWideLight.svg", Platform.ReadFileBytes("StereoKitWideLight.svg"));
		stl    = Mesh .FromFile  ("suzanne_bin.stl");
		ply    = Mesh .FromMemory(".ply", System.Text.Encoding.UTF8.GetBytes(plyText));
		model  = Model.FromFile  ("StereoKitWideLight.svg");
	}

	public void Shutdown() { }

	public void Step()
	{
		// The test harness blocks on all assets after Initialize, so these
		// have finished loading by now.
		if (!tested)
		{
			tested = true;
			Tests.Test(() => svg   .AssetState == AssetState.Loaded && svg.VertCount > 0);
			Tests.Test(() => svgMem.AssetState == AssetState.Loaded && svgMem.VertCount == svg.VertCount);
			// The Model reuses the Mesh already loaded from the same file.
			Tests.Test(() => model .AssetState == AssetState.Loaded && model.Visuals[0].Mesh.Id == svg.Id && model.Visuals[0].Mesh.VertCount == svg.VertCount);
			Tests.Test(() => stl   .AssetState == AssetState.Loaded && stl.VertCount > 0 && stl.IndCount > 0);
			Tests.Test(() => ply   .AssetState == AssetState.Loaded && ply.VertCount == 3 && ply.IndCount == 3);
			// OBJ carries materials, which a lone Mesh can't represent.
			Log.Warn("Expected error:");
			Tests.Test(() => Mesh.FromFile("suzanne.obj") == null);
			// The SVG's height normalizes to one meter, and this logo's art
			// fills about two thirds of it.
			Tests.Test(() => svg.Bounds.dimensions.y > 0.6f && svg.Bounds.dimensions.y < 0.7f);
			Tests.Test(() => svg.Bounds.dimensions.x > 3.0f && svg.Bounds.dimensions.x < 3.5f);
		}

		// In front of the default camera, turned to face it.
		Hierarchy.Push(Matrix.TR(0, 0, -0.5f, Quat.FromAngles(0, 180, 0)));
		svg  .Draw(Material.Unlit,   Matrix.TS(0,  0.00f, 0, 0.2f));
		model.Draw(                  Matrix.TS(0, -0.16f, 0, 0.2f));
		// The STL is authored in millimeters, so size it by its bounds.
		stl  .Draw(Material.Default, Matrix.TS(0,  0.14f, 0, 0.15f / stl.Bounds.dimensions.y));
		Hierarchy.Pop();
		Tests.Screenshot("Tests/MeshFile.jpg", 400, 400, 70, V.XYZ(0, 0, 0), V.XYZ(0, 0, -0.5f));
	}
}
