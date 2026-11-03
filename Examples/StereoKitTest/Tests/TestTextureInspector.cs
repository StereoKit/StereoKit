// Shows the texture inspector on a compressed 2D texture and on a cubemap,
// whose preview binds through sk_renderer's 2D array view.
using StereoKit;
using StereoKit.Framework;

class TestTextureInspector : ITest
{
	Tex plate;
	Tex sky;

	public void Initialize()
	{
		plate = Tex.FromFile   ("metal_plate_diff.ktx2");
		sky   = Tex.FromCubemap("old_depot.ktx2");
		Assets.BlockForPriority(int.MaxValue);

		Tests.Test(() => plate.ArrayCount == 1 && (plate.Type & TexType.Cubemap) == 0);
		Tests.Test(() => sky  .ArrayCount == 6 && (sky  .Type & TexType.Cubemap) != 0);

		TextureInspector.Show(plate);
		Tests.RunForFrames(8);
	}

	public void Shutdown() => TextureInspector.Hide();

	public void Step()
	{
		if (Tests.SceneFrame == 3) TextureInspector.Show(sky);
		// Steppers draw before a test's Shutdown, so hide a frame early to keep
		// the inspector out of the next test's first screenshot.
		if (Tests.SceneFrame == 6) TextureInspector.Hide();

		Tests.Screenshot("Tests/TextureInspector2D.jpg",   2, 600, 500, 65, Vec3.Zero, V.XYZ(0, -0.42f, -1));
		Tests.Screenshot("Tests/TextureInspectorCube.jpg", 5, 600, 500, 65, Vec3.Zero, V.XYZ(0, -0.42f, -1));
	}
}
