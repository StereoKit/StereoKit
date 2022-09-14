using StereoKit;
using System.Collections.Generic;

class DocModel : ITest
{
	Model testModel;

	bool TestModel()
	{
		/// :CodeSample: Model.Model
		Model model = new Model();
		model.AddNode("Cube",
			Matrix .Identity,
			Mesh   .GenerateCube(Vec3.One),
			Default.Material);
		/// :End:
		/// :CodeSample: Model.AddSubset
		model.AddSubset(
			Mesh   .GenerateSphere(1),
			Default.Material,
			Matrix .T(0,1,0));
		/// :End:

		/// :CodeSample: Model Model.Visuals Mesh.VertCount Mesh.IndCount
		/// ### Counting the Vertices and Triangles in a Model
		/// 
		/// Model.Visuals are always guaranteed to have a Mesh, so no need to
		/// null check there, and VertCount and IndCount are available even if
		/// Mesh.KeepData is false!
		int vertCount = 0;
		int triCount  = 0;

		foreach (ModelNode node in model.Visuals)
		{
			Mesh mesh = node.Mesh;
			vertCount += mesh.VertCount;
			triCount  += mesh.IndCount / 3;
		}
		Log.Info($"Model stats: {vertCount} vertices, {triCount} triangles");
		/// :End:

		int lastSubset = model.AddSubset(
			Mesh   .GenerateCube(new Vec3(0.5f,1,0.5f)),
			Default.Material,
			Matrix .T(0, 1, 0));

		testModel = model;

		// Check that model bounds are as expected
		Log.Info("Expected bounds: center:<0,0.5,0> dimensions:<1,2,1> Got: {0}", model.Bounds);
		if (model.Bounds.center.x != 0 ||
			model.Bounds.center.y != 0.5f ||
			model.Bounds.center.z != 0 ||
			model.Bounds.dimensions.x != 1 ||
			model.Bounds.dimensions.y != 2 ||
			model.Bounds.dimensions.z != 1)
			return false;

		model.RemoveSubset(lastSubset);

		/// :CodeSample: Model.SubsetCount Model.GetMaterial Model.SetMaterial
		for (int i = 0; i < model.SubsetCount; i++)
		{
			// GetMaterial will often returned a shared resource, so 
			// copy it if you don't wish to change all assets that 
			// share it.
			Material mat = model.GetMaterial(i).Copy();
			mat[MatParamName.ColorTint] = Color.HSV(0, 1, 1);
			model.SetMaterial(i, mat);
		}
		/// :End:

		return true;
	}

	bool TestModelInfo()
	{
		Model model = new Model();

		/// :CodeSample: ModelNode.Info ModelNodeInfoCollection.Add ModelNodeInfoCollection.Remove
		/// ### Modifying ModelNode.Info
		/// While ModelNode.Info is automatically populated from a GLTF's
		/// "extras", you can also embed or modify with your own data as well.
		ModelNode modelNode = model.AddNode("empty", Matrix.Identity);
		modelNode.Info.Add("a", "1");
		modelNode.Info.Add("b", "2");
		modelNode.Info.Add("c", "3");
		modelNode.Info.Add("c", "10"); // overwrite 'c's value
		modelNode.Info.Remove("b");
		/// :End:

		/// :CodeSample: ModelNode.Info ModelNodeInfoCollection.Keys ModelNodeInfoCollection.Values
		/// ### Iterating through ModelNode.Info
		/// You can choose to iterate through different parts of ModelNode.Info
		/// using foreach loops.
		foreach (ModelNode node in model.Nodes)
		{
			foreach (KeyValuePair<string, string> kvp in node.Info)
				Log.Info($"{kvp.Key} - {kvp.Value}");

			foreach (string key in node.Info.Keys)
				Log.Info($"key: {key} - {node.Info[key]}");

			foreach (string val in node.Info.Values)
				Log.Info($"value: {val}");
		}
		/// :End:

		if (modelNode.Info.Count != 2) return false;
		if (modelNode.Info["a"] != "1") return false;
		if (modelNode.Info["c"] != "10") return false;

		modelNode.Info.Clear();

		if (modelNode.Info.Count != 0) return false;

		modelNode.Info.Add("c", "4");
		if (modelNode.Info["c"] != "4") return false;

		return true;
	}

	public void Initialize()
	{
		Tests.Test(TestModel);
		Tests.Test(TestModelInfo);
	}
	public void Shutdown  (){}
	public void Update    ()
	{
		testModel.Draw(Matrix.Identity);
		Tests.Screenshot("Tests/ModelSubsets.jpg", 600, 600, new Vec3(0,0.5f,-2), new Vec3(0, 0.5f, 0));
	}
}
