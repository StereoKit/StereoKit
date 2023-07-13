// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using StereoKit;

class DemoWorldMesh : ITest
{
	string title       = "World Mesh";
	string description = "";

	Pose     windowPose        = Demo.contentPose.Pose;
	Material occlusionMaterial;
	Material oldMaterial;
	bool     settingsWireframe = true;
	int      settingsColor     = 0;

	public void Initialize()
	{
		// Make the occluder material visible and obvious for the demo, by
		// default the material will work like a solid, invisible occluder.
		occlusionMaterial = Default.Material.Copy();
		occlusionMaterial[MatParamName.ColorTint] = new Color(1, 0, 0);
		occlusionMaterial.Wireframe               = true;

		oldMaterial             = World.OcclusionMaterial;
		World.OcclusionMaterial = occlusionMaterial;

		World.RefreshType     = WorldRefresh.Timer;
		World.RefreshInterval = 2;
		World.RefreshRadius   = 5;
	}

	public void Shutdown()
		=> World.OcclusionMaterial = oldMaterial;

	public void Step() {
		UI.WindowBegin("Settings", ref windowPose, Vec2.Zero);

		if (!SK.System.worldOcclusionPresent)
			UI.Label("World occlusion isn't available on this system");
		UI.PushEnabled(SK.System.worldOcclusionPresent);

		bool occlusion = World.OcclusionEnabled;
		if (UI.Toggle("Enable Occlusion", ref occlusion))
			World.OcclusionEnabled = occlusion;
		if (UI.Toggle("Wireframe", ref settingsWireframe))
			occlusionMaterial.Wireframe = settingsWireframe;
		if (UI.Radio("Red",   settingsColor == 0)) { settingsColor = 0; occlusionMaterial.SetColor("color", Color.HSV(0,1,1)); } UI.SameLine();
		if (UI.Radio("White", settingsColor == 1)) { settingsColor = 1; occlusionMaterial.SetColor("color", Color.White);      } UI.SameLine();
		if (UI.Radio("Black", settingsColor == 2)) { settingsColor = 2; occlusionMaterial.SetColor("color", Color.BlackTransparent); }

		UI.PopEnabled();

		UI.HSeparator();

		if (!SK.System.worldRaycastPresent)
			UI.Label("World raycasting isn't available on this system");
		UI.PushEnabled(SK.System.worldRaycastPresent);

		bool raycast = World.RaycastEnabled;
		if (UI.Toggle("Enable Raycast", ref raycast))
			World.RaycastEnabled = raycast;

		UI.PopEnabled();

		UI.WindowEnd();

		for (int i = 0; i < 2; i++)
		{
			Hand hand = Input.Hand(i);
			if (!hand.IsTracked) continue;

			Ray fingerRay = hand[FingerId.Index, JointId.Tip].Pose.Ray;
			if (World.Raycast(fingerRay, out Ray at))
				Mesh.Sphere.Draw(Material.Default, Matrix.TS(at.position, 0.03f), new Color(1, 0, 0));
		}

		Demo.ShowSummary(title, description, new Bounds(V.XY0(0,-0.12f), V.XYZ(.38f, .34f, 0.1f)));
	}
}