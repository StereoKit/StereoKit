// Tests for the Lighting system: dominant main light derivation, the main
// light split, last write wins assignment rules, and SetEnvironment's async
// apply respecting assignments made after the call. The split checks are a
// regression guard for over-subtraction, which once collapsed the ambient
// to black when separating the main light out of an image environment.
using StereoKit;
using System;

class TestLighting : ITest
{
	Tex                oldSkybox;
	Tex                oldReflection;
	SphericalHarmonics oldAmbient;
	SHLight            oldMainLight;
	LightingMode       oldMode;

	static readonly Vec3  lightDir   = V.XYZ(1, 2, 0.5f).Normalized;
	static readonly Color lightColor = new Color(1.0f, 0.8f, 0.6f);
	// A recognizable uniform ambient for spotting overwrites.
	static readonly Vec3  marker     = V.XYZ(0.1f, 0.2f, 0.3f);

	Tex reflection;
	int phase;

	static float Lum(Color c) => c.r*0.2126f + c.g*0.7152f + c.b*0.0722f;
	static float Lum(Vec3  c) => c.x*0.2126f + c.y*0.7152f + c.z*0.0722f;

	public void Initialize()
	{
		oldSkybox     = Renderer.SkyboxTex;
		oldReflection = Lighting.Reflection;
		oldAmbient    = Lighting.Ambient;
		oldMainLight  = Lighting.MainLight;
		oldMode       = Lighting.Mode;
		Lighting.Mode = LightingMode.Ambient;

		// The manual source is always available, and in a headless run the
		// world source is not: requesting it should settle quietly.
		if (!Lighting.SourceAvailable(LightingSource.Manual))
			Tests.Fail("The manual lighting source should always be available");
		if (!Lighting.SourceAvailable(LightingSource.World))
		{
			Lighting.RequestSource(LightingSource.World);
			if (Lighting.Source != LightingSource.Manual || Lighting.SourcePending)
				Tests.Fail("An unavailable world source request should settle in manual");
		}

		// An empty SH has no light data, and defaults to a light from above.
		if (Vec3.Dot(new SphericalHarmonics().DominantLightDirectionTo, Vec3.Up) < 0.999f)
			Tests.Fail("An empty SH should default its dominant direction to up");

		// A single authored light should extract back out of its SH nearly
		// exactly: same direction, similar color balance.
		SphericalHarmonics single = SphericalHarmonics.FromLights(new SHLight[] {
			new SHLight { directionTo = lightDir, color = lightColor } });
		if (Vec3.Dot(single.DominantLightDirectionTo, lightDir) < 0.999f)
			Tests.Fail("DominantLightDirectionTo doesn't match the authored light");
		SHLight extracted = single.DominantLight;
		if (Vec3.Dot(extracted.directionTo, lightDir) < 0.999f || Lum(extracted.color) < 0.1f)
			Tests.Fail("DominantLight doesn't match the authored light");
		if (!(extracted.color.r > extracted.color.g && extracted.color.g > extracted.color.b))
			Tests.Fail("DominantLight color balance doesn't match the authored light");

		// DominantLight and SubtractLight together split a light out of an
		// SH, removing energy facing it.
		SphericalHarmonics splitSH = single;
		splitSH.SubtractLight(extracted);
		if (Lum(splitSH.Sample(lightDir)) >= Lum(single.Sample(lightDir)))
			Tests.Fail("SubtractLight didn't remove the light's energy");

		// Assignments touch only their own value.
		SHLight before = Lighting.MainLight;
		Lighting.Ambient = single;
		SHLight after = Lighting.MainLight;
		if ((before.directionTo - after.directionTo).Length > 0.001f ||
		    MathF.Abs(Lum(before.color) - Lum(after.color)) > 0.001f)
			Tests.Fail("Assigning Ambient changed MainLight");

		// An assigned MainLight reads back with a normalized direction.
		Lighting.MainLight = new SHLight { directionTo = Vec3.Up * 3, color = new Color(1, 0, 0) };
		Vec3 dir = Lighting.MainLight.directionTo;
		if (MathF.Abs(dir.Length - 1) > 0.001f || Vec3.Dot(dir, Vec3.Up) < 0.999f)
			Tests.Fail("Assigned MainLight direction wasn't normalized");

		// A Mode change re-delivers from the scene's most recent total
		// lighting, which an Ambient assignment provides, not from a stale
		// environment.
		Lighting.Ambient = single;
		Lighting.Mode    = LightingMode.MainLight;
		if (Vec3.Dot(Lighting.MainLight.directionTo, lightDir) < 0.999f)
			Tests.Fail("Mode re-delivery used stale lighting instead of the assigned Ambient");
		if (Lum(Lighting.Ambient.Sample(lightDir)) >= Lum(single.Sample(lightDir)))
			Tests.Fail("Mode re-delivery didn't split the assigned Ambient");
		Lighting.Mode = LightingMode.Ambient;

		// An environment with a light over a uniform base, for split
		// testing. The Ambient assigned after SetEnvironment must survive
		// the async apply, while the MainLight assigned before it should be
		// replaced by the derived light.
		Vec3[] coefficients = single.ToArray();
		coefficients[0] += V.XYZ(0.4f, 0.4f, 0.4f);
		Lighting.SetEnvironment(Tex.GenCubemap(new SphericalHarmonics(coefficients)), out reflection);
		Lighting.Ambient = new SphericalHarmonics(new Vec3[] {
			marker,    Vec3.Zero, Vec3.Zero, Vec3.Zero, Vec3.Zero,
			Vec3.Zero, Vec3.Zero, Vec3.Zero, Vec3.Zero });
		Tests.RunForFrames(8);
	}

