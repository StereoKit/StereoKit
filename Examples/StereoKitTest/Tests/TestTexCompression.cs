using StereoKit;
using System;

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
		Tex quality      = Tex.FromMemory(floor, TexData.Srgb | TexData.CompressQuality);
		Tex small        = Tex.FromMemory(floor, TexData.Srgb | TexData.CompressSmall);
		Tex uncompressed = Tex.FromMemory(floor, TexData.Srgb | TexData.CompressNone);
		Tex data         = Tex.FromMemory(metal, TexData.None | TexData.CompressQuality);
		Tex sky          = Tex.FromCubemap("old_depot.hdr");
		_helmet          = Model.FromFile("DamagedHelmet.gltf", null, TexData.CompressQuality);
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
		Tex.DefaultCompression = TexData.CompressNone;
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
		procedural.SetColors(256, 256, pixels, TexFormat.Rgba32, TexData.CompressQuality);
		TexFormat proceduralFormat = procedural.Format;
		Assets.BlockForPriority(int.MaxValue);
		Tests.Test(() => compressed ? IsCompressedSrgb(proceduralFormat) : proceduralFormat == TexFormat.Rgba32);
		Tests.Test(() => procedural.AssetState == AssetState.Loaded && procedural.Format == proceduralFormat);

		Tex blocking = Tex.FromColors(pixels, 256, 256, TexData.Srgb | TexData.Blocking);
		Tests.Test(() => blocking.AssetState == AssetState.Loaded);

		// Full precision data is an HDR source
		float[] hdr = new float[256 * 256 * 4];
		for (int i = 0; i < hdr.Length; i++) hdr[i] = 2.0f;
		Tex floats = Tex.FromColors(hdr, 256, 256, TexFormat.Rgba128, TexData.None | TexData.Blocking);
		Tests.Test(() => compressed ? floats.Format == TexFormat.Bc6hRgbuf || floats.Format == TexFormat.Astc8x8RgbaHdr || floats.Format == TexFormat.Rgba128 : floats.Format == TexFormat.Rgba128);

		Tex fromMemory = new Tex();
		fromMemory.SetMemory(floor, TexData.Srgb | TexData.Blocking);
		Tests.Test(() => fromMemory.AssetState == AssetState.Loaded);

		// Copy without flags keeps its old uncompressed behavior, with flags it
		// compresses, and `into` lands the copy in an existing texture
		Tex target = Tex.RenderTarget(256, 256);
		Tex plain  = target.Copy();
		Tex baked  = target.Copy(TexType.Image, TexFormat.None, TexData.Srgb | TexData.CompressQuality);
		Tex dest   = new Tex();
		target.Copy(TexType.Image, TexFormat.None, TexData.Srgb | TexData.CompressQuality, dest);
		Tests.Test(() => plain.Format == TexFormat.Rgba32);
		Tests.Test(() => compressed ? IsCompressedSrgb(baked.Format) : baked.Format == TexFormat.Rgba32);
		Tests.Test(() => dest.Format == baked.Format && dest.AssetState == AssetState.Loaded);

		// Data lands on a compressed texture
		Tex reused = Tex.FromMemory(floor, TexData.Srgb | TexData.Blocking);
		reused.SetColors(256, 256, pixels, TexFormat.Rgba32, TexData.Blocking | TexData.CompressNone);
		Tests.Test(() => reused.Format == TexFormat.Rgba32 && reused.InputFormat == TexFormat.Rgba32);

		// Dynamic textures only take data in their own format
		Tex dynamic = new Tex(TexType.ImageNomips | TexType.Dynamic, TexFormat.Rgba32);
		dynamic.SetColors(256, 256, pixels, TexFormat.Rgba32, TexData.Blocking | TexData.CompressNone);
		dynamic.SetColors(256, 256, new Color[256 * 256], TexFormat.Rgba128, TexData.Blocking | TexData.CompressNone);
		Tests.Test(() => dynamic.Format == TexFormat.Rgba32);

		// Full precision colors keep values over 1, and sRGB ones convert to
		// linear rather than clamping to 8 bits
		Color[] bright = new Color[16 * 16];
		for (int i = 0; i < bright.Length; i++) bright[i] = new Color(2, 0.5f, 0, 1);
		Tex fullLinear = Tex.FromColors(bright, 16, 16, TexData.None | TexData.CompressNone | TexData.Blocking);
		Tex fullSrgb   = Tex.FromColors(bright, 16, 16, TexData.Srgb | TexData.CompressNone | TexData.Blocking);
		Tests.Test(() => fullLinear.Format == TexFormat.Rgba128 && fullLinear.GetColorData<Color>()[0].r == 2);
		Tests.Test(() => MathF.Abs(fullSrgb.GetColorData<Color>()[0].g - 0.214f) < 0.002f);

		// Constructors match their From* counterparts
		byte[] raw = new byte[64 * 64 * 4];
		Tex ctorColors = new Tex(pixels, 256, 256, TexData.Srgb | TexData.Blocking);
		Tex ctorRaw    = new Tex(raw, 64, 64, TexFormat.Rgba32Linear, TexData.None | TexData.Blocking);
		Tex fromRaw    = Tex.FromColors(raw, 64, 64, TexFormat.Rgba32Linear, TexData.None | TexData.Blocking);
		Tests.Test(() => ctorColors.Format == blocking.Format);
		Tests.Test(() => ctorRaw.Format == fromRaw.Format && ctorRaw.InputFormat == TexFormat.Rgba32Linear);
		Tests.Test(() => new Tex("floor.png").Format == Tex.FromFile("floor.png").Format);

		// The Model overload for older positional calls still loads
		Model positional = Model.FromFile("DamagedHelmet.gltf", null, 10);
		Assets.BlockForPriority(int.MaxValue);
		Tests.Test(() => positional.AssetState == AssetState.Loaded);
	}

	public void Shutdown() { }

	public void Step()
	{
		_helmet.Draw(Matrix.S(0.1f));
		Tests.Screenshot("Tests/TexCompression.jpg", 400, 400, new Vec3(0, 0, 0.2f), Vec3.Zero);
	}
}
