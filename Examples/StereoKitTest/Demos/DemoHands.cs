/// :CodeDoc: Guides Using Hands
/// # Using Hands
/// 
/// StereoKit uses a hands first approach to user input! Even when hand-sensors
/// aren't available, hand data is simulated instead using existing devices!
/// For example, Windows Mixed Reality controllers will blend between pre-recorded
/// hand poses based on button presses, as will mice. This way, fully articulated 
/// hand data is always present for you to work with!
/// 
/// :End:

using StereoKit;
using StereoKit.Framework;
using System;

namespace StereoKitTest
{
    class DemoHands : ITest
    {
        static Pose optionsPose = new Pose(0,0,-0.3f, Quat.LookDir(-Vec3.Forward));
        bool showHands = true;
        bool showJoints = false;
        bool showAxes = true;
        bool showPointers = true;
        bool showHandMenus = true;

        Mesh jointMesh = Mesh.GenerateSphere(1);
        HandMenuRadial handMenu;

        public void Initialize() {
            /// :CodeDoc: Guides Using Hands
            /// ## Accessing Joints
            /// 
            /// ![Hand with joints]({{site.url}}/img/screenshots/HandAxes.jpg)
            /// 
            /// Since hands are so central to interaction, accessing hand information needs
            /// to be really easy to get! So here's how you might find the fingertip of the right
            /// hand! If you ignore IsTracked, this'll give you the last known position for that
            /// finger joint.
            Hand hand = Input.Hand(Handed.Right);
            if (hand.IsTracked)
            { 
                Vec3 fingertip = hand[FingerId.Index, JointId.Tip].position;
            }
            /// Pretty straightforward! And if you prefer calling a function instead of using the
            /// [] operator, that's cool too! You can call `hand.Get(FingerId.Index, JointId.Tip)`
            /// instead!
            /// 
            /// If that's too granular for you, there's easy ways to check for pinching and 
            /// gripping! Pinched will tell you if a pinch is currently happening, JustPinched
            /// will tell you if it just started being pinched this frame, and JustUnpinched will
            /// tell you if the pinch just stopped this frame!
            if (hand.IsPinched) { }
            if (hand.IsJustPinched) { }
            if (hand.IsJustUnpinched) { }
            
            if (hand.IsGripped) { }
            if (hand.IsJustGripped) { }
            if (hand.IsJustUngripped) { }
            /// These are all convenience functions wrapping the `hand.pinchState` bit-flag, so you
            /// can also use that directly if you want to do some bit-flag wizardry!
            /// :End:

            /// :CodeSample: HandMenuRadial HandRadialLayer HandMenuItem
            /// ### Basic layered hand menu
            /// 
            /// The HandMenuRadial is an `IStepper`, so it should be registered with 
            /// `StereoKitApp.AddStepper` so it can run by itself! It's recommended to
            /// keep track of it anyway, so you can remove it when you're done with it
            /// via `StereoKitApp.RemoveStepper`
            /// 
            /// The constructor uses a params style argument list that makes it easy and
            /// clean to provide lists of items! This means you can assemble the whole
            /// menu on a single 'line'. You can still pass arrays instead if you prefer
            /// that!
            handMenu = StereoKitApp.AddStepper(new HandMenuRadial(
                new HandRadialLayer("Root",
                    new HandMenuItem("File",   null, null, "File"),
                    new HandMenuItem("Edit",   null, null, "Edit"),
                    new HandMenuItem("About",  null, () => Log.Info(StereoKitApp.VersionName)),
                    new HandMenuItem("Cancel", null, null)),
                new HandRadialLayer("File", 
                    new HandMenuItem("New",   null, () => Log.Info("New")),
                    new HandMenuItem("Open",  null, () => Log.Info("Open")),
                    new HandMenuItem("Close", null, () => Log.Info("Close")),
                    new HandMenuItem("Back",  null, null, HandMenuAction.Back)),
                new HandRadialLayer("Edit",
                    new HandMenuItem("Copy",  null, () => Log.Info("Copy")),
                    new HandMenuItem("Paste", null, () => Log.Info("Paste")),
                    new HandMenuItem("Back", null, null, HandMenuAction.Back))));
            /// :End:

            Tests.Hand(new HandJoint[] { new HandJoint(new Vec3(-0.478f, -0.201f, -0.360f), new Quat(-0.759f, -0.374f, 0.009f, -0.533f), 0.004f), new HandJoint(new Vec3(-0.478f, -0.201f, -0.360f), new Quat(-0.759f, -0.374f, 0.009f, -0.533f), 0.010f), new HandJoint(new Vec3(-0.508f, -0.183f, -0.337f), new Quat(-0.721f, -0.500f, -0.200f, -0.435f), 0.009f), new HandJoint(new Vec3(-0.532f, -0.170f, -0.331f), new Quat(-0.635f, -0.456f, -0.281f, -0.556f), 0.008f), new HandJoint(new Vec3(-0.546f, -0.165f, -0.336f), new Quat(0.458f, 0.358f, 0.393f, 0.713f), 0.006f), new HandJoint(new Vec3(-0.463f, -0.188f, -0.367f), new Quat(-0.363f, -0.671f, 0.299f, -0.573f), 0.004f), new HandJoint(new Vec3(-0.500f, -0.134f, -0.356f), new Quat(-0.363f, -0.671f, 0.299f, -0.573f), 0.009f), new HandJoint(new Vec3(-0.514f, -0.102f, -0.351f), new Quat(-0.412f, -0.633f, 0.365f, -0.544f), 0.008f), new HandJoint(new Vec3(-0.526f, -0.087f, -0.348f), new Quat(-0.329f, -0.679f, 0.271f, -0.598f), 0.007f), new HandJoint(new Vec3(-0.534f, -0.080f, -0.347f), new Quat(-0.275f, -0.700f, 0.210f, -0.625f), 0.005f), new HandJoint(new Vec3(-0.464f, -0.185f, -0.377f), new Quat(-0.286f, -0.657f, 0.375f, -0.588f), 0.004f), new HandJoint(new Vec3(-0.499f, -0.133f, -0.376f), new Quat(-0.286f, -0.657f, 0.375f, -0.588f), 0.009f), new HandJoint(new Vec3(-0.514f, -0.096f, -0.378f), new Quat(-0.335f, -0.598f, 0.449f, -0.574f), 0.008f), new HandJoint(new Vec3(-0.530f, -0.079f, -0.378f), new Quat(-0.222f, -0.669f, 0.331f, -0.627f), 0.007f), new HandJoint(new Vec3(-0.541f, -0.073f, -0.378f), new Quat(-0.122f, -0.711f, 0.224f, -0.655f), 0.005f), new HandJoint(new Vec3(-0.467f, -0.185f, -0.388f), new Quat(-0.241f, -0.616f, 0.425f, -0.618f), 0.003f), new HandJoint(new Vec3(-0.498f, -0.139f, -0.395f), new Quat(-0.241f, -0.616f, 0.425f, -0.618f), 0.008f), new HandJoint(new Vec3(-0.516f, -0.107f, -0.402f), new Quat(-0.259f, -0.580f, 0.460f, -0.621f), 0.007f), new HandJoint(new Vec3(-0.532f, -0.092f, -0.405f), new Quat(-0.156f, -0.645f, 0.361f, -0.655f), 0.006f), new HandJoint(new Vec3(-0.543f, -0.086f, -0.406f), new Quat(-0.085f, -0.680f, 0.291f, -0.668f), 0.004f), new HandJoint(new Vec3(-0.472f, -0.190f, -0.398f), new Quat(-0.188f, -0.577f, 0.512f, -0.608f), 0.003f), new HandJoint(new Vec3(-0.499f, -0.147f, -0.412f), new Quat(-0.188f, -0.577f, 0.512f, -0.608f), 0.007f), new HandJoint(new Vec3(-0.512f, -0.124f, -0.423f), new Quat(-0.184f, -0.520f, 0.545f, -0.631f), 0.006f), new HandJoint(new Vec3(-0.524f, -0.114f, -0.427f), new Quat(-0.062f, -0.612f, 0.439f, -0.655f), 0.005f), new HandJoint(new Vec3(-0.533f, -0.108f, -0.429f), new Quat(-0.022f, -0.637f, 0.401f, -0.658f), 0.004f), new HandJoint(new Vec3(-0.483f, -0.165f, -0.383f), new Quat(-0.294f, 0.737f, 0.063f, 0.605f), 0.000f), new HandJoint(new Vec3(-0.483f, -0.165f, -0.383f), new Quat(-0.294f, 0.737f, 0.063f, 0.605f), 0.000f) });
            Tests.Screenshot(600, 600, "HandAxes.jpg", new Vec3(-0.370f, -0.020f, -0.273f), new Vec3(-1.095f, -0.610f, -0.629f));
        }

