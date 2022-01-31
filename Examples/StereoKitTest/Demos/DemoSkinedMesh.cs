using StereoKit;
using System;
using System.Collections.Generic;

class DemoSkinedMesh : ITest
{
	
	class TrySkinMesh
    {
		Mesh mesh;
		Pose poseone;
		Pose posetwo;
		Pose posethree;

		Pose trypos;

		public void Initialize()
		{
			mesh = new Mesh();
			Vertex[] verts = new Vertex[]
			{
			new Vertex(Vec3.Zero,Vec3.Up),
			new Vertex(Vec3.Zero,Vec3.Up),
			new Vertex(Vec3.Zero,Vec3.Up),
			};
			mesh.SetVerts(verts);
			uint[] ints = new uint[]
			{
			0,1,2
			};
			mesh.SetInds(ints);
			ushort[] testids = new ushort[]
			{
			0 ,0, 0, 0,1 ,0, 0, 0,2 ,0, 0, 0
			};
			Vec4[] wegts = new Vec4[]
			{
			new Vec4(1,0,0,0),new Vec4(1,0,0,0),new Vec4(1,0,0,0),
			};
			Matrix[] stating_poses = new Matrix[]
			{
			poseone.ToMatrix(),
			posetwo.ToMatrix(),
			posethree.ToMatrix(),
			};
			mesh.SetSkin(testids, wegts, stating_poses);
			poseone = new Pose(new Vec3(-1, 1, 1), Quat.Identity);
			posetwo = new Pose(new Vec3(1, -1, 1), Quat.Identity);
			posethree = new Pose(new Vec3(-1, -1, 1), Quat.Identity);
			trypos = new Pose(Vec3.Zero, Quat.Identity);
		}

        readonly string id = Guid.NewGuid().ToString();

		public void Update()
		{
			var scaledown = 0.1f;
			var bounds = new Bounds(new Vec3(0.1f));
			UI.PushId(id);

			var centerpos = (poseone.position + posetwo.position + posethree.position) / 3;
			Pose center = new Pose(centerpos + trypos.position, Quat.Identity);
			UI.Handle("center", ref center, bounds);
			trypos = new Pose(center.position - centerpos, Quat.Identity);
			Mesh.Sphere.Draw(Material.Default, center.ToMatrix(scaledown), new Color(0, 0, 0));

			Hierarchy.Push(trypos.ToMatrix());

			UI.Handle("pos1", ref poseone, bounds);
			UI.Handle("pos2", ref posetwo, bounds);
			UI.Handle("pos3", ref posethree, bounds);

			UI.PopId();
			Mesh.Sphere.Draw(Material.Default, poseone.ToMatrix(scaledown), new Color(1, 0, 0));
			Mesh.Sphere.Draw(Material.Default, posetwo.ToMatrix(scaledown), new Color(0, 1, 0));
			Mesh.Sphere.Draw(Material.Default, posethree.ToMatrix(scaledown), new Color(0, 0, 1));

			//startof draw skinned try
			Matrix[] updated_poses = new Matrix[]
			{
			poseone.ToMatrix(),
			posetwo.ToMatrix(),
			posethree.ToMatrix(),
			};
			mesh.UpdateSkin(updated_poses);
			mesh.Draw(Material.Default, Matrix.Identity);
			//end of draw skinned try
			Hierarchy.Pop();
		}
	}

	List<TrySkinMesh> trySkinMeshes = new List<TrySkinMesh>();

	Pose windowPose = new Pose(0.25f, 0.21f, -0.36f, Quat.LookAt(Vec3.Forward,Vec3.Zero));

	public void Initialize() 
	{
		AddSkinMesh();
	}
	public void Shutdown  () { }

	void AddSkinMesh()
    {
		TrySkinMesh skinMesh = new TrySkinMesh();
		trySkinMeshes.Add(skinMesh);
		skinMesh.Initialize();
	}

	public void Update()
	{
		UI.WindowBegin("SkinedMesh", ref windowPose);
		if(UI.Button("Add Another Mesh"))
        {
			AddSkinMesh();
		}
		foreach (var item in trySkinMeshes)
		{
			item.Update();
		}
		UI.WindowEnd();
	}


}