using StereoKit;

// Loaded textures compress on the GPU unless they opt out. The exact format
// depends on what the device samples, so these check families and the
// relationships between hints rather than one fixed answer.
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
		Tex quality      = Tex.FromMemory(floor);
		Tex small        = Tex.FromMemory(floor, TexHint.Srgb | TexHint.Small);
		Tex uncompressed = Tex.FromMemory(floor, TexHint.Srgb | TexHint.Uncompressed);
		Tex data         = Tex.FromMemory(metal, TexHint.None);
		Tex sky          = Tex.FromCubemap("old_depot.hdr");
		_helmet          = Model.FromFile("DamagedHelmet.gltf");
		Assets.BlockForPriority(int.MaxValue);

		bool compressed = quality.Format != TexFormat.Rgba32;
		Log.Info($"Texture compression picked {quality.Format}, small {small.Format}, data {data.Format}, sky {sky.Format}");
		Tests.Test(() => !compressed || IsCompressedSrgb(quality.Format));
		Tests.Test(() => uncompressed.Format == TexFormat.Rgba32);
		Tests.Test(() => compressed ? IsCompressedData(data.Format) : data.Format == TexFormat.Rgba32Linear);
		Tests.Test(() => Tex.DefaultCompression == TexHint.Quality);
		Tests.Test(() => compressed == (small.Format != TexFormat.Rgba32));
		Tests.Test(() => !compressed || IsCompressedSrgb(small.Format));

		// Compression never drops the mip chain a 2D image asks for, and
		// never adds one to a skybox.
		Tests.Test(() => quality.Mips > 1 && quality.Mips == uncompressed.Mips);
		Tests.Test(() => sky.Mips == 1);
		Tests.Test(() => compressed ? sky.Format == TexFormat.Bc6hRgbuf || sky.Format == TexFormat.Astc8x8RgbaHdr || sky.Format == TexFormat.Rg11b10 : sky.Format == TexFormat.Rg11b10);

		// Opting a whole app out applies to anything loaded after.
		Tex.DefaultCompression = TexHint.Uncompressed;
		Tex optedOut = Tex.FromMemory(floor);
		Assets.BlockForPriority(int.MaxValue);
		Tests.Test(() => optedOut.Format == TexFormat.Rgba32);
		Tex.DefaultCompression = TexHint.Quality;

		// Each glTF slot keeps its own color space under compression.
		Material helmetMat = _helmet.Visuals[0].Material;
		Tex      diffuse   = helmetMat.GetTexture("diffuse");
		Tex      metalTex  = helmetMat.GetTexture("metal");
		Tests.Test(() => compressed ? IsCompressedSrgb(diffuse .Format) : diffuse .Format == TexFormat.Rgba32);
		Tests.Test(() => compressed ? IsCompressedData(metalTex.Format) : metalTex.Format == TexFormat.Rgba32Linear);
	}

	public void Shutdown() { }

	public void Step()
	{
		_helmet.Draw(Matrix.S(0.1f));
		Tests.Screenshot("Tests/TexCompression.jpg", 400, 400, new Vec3(0, 0, 0.2f), Vec3.Zero);
	}
}
