using StereoKit;
using System.Collections.Generic;
using System.Linq;

class DemoBasics : ITest
{
    Solid floorSolid;
    Model       gltf;
    List<Solid> objects = new List<Solid>();

    public void Initialize()
    {
        gltf = Model.FromFile("DamagedHelmet.gltf", Default.ShaderPbr);

        floorSolid = new Solid(new Vec3(0, -1.5f, 0), Quat.Identity, SolidType.Immovable);
        floorSolid.AddBox(new Vec3(20, .01f, 20));
    }

    public void Update()
    {
        if (Input.Hand(Handed.Right).IsJustPinched)
        {
            objects.Add(new Solid(new Vec3(0, 3, 0), Quat.Identity));
            objects.Last().AddSphere(.45f, 40);
            objects.Last().AddBox(Vec3.One * .35f, 40);
        }

        Pose solidPose;
        for (int i = 0; i < objects.Count; i++)
        {
            objects[i].GetPose(out solidPose);
            Renderer.Add(gltf, solidPose.ToMatrix(0.25f), Color.White);
        }
    }

    public void Shutdown()
    {
    }
}