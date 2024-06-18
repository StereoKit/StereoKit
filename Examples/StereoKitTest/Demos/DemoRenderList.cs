using StereoKit;

class DemoRenderList : ITest
{
	string title       = "Render Lists";
	string description = "";

	Tex renderTex = Tex.RenderTarget(1024, 1024);
	Material renderMat = Material.Unlit.Copy();

	public void Step()
	{
		ShowTextInputWindow();

		Demo.ShowSummary(title, description, new Bounds(V.XY0(0, -0.19f), V.XYZ(.4f, .5f, 0)));
	}

	Pose winPose = Pose.Identity;
	bool clear   = false;
	void ShowTextInputWindow()
	{
		UI.WindowBegin("Render Lists", ref winPose);
		UI.Label($"Render items: {RenderList.Primary.PrevCount}");
		UI.Toggle("Clear", ref clear);

		UI.Label("Offscreen List:");
		Bounds b = UI.LayoutReserve(new Vec2(0.1f, 0.1f));
		Mesh.Quad.Draw(renderMat, Matrix.TS(b.center + Vec3.Forward *0.004f, b.dimensions.XY1));
		UI.WindowEnd();

		if (clear) RenderList.Primary.Clear();
	}

	public void Initialize()
	{
		winPose = (Demo.contentPose * winPose.ToMatrix()).Pose;

		RenderList list = new RenderList();
		Model model = Model.FromFile("DamagedHelmet.gltf");
		list.Add(model, Matrix.R(0,180,0), Color.White);

		Assets.BlockForPriority(int.MaxValue);

		Vec3 at = new Vec3(0,0,1.8f);
		Renderer.ClearColor = Color.HSV(0.4f, 0.3f, 0.5f);
		list.DrawNow(renderTex, Matrix.TR(-at, Quat.LookAt(at, Vec3.Zero)), Matrix.Perspective(90, 1, 0.01f,50), new Rect(0,0,1,1));

		renderMat[MatParamName.DiffuseTex] = renderTex;
	}
	public void Shutdown() { }
}