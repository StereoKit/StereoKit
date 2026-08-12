using StereoKit;

// A postfx covers what the geometry drew, so a fullscreen postfx shader's uv
// has to span 0-1 across the pass viewport, not the whole render target.

// Needs an sk_renderer whose resolve and postfx subpasses honor that viewport.
class TestPostFXViewport : ITest
{
	public void Initialize()
	{
		const int size = 64;
		const int half = size / 2;

		Tex        dest = Tex.RenderTarget(size, size, 1, TexFormat.Rgba32Linear);
		Material   fx   = new Material("postfx_uv_probe.hlsl");
		RenderList list = new RenderList();
		list.Add(Mesh.Cube, Material.Unlit, Matrix.S(0.5f), Color.White);

		// Draw into the top-left quadrant only. The probe shader writes its
		// own uv into red/green.
		list.DrawNow(dest, Matrix.Identity, Matrix.Perspective(90, 1, 0.01f, 10),
			new RenderSettings {
				clear       = RenderClear.All,
				clearColor  = Color.BlackTransparent,
				viewport    = new Rect(0, 0, 0.5f, 0.5f),
				postProcess = new Material[] { fx } });

		Color32[] px  = dest.GetColorData<Color32>();
		int       row = size / 4; // a row inside the viewport

		Tests.Test(() => {
			// uv must reach ~1 by the viewport's last column, not the target's.
			float atViewportEdge = px[row * size + half - 1].r / 255.0f;
			float atTargetEdge   = px[row * size + size - 1].r / 255.0f;
			return atViewportEdge > 0.9f && atTargetEdge < 0.1f;
		});
	}

	public void Shutdown() { }
	public void Step()     { }
}
