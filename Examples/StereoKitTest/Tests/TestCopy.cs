using StereoKit;

class TestCopy : ITest
{
	Model _model1;
	Model _model2;

	public void Initialize() {
		/// :CodeSample: Model.Copy Material.Copy
		/// ### Copying assets
		/// Modifying an asset will affect everything that uses that asset!
		/// Often you'll want to copy an asset before modifying it, to
		/// ensure other parts of your application look the same. In
		/// particular, modifying default assets is not a good idea, unless
		/// you _do_ want to modify the defaults globally.
		Model model1 = new Model(Mesh.Sphere, Material.Default);
		model1.RootNode.LocalTransform = Matrix.S(0.1f);

		Material mat = Material.Default.Copy();
		mat[MatParamName.ColorTint] = new Color(1,0,0,1);
		Model model2 = model1.Copy();
		model2.RootNode.Material = mat;
		/// :End:

		_model1 = model1;
		_model2 = model2;
	}
	public void Shutdown() { }
	public void Update()
	{
		_model1.Draw(Matrix.T(-0.1f, 0, 0));
		_model2.Draw(Matrix.T( 0.1f, 0, 0));

		Tests.Screenshot("Tests/ModelCopy.jpg", 400, 400, new Vec3(0, 0, .2f), Vec3.Zero);
	}
}