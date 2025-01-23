using StereoKit;

class DocRenderList : ITest
{
	/// :CodeSample: RenderList RenderList.Add RenderList.DrawNow
	/// ### Render Icon From a Model
	///
	/// ![UI with a custom rendererd icon]({{site.screen_url}}/Docs/RenderListIcon.jpg)
	/// 
	/// One place where RenderList excels, is at rendering icons or previews of
	/// Models or scenes! This snippet of code will take a Model asset, and
	/// render a preview of it into a small Sprite.
	///
	static Sprite MakeIcon(Model model, int resolution)
	{
		RenderList list   = new RenderList();
		Tex        result = Tex.RenderTarget(resolution, resolution, 8);

		// Calculate a standard size that will fill the icon to the edges,
		// based on the camera parameters we pass to DrawNow.
		float scale = 1/model.Bounds.dimensions.Length;

		list.Add(model, Matrix.TS(-model.Bounds.center*scale, scale), Color.White);

		// OpenGL renders upside-down to rendertargets, so this is a simple fix
		// for our case here, we just flip the camera upside down.
		Vec3 up = Backend.Graphics == BackendGraphics.D3D11
			?  Vec3.Up
			: -Vec3.Up;
		list.DrawNow(result,
			Matrix.LookAt(V.XYZ(0,0,-1), Vec3.Zero, up),
			Matrix.Perspective(45, 1, 0.01f, 10));

		// Clearing isn't _necessary_ here, but DrawNow does not clear the list
		// after drawing! This will free up assets that were referenced in the
		// list without waiting for GC to destroy the RenderList object.
		list.Clear();

		return Sprite.FromTex(result.Copy());
	}
	/// From there, it's pretty easy to load a Model up, and draw it on a button
	/// in the UI.
	Sprite icon;
	public void Initialize()
	{
		Model model = Model.FromFile("Watermelon.glb");
		// Model loading is async, so we want to make sure the Model is fully
		// loaded before comitting it to a Sprite!
		Assets.BlockForPriority(int.MaxValue);
		icon = MakeIcon(model, 128);
	}

	Pose windowPose = new Pose(0,0,-0.5f, Quat.LookDir(0,0,1));
	void ShowWindow()
	{
		UI.WindowBegin("RenderList Icons", ref windowPose);
		UI.ButtonImg("Icon", icon, UIBtnLayout.CenterNoText, V.XX(UI.LineHeight*2));
		UI.WindowEnd();
	}
	/// :End:

	public void Step()
	{
		ShowWindow();
		Tests.Screenshot("Docs/RenderListIcon.jpg", 1, 400, 400, 45, new Vec3(0,-0.02f,-0.3f), new Vec3(0,-0.02f,-0.5f));
	}
	public void Shutdown() {}
}