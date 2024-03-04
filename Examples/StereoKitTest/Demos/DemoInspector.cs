using StereoKit;
using StereoKit.Framework;

class DemoInspector : ITest
{
	public string name = "Inspector";
	[Inspector.Separator]
	[Inspector.Slider(0,1)]
	public float brightness = 1.0f;
	public bool  visible    = true;
	[Inspector.Pose(0.1f)]
	public Pose  pose       = new Pose(0,0,-0.5f);

	public void Initialize()
	{
	}
	public void Step()
	{
		if (visible) Mesh.Sphere.Draw(Material.Default, pose.ToMatrix(0.1f), Color.White * brightness);
	}

	public void Shutdown() { }
}