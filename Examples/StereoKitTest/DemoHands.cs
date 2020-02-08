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
    class DemoHands : IDemo
    {
        static Pose optionsPose = new Pose(0,0,-0.3f, Quat.LookDir(-Vec3.Forward));
        bool showHands = true;
        bool showJoints = false;
        bool showAxes = true;
        bool showPointers = true;

        Mesh jointMesh = Mesh.GenerateSphere(1);
        HandMenuRadial handMenu;

        public void Initialize() {
            /// :CodeDoc: Guides Using Hands
            /// ## Accessing Joints
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
        }

        public void Shutdown() { StereoKitApp.RemoveStepper(handMenu); }

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
            UI.WindowEnd();

            if (showJoints)   DrawJoints(jointMesh, Default.Material);
            if (showAxes)     DrawAxes();
            if (showPointers) DrawPointers();

            DrawHandMenu(Handed.Right);
            DrawHandMenu(Handed.Left);
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