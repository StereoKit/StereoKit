using StereoKit;

class DocSliderBehavior : ITest
{
	Vec2 touchPt    = new Vec2(0.5f,0.5f);
	Pose windowPose = new Pose(0,0,-0.5f, Quat.FromAngles(0,180,0));
	public void Step()
	{
		UI.WindowBegin("Slider Panel", ref windowPose, new Vec2(0.3f, 0f));

		Vec2 size = V.XX(UI.LayoutRemaining.x);
		UITouchPanel("touch panel", ref touchPt, size);
		UI.Label($"{(int)(touchPt.x*100)}, {(int)(touchPt.y * 100)}");

		UI.WindowEnd();
	}

	static bool UITouchPanel(string id, ref Vec2 pt, Vec2 size)
	{
		IdHash hash      = UI.StackHash(id);
		float  depth     = UI.Settings.depth;
		Bounds bounds    = UI.LayoutReserve(size, false, depth);
		float  btnHeight = UI.LineHeight * 0.75f;
		Vec3   btnSize   = new Vec3(btnHeight, btnHeight, depth);

		Vec2 prev = pt;
		UI.SliderBehavior(bounds.TLB, bounds.dimensions.XY, hash, ref pt, Vec2.Zero, Vec2.One, btnSize.XY, Vec2.Zero, UIConfirm.Push, out UISliderData slider);
		float focus = UI.GetAnimFocus(hash, slider.focusState, slider.activeState);
		UI.DrawElement(UIVisual.SliderLine, bounds.TLB, new Vec3(bounds.dimensions.x, bounds.dimensions.y, depth*0.1f), slider.focusState.IsActive() ? 0.5f:0);
		UI.DrawElement(UIVisual.SliderPush, slider.buttonCenter.XY0 + btnSize.XY0/2.0f, btnSize, focus);

		return prev.x != pt.x || prev.y != pt.y;
	}

	public void Initialize() {}
	public void Shutdown  () {}
}