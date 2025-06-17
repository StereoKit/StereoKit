using System;
using System.Linq;
using StereoKit;

class TestInteractors : ITest
{
	DefaultInteractors prevDefault;
	Interactor         ray;

	struct Update
	{
		public Vec3 pos, dir;
		public BtnState state;
		public Update(Vec3 pos, Vec3 dir, BtnState state)
		{
			this.pos = pos;
			this.dir = dir;
			this.state = state;
		}
	}
	struct Test
	{
		public string name;
		public Update[] u;
		public Action draw;
		public Func<int, bool> expects;
	}

	Update[] buttonClick = [
		new (V.XYZ(0,0,0.2f), V.XYZ(0,0,-1), BtnState.Inactive), // Must have focus for a frame before element can activate
		new (V.XYZ(0,0,0.2f), V.XYZ(0,0,-1), BtnState.Active | BtnState.JustActive),
		new (V.XYZ(0,0,0.2f), V.XYZ(0,0,-1), BtnState.Active),
		new (V.XYZ(0,0,0.2f), V.XYZ(0,0,-1), BtnState.JustInactive),
	];
	Update[] sliderDrag = [ // Requires slider size of 0.2f
		new (V.XYZ(-.1f,0,0.2f), V.XYZ(0,0,-1), BtnState.Inactive), // Must have focus for a frame before element can activate
		new (V.XYZ(-.1f,0,0.2f), V.XYZ(0,0,-1), BtnState.Active | BtnState.JustActive),
		new (V.XYZ(  0, 0,0.2f), V.XYZ(0,0,-1), BtnState.Active),
		new (V.XYZ( .1f,0,0.2f), V.XYZ(0,0,-1), BtnState.Active),
		new (V.XYZ(-.1f,0,0.2f), V.XYZ(0,0,-1), BtnState.Active),
		new (V.XYZ(-.1f,0,0.2f), V.XYZ(0,0,-1), BtnState.JustInactive),
	];
	Test[] testUpdates;

	bool  button      = false;
	bool  roundButton = false;
	bool  toggle      = false;
	bool  toggleEvt   = false;
	int   radio       = -1;
	float slider1     = 0;
	bool  slider1Evt  = false;
	float slider2     = 0;
	bool  slider2Evt  = false;

	public void Initialize()
	{
		testUpdates = [
			new Test { name = "Button", u = buttonClick, draw = ()=>{ button      = UI.Button     ("Button"); },                         expects = (f)=>{ return new bool[] {false, false, false, true}[f] == button; } },
			new Test { name = "Round",  u = buttonClick, draw = ()=>{ roundButton = UI.ButtonRound("RoundButton", Sprite.Shift); },      expects = (f)=>{ return new bool[] {false, false, false, true}[f] == roundButton; } },
			new Test { name = "Toggle", u = buttonClick, draw = ()=>{ toggleEvt   = UI.Toggle     ("Toggle", ref toggle); },             expects = (f)=>{ return new bool[] {false, false, false, true}[f] == toggleEvt && new bool[] {false, false, false, true}[f] == toggle; } },
			new Test { name = "Radio1", u = buttonClick, draw = ()=>{ if (UI.Radio("Radio 1", radio == 0)) radio = 0; },                 expects = (f)=>{ return new int [] {-1,    -1,    -1,    0}   [f] == radio; } },
			new Test { name = "Radio2", u = buttonClick, draw = ()=>{ if (UI.Radio("Radio 2", radio == 1)) radio = 1; },                 expects = (f)=>{ return new int [] { 0,     0,     0,    1}   [f] == radio; } },
			new Test { name = "Slider1", u = sliderDrag, draw = ()=>{ slider1Evt = UI.HSlider("slider1", ref slider1, 0, 1, 0, 0.2f); }, expects = (f)=>{ return new bool[] { false, false, true, true, true, false}[f] == slider1Evt && Math.Abs(new float[] { 0, 0, .5f, 1, 0, 0}[f] - slider1) < 0.0001f; } },
			new Test { name = "Slider2", u = sliderDrag, draw = ()=>{ slider2Evt = UI.HSlider("slider2", ref slider2, 0, 1, 0, 0.2f); }, expects = (f)=>{ return new bool[] { false, false, true, true, true, false}[f] == slider2Evt && Math.Abs(new float[] { 0, 0, .5f, 1, 0, 0}[f] - slider2) < 0.0001f; } },
		];

		prevDefault = Interaction.DefaultInteractors;
		Interaction.DefaultInteractors = DefaultInteractors.None;

		ray = Interactor.Create(InteractorType.Line, InteractorEvent.Pinch | InteractorEvent.Poke, InteractorActivation.State, -1, 0.001f, 0);

		// This puts us alll the way back to test == 0, so we know we've
		// executed all our interactor tests.
		Tests.RunForFrames(testUpdates.Sum(u => u.u.Length + 1) + 1);
	}

	public void Shutdown()
	{
		ray.Destroy();
		Interaction.DefaultInteractors = prevDefault;
	}

	Pose windowPose = new Pose(0, 0, -0.5f, Quat.LookDir(-Vec3.Forward));
	Vec3 uiPos;

	int frame = -1;
	int test  = 0;

	public void Step()
	{
		if (TestUpdate(testUpdates[test].u, uiPos, frame))
		{
			frame = -1;
			test  = (test + 1) % testUpdates.Length;
			if (test == 0)
			{
				button      = false;
				roundButton = false;
				toggle      = false;
				toggleEvt   = false;
				radio       = -1;
				slider1     = 0;
				slider1Evt  = false;
				slider2     = 0;
				slider2Evt  = false;
			}
		}
		DrawInteractor(ray);

		UI.WindowBegin("Testing UI", ref windowPose, V.XY(0.3f, 0.4f));
		for (int i = 0; i < testUpdates.Length; i++)
		{
			testUpdates[i].draw();
			if (test == i) uiPos = Hierarchy.ToWorld(UI.LayoutLast.center);
			if (test == i && frame >= 0)
			{
				if (!testUpdates[i].expects(frame))
					Log.Err($"Failed interactor test {testUpdates[i].name} frame {frame}");
			}
		}
		UI.WindowEnd();

		frame++;
	}

	bool TestUpdate(Update[] update, Vec3 elementCenter, int frame)
	{
		if (frame < 0) return false;
		if (frame >= update.Length) return true;

		Update u   = update[frame];
		Vec3   pos = u.pos + elementCenter;
		ray.Update(pos, pos + u.dir * 100, new Pose(pos, Quat.LookDir(u.dir)), pos, Vec3.Zero, u.state, BtnState.Active);
		return false;
	}

	static void DrawInteractor(Interactor actor)
	{
		Lines.Add(actor.Motion.Ray, 100, Color.White, 0.002f);
		if (actor.TryGetFocusBounds(out Pose p, out Bounds b))
			Mesh.Cube.Draw(Material.UIBox, Matrix.TS(b.center, b.dimensions) * p.ToMatrix());
	}
}