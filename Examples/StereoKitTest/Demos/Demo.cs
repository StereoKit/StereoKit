// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using StereoKit;
using System;

abstract class Demo
{
	public static readonly Matrix contentPose = Matrix.TR(0, -0.1f, -0.6f, Quat.LookDir(0, 0, 1));
	public static readonly Matrix descPose    = Matrix.TR (0.7f, 0, -0.3f, Quat.LookDir(-1,0,1));
	public static readonly Matrix titlePose   = Matrix.TRS(V.XYZ(0.7f, 0.05f, -0.3f), Quat.LookDir(-1, 0, 1), 2);

	public static void ShowSummary(string title, string description, Bounds experienceBounds, bool showBounds = true)
	{
		if (Tests.IsTesting)
		{
			int imgWidth = 480;
			int imgHeight = 270;
			float aspect = (float)imgWidth / imgHeight;

			float hFov = 45;
			float wFov = hFov * aspect;
			float width     = experienceBounds.dimensions.x / 2;
			float height    = experienceBounds.dimensions.y / 2;
			float wDistance = width  / (float)Math.Tan((wFov * Units.deg2rad) / 2);
			float hDistance = height / (float)Math.Tan((hFov * Units.deg2rad) / 2);

			Vec3 expCenter = contentPose.Transform(experienceBounds.center);
			Vec3 cameraPos = expCenter + V.XYZ(0, 0, experienceBounds.dimensions.z/2 + Math.Max(wDistance, hDistance));

			Tests.Screenshot($"Demos/{title.Replace(" ", "")}.jpg", 1, imgWidth, imgHeight, hFov, cameraPos, expCenter);
			return;
		}

		Text.Add(title,       titlePose);
		Text.Add(description, descPose, V.XY(0.4f, 0), TextFit.Wrap, TextAlign.TopCenter, TextAlign.TopLeft);
		if (showBounds)
			Mesh.Cube.Draw(Material.UIBox, Matrix.TS(experienceBounds.center, experienceBounds.dimensions) * contentPose);
	}
}