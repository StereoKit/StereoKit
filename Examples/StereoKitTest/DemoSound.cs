using StereoKit;
using System.Collections.Generic;

namespace StereoKitTest
{
    class DemoSound : IDemo
    {
        Pose  windowPose = new Pose(.4f, 0, -0.4f, Quat.LookDir(-1,0,1));
        Sound fileSound  = Sound.FromFile("BlipNoise.wav");

        float      genDuration = 0.5f;
        Sound      genSound;
        Bounds     genVolume = new Bounds(new Vec3(0.25f, 0.25f, 0.25f));
        bool       genPrevContains = false;
        List<LinePoint> genPath = new List<LinePoint>();

        Mesh boundsMesh = Mesh.GenerateCube(Vec3.One);
        Material boundsMat = Default.MaterialUI.Copy();

        public void Initialize() {
            boundsMat.Transparency = Transparency.Blend;
            boundsMat["color"] = new Color(1, 1, 1, 0.25f);
        }
        public void Shutdown() { }

        public void Update()
        {
            UI.WindowBegin("Sound", ref windowPose, new Vec2(20, 0) * Units.cm2m);
            if (UI.Button("BlipNoise.wav"))
                fileSound.Play(windowPose.position);
            UI.Label("Generated Sound:");
            UI.Label("Duration"); UI.SameLine();
            UI.HSlider("Duration", ref genDuration, 0.1f, 2, 0, UI.AreaRemaining.x);
            if (genSound != null && UI.Button("Play"))
                genSound.Play(windowPose.position);
            UI.WindowEnd();

            boundsMesh.Draw(boundsMat, Matrix.TS(genVolume.center, genVolume.dimensions));

            Hand hand = Input.Hand(Handed.Right);
            bool contains = genVolume.Contains(hand[FingerId.Index, JointId.Tip].position);
            if (contains && !genPrevContains)
                genPath.Clear();
            if (contains) {
                Vec3 pt = hand[FingerId.Index, JointId.Tip].position - genVolume.center;
                if (genPath.Count == 0 || Vec3.DistanceSq(pt, genPath[genPath.Count-1].pt) > 0.0001f)
                genPath.Add(new LinePoint(pt, Color.White, 0.01f));
            }

            if (!contains && genPrevContains)
            {
                genSound = Sound.Generate((t) =>
                {
                    float sampleAt = (t / genDuration) * (genPath.Count-1);
                    float pct = sampleAt - (int)sampleAt;
                    int s1 = (int)(sampleAt);
                    int s2 = (int)System.Math.Ceiling(sampleAt);
                    Vec3 sample = (Vec3.Lerp(genPath[s1].pt, genPath[s2].pt, pct) + genVolume.dimensions / 2) / genVolume.dimensions.x;

                    float band1 = SKMath.Sin(t * (600 + sample.x * 4000) * 6.28f) * sample.y;
                    float band2 = SKMath.Sin(t * (100 + sample.z * 600) * 6.28f) * sample.y;

                    return band1*0.45f + band2*0.45f;
                }, genDuration);
                genSound.Play(genVolume.center);
            }
            Lines.Add(genPath.ToArray());
            genPrevContains = contains;
        }
    }
}
