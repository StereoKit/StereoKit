using StereoKit;

class TestZBuffer : ITest
{

	Material front;
	Material back;
	public void Initialize()
	{
		front = Material.Unlit.Copy();
		back  = Material.Unlit.Copy();

		front[MatParamName.ColorTint] = Color.HSV(0,     0.8f, 0.8f);
		back [MatParamName.ColorTint] = Color.HSV(0.33f, 0.8f, 0.8f);

		UpdateMaterialSettings();
	}
	public void Shutdown() { }

	float dist       = 1f;
	float separation = 0.001f;
	public void Step()
	{
		Mesh.Quad.Draw(front, Matrix.TR(0,0, -dist-separation, Quat.FromAngles(0,180,0)));
		Mesh.Quad.Draw(back,  Matrix.TR(0,0, -dist,            Quat.FromAngles(0,180,0)));

		if (Input.Key(Key.T).IsActive()) dist -= Time.Stepf * 2f;
		if (Input.Key(Key.G).IsActive()) dist += Time.Stepf * 2f;

		const float step = -5;
		if (Input.Key(Key.N1).IsJustActive()) dist = 1 * step;
		if (Input.Key(Key.N2).IsJustActive()) dist = 2 * step;
		if (Input.Key(Key.N3).IsJustActive()) dist = 3 * step;
		if (Input.Key(Key.N4).IsJustActive()) dist = 4 * step;
		if (Input.Key(Key.N5).IsJustActive()) dist = 5 * step;
		if (Input.Key(Key.N6).IsJustActive()) dist = 6 * step;
		if (Input.Key(Key.N7).IsJustActive()) dist = 7 * step;
		if (Input.Key(Key.N8).IsJustActive()) dist = 8 * step;
		if (Input.Key(Key.N9).IsJustActive()) dist = 9 * step;
		if (Input.Key(Key.N0).IsJustActive()) dist = 10 * step;

		SettingsWindow();
	}

	Pose pose = new Pose(0, 0, -0.5f, Quat.FromAngles(0,180,0));
	bool frontFirst = false;
	bool frontEq    = false;
	bool backEq     = false;
	void SettingsWindow()
	{
		Vec2 labelSize = new Vec2(0.1f,0);
		UI.WindowBegin("Settings", ref pose, V.XY(0.3f,0));

		UI.Label("Draw first", labelSize);
		UI.SameLine();
		if( UI.Radio("Front", frontFirst)) { frontFirst = true; UpdateMaterialSettings(); } 
		UI.SameLine();
		if (UI.Radio("Back", !frontFirst)) { frontFirst = false; UpdateMaterialSettings(); }

		if (UI.Toggle("Front discard if equal", ref frontEq)) UpdateMaterialSettings();
		if (UI.Toggle("Back discard if equal",  ref backEq)) UpdateMaterialSettings();

		UI.Label($"Separation: {separation*1000:0.0}mm", labelSize);
		UI.SameLine();
		UI.HSlider("separation", ref separation, 0, 0.002f);

		UI.Label($"Distance: {dist:0.0}m", labelSize);
		UI.SameLine();
		UI.HSlider("distance", ref dist, 1, 50);

		UI.WindowEnd();
	}

	void UpdateMaterialSettings ()
	{
		if (frontFirst)
		{
			front.QueueOffset = 10;
			back .QueueOffset = 11;
		} else {
			front.QueueOffset = 11;
			back .QueueOffset = 10;
		}

		front.DepthTest = frontEq ? DepthTest.LessOrEq : DepthTest.Less;
		back .DepthTest = backEq  ? DepthTest.LessOrEq : DepthTest.Less;
	}
}