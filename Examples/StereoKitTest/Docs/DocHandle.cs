using StereoKit;

class DocHandle : ITest
{
	/// :CodeSample: Model Model.FromFile UI.HandleBegin UI.HandleEnd Bounds Mesh.Bounds Material.UIBox
	/// ### An Interactive Model
	/// 
	/// ![A grabbable GLTF Model using UI.Handle]({{site.screen_url}}/HandleBox.jpg)
	/// 
	/// If you want to grab a Model and move it around, then you can use a
	/// `UI.Handle` to do it! Here's an example of loading a GLTF from file,
	/// and using its information to create a Handle and a UI 'cage' box that
	/// indicates an interactive element.
	/// 
	Model model      = Model.FromFile("DamagedHelmet.gltf");
	Pose  handlePose = new Pose(0,0,0, Quat.Identity);
	float scale      = .15f;

	public void Step() {
		UI.HandleBegin("Model Handle", ref handlePose, model.Bounds*scale);

		model.Draw(Matrix.S(scale));
		Mesh.Cube.Draw(Material.UIBox, Matrix.TS(model.Bounds.center*scale, model.Bounds.dimensions*scale));

		UI.HandleEnd();
	}
	/// :End:

	public void Initialize() { }
	public void Shutdown() { }
	public void Update()
	{
		Step();

		Tests.Screenshot("HandleBox.jpg", 500, 500, 60, V.XYZ(.2f,.2f,-.4f), Vec3.Zero);
	}
}