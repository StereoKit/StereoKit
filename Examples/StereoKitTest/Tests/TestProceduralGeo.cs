using StereoKit;

class TestProceduralGeo : ITest
{
	Mesh meshCube        = null;
	Mesh meshRoundedCube = null;
	Mesh meshSphere      = null;
	Mesh meshCylinder    = null;
	Mesh meshPlane       = null;

	Material material;
	Material materialWire;

	public void Initialize() {
		Tex uvTex = Tex.FromFile("UVTex.png");
		uvTex.SampleMode = TexSample.Point;

		material = Default.Material.Copy();
		material[MatParamName.DiffuseTex] = uvTex;

		materialWire = Default.MaterialUnlit.Copy();
		materialWire.Wireframe = true;
		materialWire.DepthTest = DepthTest.Always;

		/// :CodeSample: Mesh.GenerateCube
		/// ### UV and Face layout
		/// Here's a test image that illustrates how this mesh's geometry is
		/// laid out.
		/// ![Procedural Cube Mesh]({{site.screen_url}}/ProcGeoCube.jpg)
		meshCube = Mesh.GenerateCube(Vec3.One);
		/// :End:
		/// :CodeSample: Mesh.GenerateRoundedCube
		/// ### UV and Face layout
		/// Here's a test image that illustrates how this mesh's geometry is
		/// laid out.
		/// ![Procedural Cube Mesh]({{site.screen_url}}/ProcGeoRoundedCube.jpg)
		meshRoundedCube = Mesh.GenerateRoundedCube(Vec3.One, 0.05f);
		/// :End:
		/// :CodeSample: Mesh.GenerateSphere
		/// ### UV and Face layout
		/// Here's a test image that illustrates how this mesh's geometry is
		/// laid out.
		/// ![Procedural Cube Mesh]({{site.screen_url}}/ProcGeoSphere.jpg)
		meshSphere = Mesh.GenerateSphere(1);
		/// :End:
		/// :CodeSample: Mesh.GenerateCylinder
		/// ### UV and Face layout
		/// Here's a test image that illustrates how this mesh's geometry is
		/// laid out.
		/// ![Procedural Cube Mesh]({{site.screen_url}}/ProcGeoCylinder.jpg)
		meshCylinder = Mesh.GenerateCylinder(1, 1, Vec3.Up);
		/// :End:
		/// :CodeSample: Mesh.GeneratePlane
		/// ### UV and Face layout
		/// Here's a test image that illustrates how this mesh's geometry is
		/// laid out.
		/// ![Procedural Cube Mesh]({{site.screen_url}}/ProcGeoPlane.jpg)
		meshPlane = Mesh.GeneratePlane(Vec2.One);
		/// :End:
	}

	public void Shutdown() { }

	public void Update()
	{
		Vec3 at   = new Vec3(0,-10,0);
		Vec3 from = new Vec3(1, 1, 1) * 0.7f;

		meshCube.Draw(material, Matrix.T(at));
		meshCube.Draw(materialWire, Matrix.T(at));
		Tests.Screenshot("ProcGeoCube.jpg", 600,600, at+from, at);

		at += Vec3.Right * 100;
		meshRoundedCube.Draw(material, Matrix.T(at));
		meshRoundedCube.Draw(materialWire, Matrix.T(at));
		Tests.Screenshot("ProcGeoRoundedCube.jpg", 600, 600, at + from, at);

		at += Vec3.Right * 100;
		meshSphere.Draw(material, Matrix.T(at));
		meshSphere.Draw(materialWire, Matrix.T(at));
		Tests.Screenshot("ProcGeoSphere.jpg", 600, 600, at + from, at);

		at += Vec3.Right * 100;
		meshCylinder.Draw(material, Matrix.T(at));
		meshCylinder.Draw(materialWire, Matrix.T(at));
		Tests.Screenshot("ProcGeoCylinder.jpg", 600, 600, at + from, at);

		at += Vec3.Right * 100;
		meshPlane.Draw(material, Matrix.T(at));
		meshPlane.Draw(materialWire, Matrix.T(at));
		Tests.Screenshot("ProcGeoPlane.jpg", 600, 600, at + from, at);
	}
}