using StereoKit;
using System;
using System.Collections.Generic;

namespace StereoKitTest
{
	class DemoManyObjects : ITest
	{
		Model model = Model.FromFile("DamagedHelmet.gltf");
		const int cacheCount = 100;
		List<Pose> poseCache = new List<Pose>(cacheCount); 

		public void Initialize() {
			for (int i = 0; i < cacheCount; i++) {
				poseCache.Add(Pose.Identity);
			}
		}

		public void Shutdown() { }

		public void Update()
		{
			Pose curr = poseCache[0];
			UI.Handle("Model", ref curr, model.Bounds * 0.1f);

			for (int y = 0; y < 5; y++)
			{
				for (int x = 0; x < 5; x++)
				{
					Vec3  grid = new Vec3(x-2f, 0, y-4) * 0.5f;
					float dist = Math.Min(1, grid.Length/10.0f);
					Pose  pose = poseCache[(int)(dist * (cacheCount-1))];
					pose.position += grid;
					model.Draw(pose.ToMatrix(0.1f));
				}
			}

			poseCache.RemoveAt(poseCache.Count-1);
			poseCache.Insert(0, curr);
		}
	}
}
