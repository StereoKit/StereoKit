// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using StereoKit;
using System;
using System.Collections.Generic;

class DemoManyObjects : ITest
{
	string title       = "Many Objects";
	string description = "......";

	Model model = Model.FromFile("DamagedHelmet.gltf");
	const int cacheCount = 100;
	List<Pose> poseCache = new List<Pose>(cacheCount); 

	public void Initialize() {
		for (int i = 0; i < cacheCount; i++) {
			poseCache.Add(Demo.contentPose.Pose);
		}
	}

	public void Shutdown() { }

	public void Step()
	{
		Pose curr = poseCache[0];
		UI.Handle("Model", ref curr, model.Bounds * 0.04f);
		poseCache.RemoveAt(poseCache.Count-1);
		poseCache.Insert(0, curr);

		Mesh.Cube.Draw(Material.UIBox, Matrix.TS(model.Bounds.center*0.04f, model.Bounds.dimensions*0.04f)* curr.ToMatrix());

		for (int y = 0; y < 5; y++)
		{
			for (int x = 0; x < 5; x++)
			{
				Vec3  grid = new Vec3(x-2f, y-2, 0) ;
				float dist = Math.Min(1, grid.Length/14.0f);
				Pose  pose = poseCache[(int)(dist * (cacheCount-1))];
				//pose.position += grid* 0.1f;
				model.Draw(Matrix.TS(grid*0.1f, 0.04f) * pose.ToMatrix());
			}
		}

		Demo.ShowSummary(title, description,
			new Bounds(.6f, .6f, .1f));
	}
}