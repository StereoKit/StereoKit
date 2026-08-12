using StereoKit;

// The windowed backends' present blit uses sk/blit's uv_scale to read just the
// drawn corner of a surface, so a flipped or mis-scaled sample matters here.
class TestBlitUVScale : ITest
{
	public void Initialize()
	{
		// Top-left quadrant red, the other three blue. uv_scale 0.5 should
		// stretch the red quadrant over the whole destination.
		Color32 red  = new Color32(255, 0, 0, 255);
		Color32 blue = new Color32(0, 0, 255, 255);
		Color32[] src = {
			red,  red,  blue, blue,
			red,  red,  blue, blue,
			blue, blue, blue, blue,
			blue, blue, blue, blue, };

		Tex source = Tex.FromColors(src, 4, 4, false);
		source.SampleMode = TexSample.Point;

		Material mat = new Material(Shader.Find("default/shader_blit"));
		mat["source"]   = source;
		mat["uv_scale"] = new Vec2(0.5f, 0.5f);

		Tex dest = Tex.RenderTarget(4, 4, 1, TexFormat.Rgba32Linear);
		Renderer.Blit(dest, mat);

		Tests.Test(() => {
			Color32[] result = dest.GetColorData<Color32>();
			foreach (Color32 c in result)
				if (c.r < 128 || c.b > 128) return false;
			return true;
		});
	}

	public void Shutdown() { }
	public void Step()     { }
}