        public void Shutdown()
        {
            /// :CodeSample: HandMenuRadial HandRadialLayer HandMenuItem
            StereoKitApp.RemoveStepper(handMenu); 
            /// :End:
        }

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
            UI.SameLine();
            UI.Toggle("Pointers", ref showPointers);
            UI.SameLine();
            UI.Toggle("Menu", ref showHandMenus);
            UI.WindowEnd();

            if (showJoints)   DrawJoints(jointMesh, Default.Material);
            if (showAxes)     DrawAxes();
            if (showPointers) DrawPointers();
            if (showHandMenus) 
            { 
                DrawHandMenu(Handed.Right);
                DrawHandMenu(Handed.Left);
            }
        }

        /// :CodeDoc: Guides Using Hands
        /// ## Hand Menu
        /// 
        /// Lets imagine you want to make a hand menu, you might need to know 
        /// if the user is looking at the palm of their hand! Here's a quick 
        /// example of using the palm's pose and the dot product to determine 
        /// this.
        static bool HandFacingHead(Handed handed)
        {
            Hand hand = Input.Hand(handed);
            if (!hand.IsTracked)
                return false;

            Vec3 palmDirection   = (hand.palm.Forward).Normalized();
            Vec3 directionToHead = (Input.Head.position - hand.palm.position).Normalized();

            return Vec3.Dot(palmDirection, directionToHead) > 0.5f;
        }
        /// Once you have that information, it's simply a matter of placing a
        /// window off to the side of the hand! The palm pose Right direction
        /// points to different sides of each hand, so a different X offset
        /// is required for each hand.
        public static void DrawHandMenu(Handed handed)
        {
            if (!HandFacingHead(handed))
                return;

            // Decide the size and offset of the menu
            Vec2  size   = new Vec2(4, 16);
            float offset = handed == Handed.Left ? -4-size.x : 6+size.x;

            // Position the menu relative to the palm
            Pose menuPose = Input.Hand(handed).palm;
            menuPose.position += menuPose.Right * offset * Units.cm2m;
            menuPose.position += menuPose.Up * (size.y/2) * Units.cm2m;

            // And make a menu!
            UI.WindowBegin("HandMenu", ref menuPose, size * Units.cm2m, false);
            UI.Button("Test");
            UI.Button("That");
            UI.Button("Hand");
            UI.WindowEnd();
        }
        /// :End:

