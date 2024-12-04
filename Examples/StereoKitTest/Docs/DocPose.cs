// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2024 Nick Klingensmith
// Copyright (c) 2024 Qualcomm Technologies, Inc.

using StereoKit;

class DocPose : ITest
{
	static void IdentityPose()
	{
		/// :CodeSample: Pose Pose.Identity Lines Lines.AddAxis
		/// ### Identity Pose
		/// 
		/// The identity pose is a `Pose` at (0,0,0) facing Forward, which in
		/// StereoKit is a direction of (0,0,-1) represented by a Quaternion of
		/// (0,0,0,1). Note that a Quaternion of (0,0,0,0) is invalid, and can
		/// cause problems with math, so using `default` or an empty
		/// `new Pose()` with this struct can result in bad math results.
		/// `Pose.Identity` is a good default to get used to!
		/// 
		/// ![Identity pose at the origin]({{site.screen_url}}/Docs/PoseIdentity.jpg)
		/// 
		/// Note that `Lines.AddAxis` here shows the `Pose` orientation by
		/// drawing the pose local X+ (red) Y+ (blue) Z+ (green) axis lines in
		/// the positive direction, and `Forward` in white.
		/// 
		Pose pose = Pose.Identity;
		Lines.AddAxis(pose);

		// Show the origin for clarity
		Lines.Add(V.XYZ(-1,0,0), V.XYZ(1,0,0), new Color32(100,0,0,100), 0.0005f);
		Lines.Add(V.XYZ(0,-1,0), V.XYZ(0,1,0), new Color32(0,100,0,100), 0.0005f);
		Lines.Add(V.XYZ(0,0,-1), V.XYZ(0,0,1), new Color32(0,0,100,100), 0.0005f);
		/// :End:

		Tests.Screenshot("Docs/PoseIdentity.jpg", 400, 400, 30, Hierarchy.ToWorld(V.XYZ(.1f,.1f,.1f)), Hierarchy.ToWorld(V.XYZ(0,0,0)));
	}

	static void LerpPoses()
	{
		/// :CodeSample: Pose Pose.Pose Pose.LookAt Pose.Lerp
		/// ### Lerping Poses
		/// 
		/// ![Lerping between two poses]({{site.screen_url}}/Docs/PoseLerp.jpg)
		/// 
		/// Here we construct two `Pose`s, one using a position + direction
		/// constructor, and one using a from -> to LookAt function. Both are
		/// valid ways of constructing a `Pose`, check out the [`Quat`]({{site.url}}/Pages/StereoKit/Quat.html)
		/// functions for more tools for creating `Pose` orientations!
		/// 
		/// After that, we're just blending between these two `Pose`s with a
		/// `Pose.Lerp`, and showing the result at 10% intervals.
		Pose a = new Pose(0, 0.5f, -0.5f, Quat.LookDir(1,0,0));
		Pose b = Pose.LookAt(V.XYZ(0,0,0), V.XYZ(0,1,0));

		for (int i = 0; i < 11; i++) {
			Pose p = Pose.Lerp(a, b, i/10.0f);
			Lines.AddAxis(p, 0.05f);
		}

		// Show the origin for clarity
		Lines.Add(V.XYZ(-1,0,0), V.XYZ(1,0,0), new Color32(100,0,0,100), 0.0025f);
		Lines.Add(V.XYZ(0,-1,0), V.XYZ(0,1,0), new Color32(0,100,0,100), 0.0025f);
		Lines.Add(V.XYZ(0,0,-1), V.XYZ(0,0,1), new Color32(0,0,100,100), 0.0025f);
		/// :End:

		Tests.Screenshot("Docs/PoseLerp.jpg", 400, 400, 47, Hierarchy.ToWorld(V.XYZ(.5f, .75f, .25f)), Hierarchy.ToWorld(V.XYZ(0, 0.25f, -0.25f)));
	}

	static void DrawPose()
	{
		/// :CodeSample: Pose Pose.ToMatrix
		/// ### Draw Pose
		/// 
		/// Having a raw and malleable position/orientation available is great,
		/// but with `Pose.ToMatrix`, you can also quickly turn a `Pose` into a
		/// `Matrix` for use with drawing functions or other places where
		/// `Matrix` transforms are needed! `ToMatrix` also has overloads to
		/// include a scale, if one is available.
		/// 
		/// ![Drawing items at a Pose]({{site.screen_url}}/Docs/PoseDraw.jpg)
		/// 
		Pose  pose  = new Pose(0,0,-0.5f, Quat.FromAngles(30,45,0));
		float scale = 0.5f;

		Mesh.Cube.Draw(Material.Default, pose.ToMatrix(scale));
		/// :End:

		Tests.Screenshot("Docs/PoseDraw.jpg", 400, 400, 55, Hierarchy.ToWorld(V.XYZ(0, 0, 0.5f)), Hierarchy.ToWorld(V.XYZ(0, 0, -0.5f)));
	}

	Model model = Model.FromFile("DamagedHelmet.gltf");
	void PoseDirections()
	{
		/// :CodeSample: Pose Pose.Right Pose.Up Pose.Forward
		/// ### Pose Directions
		/// 
		/// ![Pose direction lines]({{site.screen_url}}/Docs/PoseDirections.jpg)
		/// 
		/// `Pose` provides a few handy vector properties to help working with
		/// `Pose` relative directions! `Forward`, `Right`, and `Up` are all
		/// derived from the `Pose`'s orientation, and represent the -Z, +X and
		/// +Y directions of the `Pose`.
		/// 
		Pose p = new Pose(0,0,-0.5f);
		model.Draw(p.ToMatrix(0.03f));

		Lines.Add(p.position, p.position + 0.1f*p.Right,   new Color32(255,0,0,255), 0.005f);
		Lines.Add(p.position, p.position + 0.1f*p.Up,      new Color32(0,255,0,255), 0.005f);
		Lines.Add(p.position, p.position + 0.1f*p.Forward, Color32.White,            0.005f);
		/// :End:

		Tests.Screenshot("Docs/PoseDirections.jpg", 400, 400, 55, Hierarchy.ToWorld(p.position+V.XYZ(0.15f,0.15f,-0.15f)), Hierarchy.ToWorld(p.position));
	}

	public void Step()
	{
		Hierarchy.Push(Tests.IsTesting ? Matrix.T(0, -100, 0) : Matrix.T(-1,0,-0.5f));
		IdentityPose();
		Hierarchy.Pop();

		Hierarchy.Push(Tests.IsTesting ? Matrix.T(100, -100, 0) : Matrix.T(0, 0, -0.5f));
		LerpPoses();
		Hierarchy.Pop();

		Hierarchy.Push(Tests.IsTesting ? Matrix.T(-100, -100, 0) : Matrix.T(1, 0, -0.5f));
		DrawPose();
		Hierarchy.Pop();

		Hierarchy.Push(Tests.IsTesting ? Matrix.T(-200, -100, 0) : Matrix.T(2, 0, -0.5f));
		PoseDirections();
		Hierarchy.Pop();
	}

	public void Initialize() {}
	public void Shutdown  () {}
}