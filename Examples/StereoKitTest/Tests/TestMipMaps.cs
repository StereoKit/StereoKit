using StereoKit;

class TestMipMaps : ITest
{

	public void Initialize()
	{
		Color32[] colors  = new Color32[8 * 8];

		Tex tex1 = new Tex(TexType.Image, TexFormat.Rgba32);
		tex1.SetColors(8, 8, colors);

		Tex tex2 = new Tex(TexType.Image, TexFormat.Rgba32Linear);
		tex2.SetColors(6, 8, colors);


		float[] colors2 = new float[8 * 8];

		Tex tex5 = new Tex(TexType.Image, TexFormat.R32);
		tex5.SetColors(8, 8, colors2);

		Tex tex6= new Tex(TexType.Image, TexFormat.R32);
		tex6.SetColors(6, 8, colors2);


		Color[] colors3 = new Color[8 * 8];

		Tex tex7 = new Tex(TexType.Image, TexFormat.Rgba128);
		tex7.SetColors(8, 8, colors3);

		Tex tex8 = new Tex(TexType.Image, TexFormat.Rgba128);
		tex8.SetColors(6, 8, colors3);


		byte[] colors4 = new byte[8 * 8];

		Tex tex9 = new Tex(TexType.Image, TexFormat.R8);
		tex9.SetColors(8, 8, colors4);

		Tex tex10 = new Tex(TexType.Image, TexFormat.R8);
		tex10.SetColors(6, 8, colors4);


		ushort[] colors5 = new ushort[8 * 8];

		Tex tex11 = new Tex(TexType.Image, TexFormat.R16);
		tex11.SetColors(8, 8, colors5);

		Tex tex12 = new Tex(TexType.Image, TexFormat.R16);
		tex12.SetColors(6, 8, colors5);


		ushort[] colors6 = new ushort[8 * 8 * 4];

		Tex tex13 = new Tex(TexType.Image, TexFormat.Rgba64);
		tex13.SetColors(8, 8, colors6);

		Tex tex14 = new Tex(TexType.Image, TexFormat.Rgba64);
		tex14.SetColors(6, 8, colors6);


		byte[] colors7 = new byte[8 * 8 * 4];

		Tex tex15 = new Tex(TexType.Image, TexFormat.Bgra32Linear);
		tex15.SetColors(8, 8, colors7);

		Tex tex16 = new Tex(TexType.Image, TexFormat.Bgra32);
		tex16.SetColors(6, 8, colors7);
	}
	public void Shutdown() { }
	public void Update() { }
}