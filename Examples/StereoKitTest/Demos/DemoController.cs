
using StereoKit;
using StereoKit.Framework;
using System;

namespace StereoKitTest
{
	class DemoController : ITest
	{
		public void Initialize()
		{
			Default.MaterialHand[MatParamName.ColorTint] = new Color(1,1,1,0.4f);
		}

		public void Shutdown()
		{
			Default.MaterialHand[MatParamName.ColorTint] = new Color(1,1,1,1);
		}

		Pose windowPose = new Pose(0.4f,0,-0.4f, Quat.LookDir(-1,0,1));
		public void Update()
		{
			UI.WindowBegin("Controllers", ref windowPose);

			UI.WindowEnd();

			ShowController(Handed.Right);
			ShowController(Handed.Left);
		}

		void ShowController(Handed hand)
		{
			Controller c = Input.Controller(hand);
			if (!c.IsTracked) return;

			Hierarchy.Push(c.pose.ToMatrix());
				Color color = Color.White;
				if (c.IsRotTracked) color = new Color(1,1,0);
				if (c.IsPosTracked) color = new Color(0,1,0);
				Default.MeshCube.Draw(Default.Material, Matrix.S(new Vec3(3, 3, 8) * U.cm), color);
				Hierarchy.Push(Matrix.TR(0,1.6f*U.cm,0, Quat.LookAt(Vec3.Zero, new Vec3(0,1,0), new Vec3(0,0,-1))));
					Text.Add(c.IsX1Pressed?"(X1)":"X1", Matrix.TS(0,0.00f,0, 0.25f));
					Text.Add(c.IsX2Pressed?"(X2)":"X2", Matrix.TS(0,0.01f,0, 0.25f));

					Vec3 stickAt = new Vec3(0, 0.03f, 0);
					Lines.Add(stickAt, stickAt + c.stick.XY0*0.01f, Color.White, 0.001f);
					if (c.IsStickClicked) Text.Add("O", Matrix.TS(stickAt, 0.25f));

					Default.MeshCube.Draw(Default.Material, Matrix.TS(0, -0.015f, -0.005f, new Vec3(0.01f, 0.04f, 0.01f)) * Matrix.TR(new Vec3(0,0.02f,0.03f), Quat.FromAngles(-45+c.trigger*40, 0,0) ));
					Default.MeshCube.Draw(Default.Material, Matrix.TS(0.0149f*(hand == Handed.Right?1:-1), 0, 0.015f, new Vec3(0.01f*(1-c.grip), 0.04f, 0.01f)));
				Hierarchy.Pop();
			Hierarchy.Pop();
			Default.MeshCube.Draw(Default.Material, c.aim.ToMatrix(new Vec3(1,1,4) * U.cm), Color.HSV(0,0.5f,0.8f).ToLinear());
		}
	}
}
