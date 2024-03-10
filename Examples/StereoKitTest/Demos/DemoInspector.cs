using StereoKit;
using StereoKit.Framework;

class DemoInspector : ITest
{
	[Inspector.Show]
	string name = "Inspector";
	[Inspector.Separator]
	[Inspector.Slider(0,1)]
	public float brightness = 1.0f;
	[Inspector.Slider(0,10)]
	public int   count      = 5;
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