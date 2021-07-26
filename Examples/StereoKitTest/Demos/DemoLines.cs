using StereoKit;
using System;
using System.Collections.Generic;

namespace StereoKitTest
{
	class DemoLines : ITest
	{
		Pose  windowPose   = new Pose(new Vec3(0.3f, 0, -0.3f), Quat.LookDir(-1,0,1));
		Model paletteModel = Model.FromFile("Palette.glb", Default.ShaderUI);
		Pose  palettePose  = new Pose(new Vec3(-0.3f, 0, -0.3f), Quat.LookDir(1, 0, 1));
		Color activeColor  = Color.White;
		float lineSize     = 0.02f;

		public void Initialize() { }
		public void Shutdown() { }

		public void Update()
		{
			Hierarchy.Push(Matrix.T(0, 0, -0.5f));
			/// :CodeSample: Lines.Add
			Lines.Add(new Vec3(0.1f,0,0), new Vec3(-0.1f,0,0), Color.White, 0.01f);
			/// :End:
			Hierarchy.Pop();

			Hierarchy.Push(Matrix.T(0,0.05f,-0.5f));
			/// :CodeSample: Lines.Add
			Lines.Add(new Vec3(0.1f,0,0), new Vec3(-0.1f,0,0), Color.White, Color.Black, 0.01f);
			/// :End:
			Hierarchy.Pop();

			Hierarchy.Push(Matrix.T(0, 0.1f, -0.5f));
			/// :CodeSample: Lines.Add
			Lines.Add(new LinePoint[]{ 
				new LinePoint(new Vec3( 0.1f, 0,     0), Color.White, 0.01f),
				new LinePoint(new Vec3( 0,    0.02f, 0), Color.Black, 0.005f),
				new LinePoint(new Vec3(-0.1f, 0,     0), Color.White, 0.01f),
			});
			/// :End:
			Hierarchy.Pop();


			UI.WindowBegin("Settings", ref windowPose, new Vec2(20,0)*U.cm);
			if (UI.Button("Clear")) {
				drawList  .Clear();
				drawPoints.Clear();
			}
			UI.WindowEnd();

			DrawMenu();
			Draw(Handed.Right);
		}

		void DrawMenu()
		{
			UI.HandleBegin("PaletteMenu", ref palettePose, paletteModel.Bounds);
			paletteModel.Draw(Matrix.Identity);

			Pose p = new Pose(Vec3.Zero, Quat.FromAngles(90, 0, 0));
			UI.HandleBegin("LineSlider", ref p, new Bounds());
			UI.HSliderAt("Size", ref lineSize, 0.001f, 0.02f, 0, new Vec3(6,-1,0) * U.cm, new Vec2(8,2) * U.cm);
			Lines.Add(new Vec3(6, 1, -1) * U.cm, new Vec3(-2,1,-1) * U.cm, activeColor, lineSize);
			UI.HandleEnd();

			if (UI.VolumeAt("White", new Bounds(new Vec3(4, 0, 7) * U.cm, new Vec3(4,2,4) * U.cm)))
				SetColor(Color.White);
			if (UI.VolumeAt("Red",   new Bounds(new Vec3(9, 0, 3) * U.cm, new Vec3(4,2,4) * U.cm)))
				SetColor(new Color(1,0,0));
			if (UI.VolumeAt("Green", new Bounds(new Vec3(9, 0,-3) * U.cm, new Vec3(4,2,4) * U.cm)))
				SetColor(new Color(0,1,0));
			if (UI.VolumeAt("Blue",  new Bounds(new Vec3(3, 0,-6) * U.cm, new Vec3(4,2,4) * U.cm)))
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
				float     speed = Vec3.Distance(tip, prevTip) * Time.Elapsedf;
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
}
