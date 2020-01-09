

using StereoKit;
using System.Collections.Generic;

namespace StereoKitTest
{
    class DemoLines : IDemo
    {
        public void Initialize() {}
        public void Shutdown() { }

        public void Update()
        {
            /// :CodeSample: Lines.Add
            Lines.Add(new Vec3(0.1f,0,0), new Vec3(-0.1f,0,0), Color.White, 0.01f);
            /// :End:
            
            Hierarchy.Push(Matrix.T(0,0.05f,0));
            /// :CodeSample: Lines.Add
            Lines.Add(new Vec3(0.1f,0,0), new Vec3(-0.1f,0,0), Color.White, Color.Black, 0.01f);
            /// :End:
            Hierarchy.Pop();

            Hierarchy.Push(Matrix.T(0, 0.1f, 0));
            /// :CodeSample: Lines.Add
            Lines.Add(new LinePoint[]{ 
                new LinePoint(new Vec3( 0.1f, 0, 0), Color.White, 0.01f),
                new LinePoint(new Vec3( 0,    0.02f, 0), Color.Black, 0.005f),
                new LinePoint(new Vec3(-0.1f, 0, 0), Color.White, 0.01f),
            });
            /// :End:
            Hierarchy.Pop();

            Draw(Handed.Right);
        }

        List<LinePoint> drawPoints = new List<LinePoint>();
        void Draw(Handed handed)
        {
            Hand hand = Input.Hand(handed);
            Vec3 tip  = hand[FingerId.Index, JointId.Tip].position;
            const float size    = 0.03f;
            const float minDist = 2 * Units.cm2m;

            if (hand.IsJustPinched) { 
                drawPoints.Clear();
                drawPoints.Add(new LinePoint(tip, Color.White, size));
            }

            if (hand.IsPinched)
            {
                Vec3  prev = drawPoints[drawPoints.Count-1].pt;
                float dist = (prev-tip).Magnitude;// Vec3.Distance(prev, tip);
                Log.Info(dist.ToString());
                if (dist > minDist) { 
                    drawPoints.Add(new LinePoint(tip, Color.White, (minDist/dist) * size));
                }
            }

            Lines.Add(drawPoints.ToArray());
        }
    }
}