	public void Step()
	{
		if (phase == 0)
		{
			Assets.BlockUntil(reflection, AssetState.Loaded);
			if (reflection.AssetState != AssetState.Loaded)
				Tests.Fail("Environment reflection failed to generate");
			// The derived lighting applies on the frame after the load.
			phase = 1;
			return;
		}
		if (phase != 1) return;
		phase = 2;

		// The post-SetEnvironment assignment wins over the async apply for
		// its own value only, MainLight still receives the derived light.
		if ((Lighting.Ambient.coefficient1 - marker).Length > 0.001f)
			Tests.Fail("SetEnvironment's async apply overwrote a later Ambient assignment");
		if (Vec3.Dot(Lighting.MainLight.directionTo, lightDir) < 0.98f)
			Tests.Fail("Environment derived MainLight direction is off");

		// A mode change re-delivers the environment lighting, replacing
		// earlier assignments: last write wins.
		Lighting.Mode = LightingMode.MainLight;
		Lighting.Mode = LightingMode.Ambient;
		SphericalHarmonics total = Lighting.Ambient;
		if ((total.coefficient1 - marker).Length < 0.001f)
			Tests.Fail("A Mode change didn't re-deliver the environment lighting");

		// Splitting the main light out must not collapse the ambient! The
		// uniform portion of the environment stays behind.
		Lighting.Mode = LightingMode.MainLight;
		SphericalHarmonics split = Lighting.Ambient;
		if (Lum(split.coefficient1) < 0.25f * Lum(total.coefficient1))
			Tests.Fail("Main light split collapsed the ambient to black");
		float away  = Lum(split.Sample(-lightDir));
		float awayT = Lum(total.Sample(-lightDir));
		if (away < awayT * 0.5f || away > awayT * 1.5f)
			Tests.Fail("Main light split changed lighting opposite the light");
		if (Lum(split.Sample(lightDir)) >= Lum(total.Sample(lightDir)))
			Tests.Fail("Main light split didn't remove the light's energy");

		// Toggling back restores the unsplit ambient.
		Lighting.Mode = LightingMode.Ambient;
		if (Lum(Lighting.Ambient.Sample(lightDir)) < Lum(total.Sample(lightDir)) * 0.99f)
			Tests.Fail("Returning to ambient mode didn't restore the total lighting");
	}

	public void Shutdown()
	{
		Lighting.Mode       = oldMode;
		Renderer.SkyboxTex  = oldSkybox;
		Lighting.Reflection = oldReflection;
		Lighting.Ambient    = oldAmbient;
		Lighting.MainLight  = oldMainLight;
	}
}
