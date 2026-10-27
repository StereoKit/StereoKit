using StereoKit;
using System;

class TestLayout : ITest
{
	const float Width  = 0.2f;
	const float Height = 0.03f;

	int frame;

	public void Initialize() { frame = 0; Tests.RunForFrames(4); }
	public void Shutdown  () { }

	public void Step()
	{
		// Auto-sized windows measure themselves from the previous frame, so
		// the first frame has nothing to go on.
		bool  check  = frame > 0;
		float gutter = UI.Settings.gutter;
		frame++;

		Pose pose = new Pose(0, 0, -0.5f, Quat.LookDir(0, 0, 1));
		UI.WindowBegin("Auto", ref pose);
		float top          = UI.LayoutAt.y;
		Vec2  topRemaining = UI.LayoutRemaining;

		UI.LayoutReserve(V.XY(Width, Height));
		Expect(check, "x after a full line", UI.LayoutRemaining.x, Width);

		UI.LayoutReserve(V.XY(0.08f, Height));
		UI.SameLine();
		Expect(check, "x on the same line", UI.LayoutRemaining.x, Width - (0.08f + gutter));
		UI.NextLine();

		UI.LayoutReserve(Vec2.Zero);
		Expect(check, "fill width", UI.LayoutLast.dimensions.x, Width);

		UI.PanelBegin();
		Expect(check, "x in a panel", UI.LayoutRemaining.x, Width);
		UI.Label("Panel");
		UI.PanelEnd();

		float bottom = UI.LayoutAt.y + gutter;
		UI.WindowEnd();

		Expect(check, "x at the top", topRemaining.x, Width);
		Expect(check, "y at the top", topRemaining.y, top - bottom);
	}

	void Expect(bool check, string name, float actual, float expected)
	{
		if (check && MathF.Abs(actual - expected) > 0.0001f)
			Tests.Fail($"LayoutRemaining {name} on frame {frame}: expected {expected:0.0000}, got {actual:0.0000}");
	}
}
