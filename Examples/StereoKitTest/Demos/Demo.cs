// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using StereoKit;

abstract class Demo
{
	public static readonly Matrix contentPose = Matrix.TR(0, 0, -0.6f, Quat.LookDir(0, 0, 1));
	public static readonly Matrix descPose    = Matrix.TR (0.7f, 0, -0.3f, Quat.LookDir(-1,0,1));
	public static readonly Matrix titlePose   = Matrix.TRS(V.XYZ(0.7f, 0.05f, -0.3f), Quat.LookDir(-1, 0, 1), 2);

	public static void ShowSummary(string title, string description, Bounds experienceBounds)
	{
		if (Tests.IsTesting)
		{
			Tests.Screenshot($"Demos/{title.Replace(" ", "")}.jpg", 1, 480, 270, 60, contentPose.Transform(V.XYZ(0,0.05f,-0.5f)), contentPose.Transform(V.XYZ(0,-0.1f,0)));
			return;
		}

		Text.Add(title,       titlePose);
		Text.Add(description, descPose, V.XY(0.4f, 0), TextFit.Wrap, TextAlign.TopCenter, TextAlign.TopLeft);
		Mesh.Cube.Draw(Material.UIBox, Matrix.TS(experienceBounds.center, experienceBounds.dimensions) * contentPose);
	}
}