using StereoKit;

internal class DocTextures : ITest
{
	public void Initialize()
	{
		/// :CodeSample: Tex.GetColorData
		/// ### Get data from a Tex
		/// Reading texture data from a Tex can be a slow operation, since
		/// texture memory lives on the GPU, and isn't generally optimized for
		/// readability. But, sometimes you still have to do it! Just remember
		/// to avoid doing it too often or casually, and be cautious about how
		/// you treat the large amounts of memory involved.

		// Reading colors can be as simple as this! Remember to select a color
		// format that matches the data stored in the texture, as StereoKit
		// will not convert the data for you. Most images from file are 32 bit
		// RGBA images!
		Tex texture = Tex.FromFile("floor.png");
		Color32[] colors = texture.GetColorData<Color32>();

		// For a more complex texture, such as this generated texture with 32
		// bits per channel, we can load the data into a float array, with 4
		// floats per-pixel! A `Color` would probably be fine here too.
		Tex texture2 = Tex.GenColor(Color.White, 16, 16, TexType.Image, TexFormat.Rgba128);
		float[] colors2 = texture2.GetColorData<float>(0, 4);
		/// :End:
	}

	public void Shutdown()
	{
	}

	public void Step()
	{
	}
}