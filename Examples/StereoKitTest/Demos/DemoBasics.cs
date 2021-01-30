using StereoKit;
using System.Collections.Generic;
using System.Linq;

class DemoBasics : ITest
{
	Solid floorSolid;
	Model       gltf;
	List<Solid> objects = new List<Solid>();
	Pose        optionsPose = new Pose(-0.5f, 0,0, Quat.LookDir(1,0,0));

	public void Initialize()
	{
		gltf = Model.FromFile("DamagedHelmet.gltf", Default.ShaderPbr);

		floorSolid = new Solid(new Vec3(0, -1.5f, 0), Quat.Identity, SolidType.Immovable);
		floorSolid.AddBox(new Vec3(20, .01f, 20));

		Input.HandSolid(Handed.Max, false);
	}

	public void Update()
	{
		Pose solidPose;
		for (int i = 0; i < objects.Count; i++)
		{
			objects[i].GetPose(out solidPose);
			Renderer.Add(gltf, solidPose.ToMatrix(0.25f), Color.White);
		}
		for (int i = 0; i < (int)Handed.Max; i++)
		{
			Hand h = Input.Hand((Handed)i);
			if (h.IsJustGripped)
				Input.HandSolid((Handed)i, true);
			if (h.IsJustUngripped)
				Input.HandSolid((Handed)i, false);
		}

		UI.WindowBegin("Options", ref optionsPose);
		if (UI.Button("Add"))
		{
			objects.Add(new Solid(new Vec3(0, 3, 0), Quat.Identity));
			objects.Last().AddSphere(.45f, 40);
			objects.Last().AddBox(Vec3.One * .35f, 40);
		}
		if (UI.Button("Clear"))
			objects.Clear();
		UI.WindowEnd();
	}

	public void Shutdown()
	{
		Input.HandSolid(Handed.Max, true);
	}
}