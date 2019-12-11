using StereoKit;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StereoKitTest
{
    class DemoHands : IDemo
    {
        static Pose optionsPose = new Pose(0,0,-0.3f, Quat.LookDir(-Vec3.Forward));
        bool showHands = true;
        bool showJoints = false;
        bool showAxes = true;

        Mesh     jointMesh     = Mesh.GenerateSphere(1);
        Material jointMaterial = Material.Find(DefaultIds.material);

        public void Initialize() {}

        public void Shutdown() { }

        public void Update()
        {
            UI.WindowBegin("Options", ref optionsPose, new Vec2(24, 0)*Units.cm2m);
            UI.Label("Show");
            if (UI.Toggle("Hands", ref showHands))
                Input.HandVisible(Handed.Max, showHands);
            UI.SameLine();
            UI.Toggle("Joints", ref showJoints);
            UI.SameLine();
            UI.Toggle("Axes", ref showAxes);
            UI.WindowEnd();

            if (showJoints)
                DrawJoints();

            if (showAxes)
            {
                if (Input.Hand(Handed.Right).IsTracked)
                    Lines.AddAxis(Input.Hand(Handed.Right).palm);
                if (Input.Hand(Handed.Left).IsTracked)
                    Lines.AddAxis(Input.Hand(Handed.Left).palm);
            }

            DrawHandMenu();
        }

        public void DrawJoints()
        {
            for (int i = 0; i < (int)Handed.Max; i++) {
                Hand hand = Input.Hand((Handed)i);
                for (int f = 0; f < 5; f++) {
                for (int j = 0; j < 5; j++) {
                    HandJoint joint = hand[f, j];
                    jointMesh.Draw(jointMaterial, Matrix.TRS( joint.position, joint.orientation, joint.size/2));
                } }
            }
        }

        bool handMenuShow = false;
        Handed handMenuHand;
        public void DrawHandMenu()
        {
            if (handMenuShow && HandFacing(handMenuHand))
            {
                Pose handPose = Input.Hand(handMenuHand).palm;
                Hierarchy.Push(handPose.ToMatrix());
                Pose handMenuPose = new Pose((handMenuHand == Handed.Left ? -8 : 10)*Units.cm2m,8*Units.cm2m,0,Quat.Identity);
                UI.WindowBegin("HandMenu", ref handMenuPose, new Vec2(4,16) * Units.cm2m, false);
                UI.Button("Test");
                UI.Button("That");
                UI.Button("Hand");
                UI.WindowEnd();
                Hierarchy.Pop();
            }
            else
            {
                handMenuShow = false;
                if (HandFacing(Handed.Left)) 
                {
                    handMenuShow = true;
                    handMenuHand = Handed.Left;
                }
                else if (HandFacing(Handed.Right))
                {
                    handMenuShow = true;
                    handMenuHand = Handed.Right;
                }
            }
        }
        static bool HandFacing(Handed handed)
        {
            Hand hand = Input.Hand(handed);
            if (!hand.IsTracked)
                return false;

            Vec3 palmDir = (hand.palm.orientation * Vec3.Forward).Normalized();
            Vec3 headDir = (Input.Head.position - hand.palm.position).Normalized();

            return Vec3.Dot(palmDir, headDir) > 0.5f;
        }
    }
}
