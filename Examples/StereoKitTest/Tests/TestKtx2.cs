// KTX2 coverage from both transcoders: a UASTC cubemap with a single mip,
// and an ETC1S image whose mip chain takes the sliced transcode.
using StereoKit;

class TestKtx2 : ITest
{
	Tex                oldSkybox;
	Tex                oldReflection;
	SphericalHarmonics oldAmbient;
	SHLight            oldMainLight;

	Material _material;

	public void Initialize()
	{
		oldSkybox     = Renderer.SkyboxTex;
		oldReflection = Lighting.Reflection;
		oldAmbient    = Lighting.Ambient;
		oldMainLight  = Lighting.MainLight;

		Tex sky   = Tex.FromCubemap("old_depot.ktx2");
		Tex plate = Tex.FromFile("metal_plate_diff.ktx2");
		Lighting.SetEnvironment(sky, out Tex reflection);
		Assets.BlockForPriority(int.MaxValue);
		Assets.BlockUntil(reflection, AssetState.Loaded);

		Tests.Test(() => sky.AssetState == AssetState.Loaded && sky.Width == 64 && sky.Mips == 1);
		Tests.Test(() => plate.AssetState == AssetState.Loaded && plate.Width == 256 && plate.Mips == 9);
		// Reflections refuse anything but a cubemap, so this also checks the
		// file's six faces became one.
		Tests.Test(() => reflection.AssetState == AssetState.Loaded && reflection.Mips > 1);

		_material = Material.PBR.Copy();
		_material[MatParamName.DiffuseTex] = plate;
		_material[MatParamName.MetalTex  ] = Tex.FromFile("metal_plate_metal.jpg", TexData.None);
	}

	public void Shutdown()
	{
		Renderer.SkyboxTex  = oldSkybox;
		Lighting.Reflection = oldReflection;
		Lighting.Ambient    = oldAmbient;
		Lighting.MainLight  = oldMainLight;
	}

	public void Step()
	{
		Mesh.Sphere.Draw(_material, Matrix.S(0.1f));
		Tests.Screenshot("Tests/Ktx2.jpg", 400, 400, 90, new Vec3(0, 0, 0.12f), Vec3.Zero);
	}
}
