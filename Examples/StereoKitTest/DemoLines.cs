using StereoKit;
using System;
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

        Vec3 prevTip;
        List<LinePoint> drawPoints = new List<LinePoint>();
        void Draw(Handed handed)
        {
            Hand hand = Input.Hand(handed);
            Vec3 tip  = hand[FingerId.Index, JointId.Tip].position;
            tip = prevTip + (tip-prevTip) * 0.3f;
            const float size    = 0.03f;
            const float minDist = 2 * Units.cm2m;

            if (hand.IsJustPinched) { 
                drawPoints.Clear();
                drawPoints.Add(new LinePoint(tip, Color.White, size));
                drawPoints.Add(new LinePoint(tip, Color.White, size));
                prevTip = tip;
            }

            if (hand.IsPinched)
            {
                Vec3  prev  = drawPoints[drawPoints.Count - 2].pt;
                Vec3  dir   = (prev - (drawPoints.Count > 2 ? drawPoints[drawPoints.Count - 3].pt : drawPoints[drawPoints.Count - 1].pt)).Normalized();
                float dist  = (prev - tip).Magnitude;
                float speed = (tip - prevTip).Magnitude * Time.Elapsedf;
                LinePoint here = new LinePoint(tip, Color.White, Math.Max(1-speed/0.0003f,0.1f) * size);
                drawPoints[drawPoints.Count - 1]  = here;
                
                if ((Vec3.Dot( dir, (tip-prev).Normalized() ) < 0.99f && dist > 0.005f) || dist > 0.3f) { 
                    drawPoints.Add(here);
                }
            }

            Lines.Add(drawPoints.ToArray());
            prevTip = tip;
        }
    }
}
