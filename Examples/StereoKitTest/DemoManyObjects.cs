using StereoKit;
using System;
using System.Collections.Generic;

namespace StereoKitTest
{
    class DemoManyObjects : IDemo
    {
        Model model = Model.FromFile("DamagedHelmet.gltf");
        const int cacheCount = 100;
        List<Pose> poseCache = new List<Pose>(cacheCount); 

        public void Initialize() {
            for (int i = 0; i < cacheCount; i++) {
                poseCache.Add(new Pose(Vec3.Zero, Quat.Identity));
            }
        }

        public void Shutdown() { }

        public void Update()
        {
            Pose curr = poseCache[0];
            UI.AffordanceBegin("Model", ref curr, model.Bounds * 0.1f);
            UI.AffordanceEnd();

            for (int y = 0; y < 5; y++)
            {
                for (int x = 0; x < 5; x++)
                {
                    Vec3  grid = new Vec3(x-2f, 0, y-4) * 0.5f;
                    float dist = Math.Min(1, grid.Magnitude/10.0f);
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
