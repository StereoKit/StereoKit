using StereoKit;

// A named global fills only the texture slots a Material leaves at default.
class TestGlobalTexture : ITest
{
	Tex      globalTex   = Tex.GenColor(new Color(1, 0, 0, 1), 2, 2);
	Tex      localTex    = Tex.GenColor(new Color(0, 0, 1, 1), 2, 2);
	Material matDefault  = Material.Unlit.Copy();
	Material matExplicit = Material.Unlit.Copy();
	Material matCleared  = Material.Unlit.Copy();
	int      frame;

	public void Initialize()
	{
		matExplicit.SetTexture("diffuse", localTex);
		matCleared .SetTexture("diffuse", localTex);
		matCleared .SetTexture("diffuse", null);

		// Draws see a global from the frame after it's set
		Renderer.SetGlobalTexture("diffuse", globalTex);
		Tests.RunForFrames(2);
	}

	public void Step()
	{
		if (frame++ != 1) return;
		Tests.Test(GlobalRoundTrip);
		Tests.Test(GlobalFillsDefault);
		Tests.Test(ExplicitWins);
		Tests.Test(ClearedShowsGlobal);
		Tests.Test(DefaultReadsNull);
	}

	public void Shutdown() => Renderer.SetGlobalTexture("diffuse", null);

	bool GlobalRoundTrip   () => Renderer.GetGlobalTexture("diffuse")?.Id == globalTex.Id;
	bool GlobalFillsDefault() => DrawCenter(matDefault ).r > 200;
	bool ExplicitWins      () => DrawCenter(matExplicit).b > 200;
	bool ClearedShowsGlobal() => DrawCenter(matCleared ).r > 200;
	bool DefaultReadsNull  () => matDefault.GetTexture("diffuse") == null && matCleared.GetTexture("diffuse") == null && matExplicit.GetTexture("diffuse")?.Id == localTex.Id;

	static Color32 DrawCenter(Material material)
	{
		const int  size = 4;
		Tex        dest = Tex.RenderTarget(size, size, 1, TexFormat.Rgba32Linear);
		RenderList list = new RenderList();
		list.Add(Mesh.Cube, material, Matrix.T(0, 0, -1), Color.White);
		list.DrawNow(dest, Matrix.Identity, Matrix.Perspective(90, 1, 0.01f, 10),
			new RenderSettings { clear = RenderClear.All, clearColor = Color.BlackTransparent });
		return dest.GetColorData<Color32>()[size/2 * size + size/2];
	}
}
