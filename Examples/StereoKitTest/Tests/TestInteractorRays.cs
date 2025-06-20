﻿using System;
using System.Linq;
using StereoKit;

class TestInteractorRays : ITest
{
	struct Update(Vec3 pos, Vec3 dir, BtnState state)
	{
		public Vec3     pos   = pos;
		public Vec3     dir   = dir;
		public BtnState state = state;
	}

	struct Test(string name, Update[] u, Action draw, Func<int, bool> expects)
	{
		public string          name    = name;
		public Update[]        u       = u;
		public Action          draw    = draw;
		public Func<int, bool> expects = expects;
	}

	DefaultInteractors prevDefault;
	Interactor         ray;

	Pose windowPose = new Pose(0, 0, -0.5f, Quat.LookDir(-Vec3.Forward));
	Vec3 activeElementPos;

	int frameIdx = -1;
	int testIdx  = 0;

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
			new ("Button",  buttonClick, ()=>{ button      = UI.Button     ("Button"); },                         (f)=>{ return new bool[] {false, false, false, true}[f] == button; } ),
			new ("Round",   buttonClick, ()=>{ roundButton = UI.ButtonRound("RoundButton", Sprite.Shift); },      (f)=>{ return new bool[] {false, false, false, true}[f] == roundButton; } ),
			new ("Toggle",  buttonClick, ()=>{ toggleEvt   = UI.Toggle     ("Toggle", ref toggle); },             (f)=>{ return new bool[] {false, false, false, true}[f] == toggleEvt && new bool[] {false, false, false, true}[f] == toggle; } ),
			new ("Radio1",  buttonClick, ()=>{ if (UI.Radio("Radio 1", radio == 0)) radio = 0; },                 (f)=>{ return new int [] {-1,    -1,    -1,    0}   [f] == radio; } ),
			new ("Radio2",  buttonClick, ()=>{ if (UI.Radio("Radio 2", radio == 1)) radio = 1; },                 (f)=>{ return new int [] { 0,     0,     0,    1}   [f] == radio; } ),
			new ("Slider1", sliderDrag,  ()=>{ slider1Evt = UI.HSlider("slider1", ref slider1, 0, 1, 0, 0.2f); }, (f)=>{ return new bool[] { false, false, true, true, true, false}[f] == slider1Evt && Math.Abs(new float[] { 0, 0, .5f, 1, 0, 0}[f] - slider1) < 0.0001f; } ),
			new ("Slider2", sliderDrag,  ()=>{ slider2Evt = UI.HSlider("slider2", ref slider2, 0, 1, 0, 0.2f); }, (f)=>{ return new bool[] { false, false, true, true, true, false}[f] == slider2Evt && Math.Abs(new float[] { 0, 0, .5f, 1, 0, 0}[f] - slider2) < 0.0001f; } ),
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

	public void Step()
	{
		if (TestUpdate(ray, testUpdates[testIdx].u, activeElementPos, frameIdx))
		{
			frameIdx = -1;
			testIdx  = (testIdx + 1) % testUpdates.Length;
			if (testIdx == 0)
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
			if (testIdx == i) activeElementPos = Hierarchy.ToWorld(UI.LayoutLast.center);
			if (testIdx == i && frameIdx >= 0)
			{
				if (!testUpdates[i].expects(frameIdx))
					Log.Err($"Failed interactor test {testUpdates[i].name} frame {frameIdx}");
			}
		}
		UI.WindowEnd();

		frameIdx++;
	}

	static bool TestUpdate(Interactor ray, Update[] update, Vec3 elementCenter, int frame)
	{
		if (frame < 0)              return false;
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