// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using StereoKit;
using System;
using System.Collections.Generic;

class DemoLines : ITest
{
	string title       = "Lines";
	string description = "";

	Model paletteModel = Model.FromFile("Palette.glb", Default.ShaderUI);
	Pose  palettePose  = Demo.contentPose * new Pose(-0.12f, -0.15f, 0, Quat.FromAngles(-45,0,0));
	Color activeColor  = Color.White;
	float lineSize     = 0.02f;

	public void Initialize() { }
	public void Shutdown() { }

	public void Step()
	{
		Hierarchy.Push(Matrix.T(0.18f, -0.2f, 0) * Demo.contentPose);
		Hierarchy.Push(Matrix.T(0, 0, 0));
		/// :CodeSample: Lines.Add
		Lines.Add(new Vec3(0.1f,0,0), new Vec3(-0.1f,0,0), Color.White, 0.01f);
		/// :End:
		Hierarchy.Pop();

		Hierarchy.Push(Matrix.T(0,0.05f,0));
		/// :CodeSample: Lines.Add
		Lines.Add(new Vec3(0.1f,0,0), new Vec3(-0.1f,0,0), Color.White, Color.Black, 0.01f);
		/// :End:
		Hierarchy.Pop();

		Hierarchy.Push(Matrix.T(0, 0.1f, 0));
		/// :CodeSample: Lines.Add
		Lines.Add(new LinePoint[]{ 
			new LinePoint(new Vec3( 0.1f, 0,     0), Color.White, 0.01f),
			new LinePoint(new Vec3( 0,    0.02f, 0), Color.Black, 0.005f),
			new LinePoint(new Vec3(-0.1f, 0,     0), Color.White, 0.01f),
		});
		/// :End:
		Hierarchy.Pop();
		Hierarchy.Pop();

		DrawMenu();
		Draw(Handed.Right);

		Demo.ShowSummary(title, description,
			new Bounds(V.XY0(0,-0.12f), V.XYZ(.6f, .3f, .2f)));
	}

	void DrawMenu()
	{
		UI.HandleBegin("PaletteMenu", ref palettePose, paletteModel.Bounds);
		paletteModel.Draw(Matrix.Identity);

		Pose p = new Pose(Vec3.Zero, Quat.FromAngles(90, 0, 0));
		UI.PushSurface(p, Vec3.Zero, Vec2.One);
		UI.HSliderAt("Size", ref lineSize, 0.001f, 0.02f, 0, new Vec3(6,-1,0) * U.cm, new Vec2(8,2) * U.cm);
		Lines.Add(new Vec3(6, 1, -1) * U.cm, new Vec3(-2,1,-1) * U.cm, activeColor, lineSize);
		if (UI.ButtonAt("Clear", new Vec3(-1, 9, 0) * U.cm, new Vec2(.06f, UI.LineHeight)))
		{
			drawList  .Clear();
			drawPoints.Clear();
		}
		UI.PopSurface();

		if (UI.VolumeAt("White", new Bounds(new Vec3(4, 0, 7) * U.cm, new Vec3(4,2,4) * U.cm), UIConfirm.Push).IsJustActive())
			SetColor(Color.White);
		if (UI.VolumeAt("Red",   new Bounds(new Vec3(9, 0, 3) * U.cm, new Vec3(4,2,4) * U.cm), UIConfirm.Push).IsJustActive())
			SetColor(new Color(1,0,0));
		if (UI.VolumeAt("Green", new Bounds(new Vec3(9, 0,-3) * U.cm, new Vec3(4,2,4) * U.cm), UIConfirm.Push).IsJustActive())
			SetColor(new Color(0,1,0));
		if (UI.VolumeAt("Blue",  new Bounds(new Vec3(3, 0,-6) * U.cm, new Vec3(4,2,4) * U.cm), UIConfirm.Push).IsJustActive())
			SetColor(new Color(0,0,1));

		UI.HandleEnd();
	}
	void SetColor(Color color)
	{
		activeColor = color;
		Default.MaterialHand[MatParamName.ColorTint] = color;
	}

	Vec3 prevTip;
	bool painting = false;
	List<LinePoint>   drawPoints = new List<LinePoint>();
	List<LinePoint[]> drawList   = new List<LinePoint[]>();
	void Draw(Handed handed)
	{
		Hand hand = Input.Hand(handed);
		Vec3 tip  = Vec3.Lerp(prevTip, hand.pinchPt, 0.3f);

		if (hand.IsJustPinched && !UI.IsInteracting(handed)) { 
			if (drawPoints.Count > 0)
				drawList.Add(drawPoints.ToArray());
			drawPoints.Clear();
			drawPoints.Add(new LinePoint(tip, activeColor, lineSize));
			drawPoints.Add(new LinePoint(tip, activeColor, lineSize));
			prevTip  = tip;
			painting = true;
		}
		if (hand.IsJustUnpinched)
			painting = false;

		if (painting && drawPoints.Count > 1)
		{
			Vec3      prev  = drawPoints[drawPoints.Count - 2].pt;
			Vec3      dir   = (prev - (drawPoints.Count > 2 ? drawPoints[drawPoints.Count - 3].pt : drawPoints[drawPoints.Count - 1].pt)).Normalized;
			float     dist  = Vec3.Distance(prev, tip);
			float     speed = Vec3.Distance(tip, prevTip) * Time.Stepf;
			LinePoint here  = new LinePoint(tip, activeColor, Math.Max(1-speed/0.0003f,0.1f) * lineSize);

			if ((Vec3.Dot( dir, (tip-prev).Normalized ) < 0.99f && dist > 0.01f) || dist > 0.05f) { 
				drawPoints.Add(here);
			} else {
				drawPoints[drawPoints.Count - 1] = here;
			}
		}

		Lines.Add(drawPoints.ToArray());
		for (int i = 0; i < drawList.Count; i++)
			Lines.Add(drawList[i]);
		prevTip = tip;
	}
}