        public static void DrawAxes()
        {
            for (int i = 0; i < (int)Handed.Max; i++) {
                Hand hand = Input.Hand((Handed)i);
                if (!hand.IsTracked)
                    continue;

                for (int finger = 0; finger < 5; finger++) {
                for (int joint  = 0; joint  < 5; joint++ ) {
                    Lines.AddAxis(hand[finger, joint].Pose);
                }}
                Lines.AddAxis(hand.palm);
            }
        }

        public static void DrawJoints(Mesh jointMesh, Material jointMaterial)
        {
            for (int i = 0; i < (int)Handed.Max; i++) {
                Hand hand = Input.Hand((Handed)i);
                if (!hand.IsTracked)
                    continue;

                for (int finger = 0; finger < 5; finger++) {
                for (int joint  = 0; joint  < 5; joint++ ) {
                    HandJoint current = hand[finger, joint];
                    jointMesh.Draw(jointMaterial, Matrix.TRS(current.position, current.orientation, current.radius/2));
                } }
            }
        }

        /// :CodeDoc: Guides Using Hands
        /// ## Pointers
        /// 
        /// And lastly, StereoKit also has a pointer system! This applies to
        /// more than just hands. Head, mouse, and other devices will also
        /// create pointers into the scene. You can filter pointers based on
        /// source family and device capabilities, so this is a great way to 
        /// abstract a few more input sources nicely!
        public static void DrawPointers()
        {
            int hands = Input.PointerCount(InputSource.Hand);
            for (int i = 0; i < hands; i++)
            {
                Pointer pointer = Input.Pointer(i, InputSource.Hand);
                Lines.Add    (pointer.ray, 0.5f, Color.White, Units.mm2m);
                Lines.AddAxis(pointer.Pose);
            }
        }
        /// :End:
    }
}

/// :CodeDoc: Guides Using Hands
/// The code in context for this document can be found [on Github here](https://github.com/maluoi/StereoKit/blob/master/Examples/StereoKitTest/DemoHands.cs)!
/// :End: