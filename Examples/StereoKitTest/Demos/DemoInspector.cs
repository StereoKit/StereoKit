using StereoKit;
using StereoKit.Framework;
using System;

class DemoInspector : ITest
{
	[Inspector.Show]
	string visibleName   = "Inspector";
	string invisibleName = "Inspector";
	[Inspector.Separator]
	[Inspector.Slider(0,1)]
	public float brightness = 1.0f;
	[Inspector.Slider(0,10)]
	public int   count      = 5;
	public bool  visible    = true;
	[Inspector.Pose(0.1f)]
	public Pose  pose       = new Pose(0,0,-0.5f);

	public int GetOnlyProperty => count;

	public void Initialize()
	{
	}
	public void Step()
	{
		if (visible) {
			Hierarchy.Push(pose.ToMatrix());
			for (int i = 0; i < count; i++)
			{
				float pct = i / (float)(Math.Max(1,count-1));
				Mesh.Sphere.Draw(Material.Default, Matrix.TS((pct-0.5f) * (count*0.1f), 0, 0, 0.1f), Color.White * brightness);
			}
			Hierarchy.Pop();
		}
	}

	public void Shutdown() { }
}