// Regression test for reflection generation from a cubemap uploaded on the
// main thread: the convolution task once raced the upload's command batch
// and sampled an unwritten texture, producing an all-black reflection.
using StereoKit;
using System;

class TestCubemapReflection : ITest
{
	Tex reflection;

	Tex                oldSkybox;
	Tex                oldReflection;
	SphericalHarmonics oldAmbient;

	public void Initialize()
	{
		oldSkybox     = Renderer.SkyboxTex;
		oldReflection = Lighting.Reflection;
		oldAmbient    = Lighting.Ambient;

		SphericalHarmonics lighting = SphericalHarmonics.FromLights(new SHLight[] {
			new SHLight { directionTo = V.XYZ(0, 1, 0), color = Color.White }
		});
		Lighting.SetEnvironment(Tex.GenCubemap(lighting), out reflection);
		Tests.RunForFrames(1);
	}

	public void Step()
	{
		if (reflection == null) { Tests.Fail("TestCubemapReflection: SetEnvironment returned no reflection"); return; }

		// Generation is async on the asset threads, no frame stepping needed.
		Assets.BlockUntil(reflection, AssetState.Loaded);
		if (reflection.AssetState != AssetState.Loaded)
		{
			Tests.Fail($"TestCubemapReflection: reflection failed to generate, state {reflection.AssetState}");
			return;
		}

		if (reflection.Mips < 2)
			Tests.Fail($"TestCubemapReflection: reflection has no mip chain ({reflection.Mips} mips)");

		// The GGX chain preserves energy, so every mip's average should sit
		// near the environment's ~0.96 average. Black mips or NaN/garbage
		// blowups land far outside this band.
		for (int m = 0; m < reflection.Mips; m++)
		{
			float avg = MipAvgLuminance(reflection, m);
			if (avg < 0.3f || avg > 3.0f)
				Tests.Fail($"TestCubemapReflection: mip {m} average luminance {avg:0.0000} is out of band");
		}
	}

	public void Shutdown()
	{
		Renderer.SkyboxTex  = oldSkybox;
		Lighting.Reflection = oldReflection;
		Lighting.Ambient    = oldAmbient;
	}

	static float MipAvgLuminance(Tex t, int mip)
	{
		uint[] raw = t.GetColorData<uint>(mip);
		double sum = 0;
		for (int i = 0; i < raw.Length; i++)
		{
			Vec3 c = DecodeR11G11B10(raw[i]);
			sum += c.x * 0.2126f + c.y * 0.7152f + c.z * 0.0722f;
		}
		return (float)(sum / raw.Length);
	}

	static float DecodeUF(uint bits, int mantissaBits)
	{
		uint exp = bits >> mantissaBits;
		uint man = bits & ((1u << mantissaBits) - 1);
		if (exp == 0) return man * MathF.Pow(2, -14 - mantissaBits);
		return (1 + man / (float)(1 << mantissaBits)) * MathF.Pow(2, (int)exp - 15);
	}
	static Vec3 DecodeR11G11B10(uint v) => new Vec3(
		DecodeUF( v        & 0x7FF, 6),
		DecodeUF((v >> 11) & 0x7FF, 6),
		DecodeUF((v >> 22) & 0x3FF, 5));
}
