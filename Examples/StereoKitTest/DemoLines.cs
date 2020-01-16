using StereoKit;
using System;
using System.Collections.Generic;

namespace StereoKitTest
{
    class DemoLines : IDemo
    {
        Pose windowPose = new Pose(Vec3.Zero, Quat.Identity);
        Model paletteModel = Model.FromFile("Palette.glb");
        Pose palettePose = new Pose(Vec3.Zero, Quat.Identity);
        Color activeColor = Color.White;
        Material handMaterial;

        public void Initialize() {
            handMaterial = Material.Find(DefaultIds.materialHand);
            }
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

            
            UI.WindowBegin("Settings", ref windowPose, new Vec2(20,0)*Units.cm2m);
            if (UI.Button("Clear")) {
                drawList  .Clear();
                drawPoints.Clear();
            }
            UI.WindowEnd();

            DrawMenu();
            Draw(Handed.Right);
        }

        void DrawMenu()
        {
            UI.AffordanceBegin("PaletteMenu", ref palettePose, paletteModel.Bounds);
            paletteModel.Draw(Matrix.Identity);

            if (UI.VolumeAt("White", new Bounds(new Vec3(4, 0, 7) * Units.cm2m, new Vec3(4,2,4) * Units.cm2m)))
                SetColor(Color.White);
            if (UI.VolumeAt("Red",   new Bounds(new Vec3(9, 0, 3) * Units.cm2m, new Vec3(4,2,4) * Units.cm2m)))
                SetColor(new Color(1,0,0));
            if (UI.VolumeAt("Green", new Bounds(new Vec3(9, 0,-3) * Units.cm2m, new Vec3(4,2,4) * Units.cm2m)))
                SetColor(new Color(0,1,0));
            if (UI.VolumeAt("Blue",  new Bounds(new Vec3(3, 0,-6) * Units.cm2m, new Vec3(4,2,4) * Units.cm2m)))
                SetColor(new Color(0,0,1));

            UI.AffordanceEnd();
        }
        void SetColor(Color color)
        {
            activeColor = color;
            handMaterial["color"] = color;
        }

        Vec3 prevTip;
        bool painting = false;
        List<LinePoint> drawPoints = new List<LinePoint>();
        List<LinePoint[]> drawList = new List<LinePoint[]>();
        void Draw(Handed handed)
        {
            Hand hand = Input.Hand(handed);
            Vec3 tip  = hand[FingerId.Index, JointId.Tip].position;
            tip = prevTip + (tip-prevTip) * 0.3f;
            const float size    = 0.03f;
            const float minDist = 2 * Units.cm2m;

            if (hand.IsJustPinched && !UI.IsInteracting(handed)) { 
                if (drawPoints.Count > 0)
                    drawList.Add(drawPoints.ToArray());
                drawPoints.Clear();
                drawPoints.Add(new LinePoint(tip, activeColor, size));
                drawPoints.Add(new LinePoint(tip, activeColor, size));
                prevTip = tip;
                painting = true;
            }
            if (hand.IsJustUnpinched)
                painting = false;

            if (painting && drawPoints.Count > 1)
            {
                Vec3  prev  = drawPoints[drawPoints.Count - 2].pt;
                Vec3  dir   = (prev - (drawPoints.Count > 2 ? drawPoints[drawPoints.Count - 3].pt : drawPoints[drawPoints.Count - 1].pt)).Normalized();
                float dist  = (prev - tip).Magnitude;
                float speed = (tip - prevTip).Magnitude * Time.Elapsedf;
                LinePoint here = new LinePoint(tip, activeColor, Math.Max(1-speed/0.0003f,0.1f) * size);
                drawPoints[drawPoints.Count - 1]  = here;
                
                if ((Vec3.Dot( dir, (tip-prev).Normalized() ) < 0.99f && dist > 0.01f) || dist > 0.05f) { 
                    drawPoints.Add(here);
                }
            }

            Lines.Add(drawPoints.ToArray());
            for (int i = 0; i < drawList.Count; i++)
                Lines.Add(drawList[i]);
            prevTip = tip;
        }
    }
}
