using StereoKit;

class DemoMaterialChain : ITest
{
	string title       = "Material Chain";
	string description = "Materials can be chained together to create a multi-pass material! What you're seeing in this sample is an 'Inverted Shell' outline, a two-pass effect where a second render pass is scaled along the normals and flipped inside-out.";

	/// :CodeSample: Material.Chain
	/// ### Inverted Shell Chain
	/// Materials can be chained together to create a multi-pass material! What
	/// you're seeing here is an 'Inverted Shell' outline, a two-pass effect
	/// where a second render pass is scaled along the normals and flipped
	/// inside-out.
	/// 
	/// ![A sphere with an inverted shell outline]({{site.screen_url}}/InvertedShell.jpg)
	Material outlineMaterial;

	void CreateShellMaterial()
	{
		Material shellMaterial = new Material(Shader.FromFile("inflatable.hlsl"));
		shellMaterial.FaceCull = Cull.Front;
		shellMaterial[MatParamName.ColorTint] = Color.HSV(0.1f, 0.7f, 1);

		outlineMaterial = Material.Default.Copy();
		outlineMaterial.Chain = shellMaterial;
	}

	void DrawOutlineObject()
	{
		Mesh.Sphere.Draw(outlineMaterial, Matrix.S(0.3f));
	}
	/// :End:

	public void Initialize() => CreateShellMaterial();
	public void Shutdown() { }
	public void Update()
	{
		Vec3 at = new Vec3(0.5f, -0.1f, -0.5f);
		Hierarchy.Push(Matrix.T(at));
		DrawOutlineObject();
		Hierarchy.Pop();

		Tests.Screenshot("InvertedShell.jpg", 400, 400, 90, at + V.XYZ(-0.2f, -0.04f, 0.2f), at);

		Demo.ShowSummary(title, description);
	}
}