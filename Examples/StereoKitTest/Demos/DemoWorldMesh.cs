using StereoKit;
using StereoKit.Framework;

class DemoWorldMesh : ITest
{
	OcclusionMesh occluder;
	Pose windowPose        = new Pose(Vec3.Forward, Quat.LookDir(-Vec3.Forward));
	bool settingsWireframe = true;
	int  settingsColor     = 0;

	public void Initialize()
	{
		// StereoKit will support world understanding features as core
		// functionality when they become available in OpenXR! In the 
		// meantime, here's some tools for doing this through the Mirage APIs

		// If the system is capable of acquiring an occlusion mesh, do that!
		// Otherwise, we can also provide any mesh or model for testing
		// purposes!
		occluder = SK.AddStepper(OcclusionMesh.Capable 
			? new OcclusionMesh()
			: OcclusionMesh.FromSimulation(Mesh.GeneratePlane(Vec2.One*10, 10)));

		// Make it visible and obvious for the demo, by default the material
		// will work like a solid, invisible occluder.
		occluder.Material[MatParamName.ColorTint] = new Color(1,0,0);
		occluder.Material.Wireframe = true;
	}

	public void Shutdown()
		=> SK.RemoveStepper(occluder);

	public void Update() {
		UI.WindowBegin("Settings", ref windowPose, Vec2.Zero);
		if (UI.Toggle("Wireframe", ref settingsWireframe))
			occluder.Material.Wireframe = settingsWireframe;
		if (UI.Radio("Red",   settingsColor == 0)) { settingsColor = 0; occluder.Material.SetColor("color", Color.HSV(0,1,1)); } UI.SameLine();
		if (UI.Radio("White", settingsColor == 1)) { settingsColor = 1; occluder.Material.SetColor("color", Color.White);      } UI.SameLine();
		if (UI.Radio("Black", settingsColor == 2)) { settingsColor = 2; occluder.Material.SetColor("color", Color.BlackTransparent); }
		UI.WindowEnd();
	}
}