using StereoKit;

class DocHandleNode : ITest
{
	ModelNode handleNode;
	Model     model;
	Pose      modelPose = new Pose(0,0,-1, Quat.Identity);

	public void Initialize()
	{
		model = new Model();
		model.AddNode("Cube",
			Matrix.Identity,
			Mesh.GenerateCube(Vec3.One),
			Default.Material);
		model.AddNode("HandleNode",
			Matrix.T(0,0.5f,0),
			Mesh.GenerateCube(V.XYZ(0.5f,0.2f,0.5f)),
			Default.Material);

		handleNode = model.FindNode("HandleNode");
	}

	public void Update()
	{
		UI.PushSurface(modelPose);

		handleNode.ModelTransform.Decompose(out Vec3 pos, out Quat rot, out Vec3 scale);
		Pose handlePose = new Pose(pos, rot);
		if (UI.Handle("modelHandle", ref handlePose, handleNode.Mesh.Bounds))
			handleNode.ModelTransform = handlePose.ToMatrix(scale);
		
		UI.PopSurface();
		model.Draw(modelPose.ToMatrix());
	}

	public void Shutdown() { }
}