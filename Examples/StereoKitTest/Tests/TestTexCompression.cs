using StereoKit;

// Loaded textures compress on the GPU unless they opt out. The exact format
// depends on what the device samples, so these check families and the
// relationships between flags rather than one fixed answer.
class TestTexCompression : ITest
{
	Model _helmet;

	static bool IsCompressedSrgb(TexFormat f) => f == TexFormat.Bc7RgbaSrgb || f == TexFormat.Astc4x4RgbaSrgb || f == TexFormat.Bc1RgbSrgb || f == TexFormat.Bc1RgbaSrgb || f == TexFormat.Astc6x6RgbaSrgb;
	static bool IsCompressedData(TexFormat f) => f == TexFormat.Bc7Rgba     || f == TexFormat.Astc4x4Rgba     || f == TexFormat.Bc1Rgb     || f == TexFormat.Bc1Rgba     || f == TexFormat.Astc6x6Rgba;

	public void Initialize()
	{
		// From memory, so no other test's copy of these files is shared.
		byte[] floor = Platform.ReadFileBytes("floor.png");
		byte[] metal = Platform.ReadFileBytes("metal_plate_metal.jpg");
		Tex quality      = Tex.FromMemory(floor, TexData.Srgb | TexData.Quality);
		Tex small        = Tex.FromMemory(floor, TexData.Srgb | TexData.Small);
		Tex uncompressed = Tex.FromMemory(floor, TexData.Srgb | TexData.Uncompressed);
		Tex data         = Tex.FromMemory(metal, TexData.None | TexData.Quality);
		Tex sky          = Tex.FromCubemap("old_depot.hdr");
		_helmet          = Model.FromFile("DamagedHelmet.gltf", null, TexData.Quality);
		Assets.BlockForPriority(int.MaxValue);

		bool compressed = quality.Format != TexFormat.Rgba32;
		Log.Info($"Texture compression picked {quality.Format}, small {small.Format}, data {data.Format}, sky {sky.Format}");
		Tests.Test(() => !compressed || IsCompressedSrgb(quality.Format));
		Tests.Test(() => uncompressed.Format == TexFormat.Rgba32);
		Tests.Test(() => compressed ? IsCompressedData(data.Format) : data.Format == TexFormat.Rgba32Linear);
		Tests.Test(() => compressed == (small.Format != TexFormat.Rgba32));
		Tests.Test(() => !compressed || IsCompressedSrgb(small.Format));

		// Compression never drops the mip chain a 2D image asks for, and
		// never adds one to a skybox.
		Tests.Test(() => quality.Mips > 1 && quality.Mips == uncompressed.Mips);
		Tests.Test(() => sky.Mips == 1);
		Tests.Test(() => compressed ? sky.Format == TexFormat.Bc6hRgbuf || sky.Format == TexFormat.Astc8x8RgbaHdr || sky.Format == TexFormat.Rg11b10 : sky.Format == TexFormat.Rg11b10);

		// Opting a whole app out applies to anything loaded after.
		TexData prevDefault = Tex.DefaultCompression;
		Tex.DefaultCompression = TexData.Uncompressed;
		Tex optedOut = Tex.FromMemory(floor);
		Assets.BlockForPriority(int.MaxValue);
		Tests.Test(() => optedOut.Format == TexFormat.Rgba32);
		Tex.DefaultCompression = prevDefault;

		// Each glTF slot keeps its own color space under compression.
		Material helmetMat = _helmet.Visuals[0].Material;
		Tex      diffuse   = helmetMat.GetTexture("diffuse");
		Tex      metalTex  = helmetMat.GetTexture("metal");
		Tests.Test(() => compressed ? IsCompressedSrgb(diffuse .Format) : diffuse .Format == TexFormat.Rgba32);
		Tests.Test(() => compressed ? IsCompressedData(metalTex.Format) : metalTex.Format == TexFormat.Rgba32Linear);

		// Calls without TexData keep their old immediate, uncompressed behavior
		Color32[] pixels = new Color32[256 * 256];
		for (int i = 0; i < pixels.Length; i++) pixels[i] = new Color32(200, 40, 90, 255);
		Tex legacy = Tex.FromColors(pixels, 256, 256);
		Tests.Test(() => legacy.Format == TexFormat.Rgba32 && legacy.AssetState == AssetState.Loaded);

		// With TexData, pixels compress like files, and Format is final on return
		Tex procedural = new Tex(TexType.Image, TexFormat.Rgba32);
		procedural.SetColors(256, 256, pixels, TexData.Srgb | TexData.Quality);
		TexFormat proceduralFormat = procedural.Format;
		Assets.BlockForPriority(int.MaxValue);
		Tests.Test(() => compressed ? IsCompressedSrgb(proceduralFormat) : proceduralFormat == TexFormat.Rgba32);
		Tests.Test(() => procedural.AssetState == AssetState.Loaded && procedural.Format == proceduralFormat);

		Tex blocking = Tex.FromColors(pixels, 256, 256, TexData.Srgb | TexData.Blocking);
		Tests.Test(() => blocking.AssetState == AssetState.Loaded);

		// Full precision data is an HDR source
		float[] hdr = new float[256 * 256 * 4];
		for (int i = 0; i < hdr.Length; i++) hdr[i] = 2.0f;
		Tex floats = Tex.FromData(hdr, TexFormat.Rgba128, 256, 256, TexData.None | TexData.Blocking);
		Tests.Test(() => compressed ? floats.Format == TexFormat.Bc6hRgbuf || floats.Format == TexFormat.Astc8x8RgbaHdr || floats.Format == TexFormat.Rgba128 : floats.Format == TexFormat.Rgba128);

		Tex fromMemory = new Tex();
		fromMemory.SetMemory(floor, TexData.Srgb | TexData.Blocking);
		Tests.Test(() => fromMemory.AssetState == AssetState.Loaded);
	}

	public void Shutdown() { }

	public void Step()
	{
		_helmet.Draw(Matrix.S(0.1f));
		Tests.Screenshot("Tests/TexCompression.jpg", 400, 400, new Vec3(0, 0, 0.2f), Vec3.Zero);
	}
}
