using StereoKit;

class TestGradientCubemap : ITest
{
	Tex                 cubemap;
	SphericalHarmonics  lighting;
	Tex                 oldSkyTex;
	SphericalHarmonics  oldSkyLight;

	public void Initialize()
	{
		oldSkyTex   = Renderer.SkyTex;
		oldSkyLight = Renderer.SkyLight;

		Gradient gradient = new Gradient(
			new GradientKey(new Color(0.1f, 0.1f, 0.2f), 0.0f),
			new GradientKey(new Color(0.4f, 0.6f, 0.9f), 0.5f),
			new GradientKey(new Color(1.0f, 0.9f, 0.7f), 1.0f));

		cubemap = Tex.GenCubemap(gradient, out lighting, Vec3.Up, 64);

		Renderer.SkyTex   = cubemap;
		Renderer.SkyLight = lighting;
	}

	public void Shutdown()
	{
		Renderer.SkyTex   = oldSkyTex;
		Renderer.SkyLight = oldSkyLight;
	}

	public void Step()
	{
		Tests.Screenshot("Tests/GradientCubemap.jpg", 400, 400, V.XYZ(0, 0, 0), V.XYZ(0, 0, -0.5f));

		Mesh.Sphere.Draw(Material.Default, Matrix.TS(0, 0, -0.5f, 0.2f));
	}
}
