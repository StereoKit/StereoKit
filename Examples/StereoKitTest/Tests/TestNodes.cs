using StereoKit;
using System.Linq;

class TestNodes : ITest
{
	Pose      _modelPose = Pose.Identity;
	Model     _model;
	ModelNode _surfaceNode;

	public void Initialize() {
		/// :CodeSample: Model Model.AddNode Model.FindNode ModelNode.AddChild
		/// ### Assembling a Model
		/// While normally you'll load Models from file, you can also assemble
		/// them yourself procedurally! This example shows assembling a simple
		/// hierarchy of visual and empty nodes.
		Model model = new Model();
		model
			.AddNode ("Root",    Matrix.S(0.2f), Mesh.Cube, Material.Default)
			.AddChild("Sub",     Matrix.TR (V.XYZ(0.5f, 0, 0), Quat.FromAngles(0, 0, 45)), Mesh.Cube, Material.Default)
			.AddChild("Surface", Matrix.TRS(V.XYZ(0.5f, 0, 0), Quat.LookDir(V.XYZ(1,0,0)), V.XYZ(1,1,1)));

		ModelNode surfaceNode = model.FindNode("Surface");

		surfaceNode.AddChild("UnitX", Matrix.T(Vec3.UnitX));
		surfaceNode.AddChild("UnitY", Matrix.T(Vec3.UnitY));
		surfaceNode.AddChild("UnitZ", Matrix.T(Vec3.UnitZ));
		/// :End:

		Tests.Test(() => model.FindNode("NotPresent") == null);

		/// :CodeSample: Model Model.Nodes
		/// ### Simple iteration
		/// Walking through the Model's list of nodes is pretty
		/// straightforward! This will touch every ModelNode in the Model,
		/// in the order they were defined, regardless of hierarchy position
		/// or contents.
		Log.Info("Iterate nodes:");
		foreach (ModelNode node in model.Nodes)
			Log.Info("  "+ node.Name);
		/// :End:

		/// :CodeSample: Model Model.Visuals
		/// ### Simple iteration of visual nodes
		/// This will iterate through every ModelNode in the Model with visual
		/// data attached to it!
		Log.Info("Iterate visuals:");
		foreach (ModelNode node in model.Visuals)
			Log.Info("  "+ node.Name);
		/// :End:

		/// :CodeSample: Model Model.Visuals Model.Nodes
		/// ### Tagged Nodes
		/// You can search through Visuals and Nodes for nodes with some sort
		/// of tag in their names. Since these names are from your modeling
		/// software, this can allow for some level of designer configuration
		/// that can be specific to your project.
		var nodes = model.Visuals
			.Where(n => n.Name.Contains("[Wire]"));
		foreach (ModelNode node in nodes)
		{
			node.Material = node.Material.Copy();
			node.Material.Wireframe = true;
		}
		/// :End:

		model.AddNode("[Invisible]", Matrix.S(0.3f), Mesh.Sphere, Material.Default);

		/// :CodeSample: Model Model.Nodes ModelNode.Solid ModelNode.Visible
		/// ### Collision Tagged Nodes
		/// One particularly practical example of tagging your ModelNode names
		/// would be to set up collision information for your Model. If, for
		/// example, you have a low resolution mesh designed specifically for
		/// fast collision detection, you can tag your non-solid nodes as
		/// "[Intangible]", and your collider nodes as "[Invisible]":
		foreach (ModelNode node in model.Nodes)
		{
			node.Solid   = node.Name.Contains("[Intangible]") == false;
			node.Visible = node.Name.Contains("[Invisible]")  == false;
		}
		/// :End:

		Log.Info("Recursive ModelNode traversal:");
		RecursiveTraversal(model.RootNode);

		Log.Info("Depth first ModelNode traversal:");
		DepthFirstTraversal(model);

		for (int i = 0; i < model.Nodes.Count; i++)
		{
			Log.Info($"{model.Nodes[i].Name} using shader {model.Nodes[i].Material?.Shader.Name}");
		}

		_model       = model;
		_surfaceNode = surfaceNode;

		Tests.Test(TestEmptyVisuals);
	}

	bool TestEmptyVisuals()
	{
		Model     model = new Model();
		ModelNode node  = model.AddNode(null, Matrix.Identity);
		return node.Mesh == null && node.Material == null;
	}

	/// :CodeSample: Model Model.RootNode Model.Child Model.Sibling Model.Parent
	/// ### Non-recursive depth first node traversal
	/// If you need to walk through a Model's node hierarchy, this is a method
	/// of doing this without recursion! You essentially do this by walking the
	/// tree down (Child) and to the right (Sibling), and if neither is present,
	/// then walking back up (Parent) until it can keep going right (Sibling)
	/// and then down (Child) again.
	static void DepthFirstTraversal(Model model)
	{
		ModelNode node  = model.RootNode;
		int       depth = 0;
		while (node != null)
		{
			string tabs = new string(' ', depth*2);
			Log.Info(tabs + node.Name);

			if      (node.Child   != null) { node = node.Child; depth++; }
			else if (node.Sibling != null)   node = node.Sibling;
			else {
				while (node != null)
				{
					if (node.Sibling != null) {
						node = node.Sibling;
						break;
					}
					depth--;
					node = node.Parent;
				}
			}
		}
	}
	/// :End:

	/// :CodeSample: Model Model.Child Model.Sibling Model.Parent
	/// ### Recursive depth first node traversal
	/// Recursive depth first traversal is a little simpler to implement as
	/// long as you don't mind some recursion :)
	/// This would be called like: `RecursiveTraversal(model.RootNode);`
	static void RecursiveTraversal(ModelNode node, int depth = 0)
	{
		string tabs = new string(' ', depth*2);
		while (node != null)
		{
			Log.Info(tabs + node.Name);
			RecursiveTraversal(node.Child, depth + 1);
			node = node.Sibling;
		}
	}
	/// :End:

	public void Shutdown() { }
	public void Update()
	{
		UI.PushSurface(_modelPose);
		_model.Draw(Matrix.Identity, new Color(0.5f, 0.5f, 0.5f));
		UI.PushSurface(
			_surfaceNode.ModelTransform.Pose,
			_surfaceNode.ModelTransform.Scale.XY0/2,
			_surfaceNode.ModelTransform.Scale.XY);
		Vec3 center = Hierarchy.ToWorld(Vec3.Zero);
		UI.Label("UI on a Model node!");
		UI.HSeparator();
		UI.Button("Do Thing");
		UI.PopSurface();
		UI.PopSurface();

		if (_model.Intersect(new Ray(V.XYZ(SKMath.Cos(Time.Totalf)*0.15f, 1, 0), -Vec3.Up), out Ray at))
			Lines.Add(at, 0.3f, new Color32(0, 255, 0, 255), 0.005f);

		Tests.Screenshot("Tests/NodeUI.jpg", 400, 400, center+Vec3.One*0.15f, center);
	}
}