// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2025 Nick Klingensmith
// Copyright (c) 2023-2025 Qualcomm Technologies, Inc.

using StereoKit;
using StereoKit.Framework;

class DemoMixedReality : ITest
{
	string title       = "Mixed Reality";
	string description = "You can set up AR with OpenXR by changing the environment blend mode! In StereoKit, this is modifiable via Device.DisplayBlend at runtime, and SKSettings.blendPreference during initialization.\n\nNote that some devices may not support each blend mode! Like a HoloLens can't be Opaque, and some VR headsets can't be transparent!";

	Pose             windowPose = Demo.contentPose.Pose;
	PassthroughFBExt passthrough;

	public void Initialize()
	{
		passthrough = SK.GetStepper<PassthroughFBExt>();

		World.RefreshType     = WorldRefresh.Timer;
		World.RefreshInterval = 2;
		World.RefreshRadius   = 5;
	}

	public void Shutdown() { }

	public void Step()
	{
		UI.WindowBegin("Mixed Reality Settings", ref windowPose);

		UI.Label("Device.DisplayBlend");
		DisplayBlend currBlend = Device.DisplayBlend;

		// Radio buttons for switching between any blend modes that are
		// available on this headset!

		UI.PushEnabled(Device.ValidBlend(DisplayBlend.Opaque));
		if (UI.Radio("Opaque", currBlend == DisplayBlend.Opaque))
			Device.DisplayBlend = DisplayBlend.Opaque;
		UI.PopEnabled();
		UI.SameLine();

		UI.PushEnabled(Device.ValidBlend(DisplayBlend.Blend));
		if (UI.Radio("Blend", currBlend == DisplayBlend.Blend))
			Device.DisplayBlend = DisplayBlend.Blend;
		UI.PopEnabled();
		UI.SameLine();

		UI.PushEnabled(Device.ValidBlend(DisplayBlend.Additive));
		if (UI.Radio("Additive", currBlend == DisplayBlend.Additive))
			Device.DisplayBlend = DisplayBlend.Additive;
		UI.PopEnabled();

		// If we have the Facebook Passthrough extension turned on, show some
		// options for that as well! This extension has a few additional
		// options that goes beyond the simple blend mode.
		if (passthrough != null)
		{
			UI.HSeparator();
			bool toggle = passthrough.Enabled;
			UI.Label(passthrough.Available
				? "FB Passthrough EXT available!"
				: "No FB passthrough EXT available :(");
			UI.PushEnabled(passthrough.Available);
			if (UI.Toggle("Passthrough", ref toggle))
				passthrough.Enabled = toggle;
			UI.PopEnabled();
		}

		// World occlusion controls
		UI.HSeparator();
		OcclusionCaps available = World.OcclusionCapabilities;
		UI.Label(available == OcclusionCaps.None ? "Occlusion isn't available on this system" : "World.Occlusion" );
		UI.PushEnabled(available.HasFlag(OcclusionCaps.Depth));
		bool depthOcclusion = World.Occlusion.HasFlag(OcclusionCaps.Depth);
		if (UI.Toggle("OcclusionCaps.Depth", ref depthOcclusion))
		{
			if (depthOcclusion) World.Occlusion |=  OcclusionCaps.Depth;
			else                World.Occlusion &= ~OcclusionCaps.Depth;
		}
		UI.PopEnabled();
		UI.SameLine();

		UI.PushEnabled(available.HasFlag(OcclusionCaps.Hands));
		bool handOcclusion = World.Occlusion.HasFlag(OcclusionCaps.Hands);
		if (UI.Toggle("OcclusionCaps.Hand", ref handOcclusion))
		{
			if (handOcclusion) World.Occlusion |=  OcclusionCaps.Hands;
			else               World.Occlusion &= ~OcclusionCaps.Hands;
		}
		UI.PopEnabled();

		UI.PushEnabled(available.HasFlag(OcclusionCaps.Mesh));
		bool meshOcclusion = World.Occlusion.HasFlag(OcclusionCaps.Mesh);
		if (UI.Toggle("OcclusionCaps.Mesh", ref meshOcclusion))
		{
			if (meshOcclusion) World.Occlusion |=  OcclusionCaps.Mesh;
			else               World.Occlusion &= ~OcclusionCaps.Mesh;
		}
		UI.PopEnabled();

		// Raycasting controls
		UI.HSeparator();
		UI.Label(SK.System.worldRaycastPresent ? "World.RaycastEnabled" : "World raycasting isn't available on this system");
		UI.PushEnabled(SK.System.worldRaycastPresent);

		bool raycast = World.RaycastEnabled;
		if (UI.Toggle("Enable Raycast", ref raycast))
			World.RaycastEnabled = raycast;

		UI.PopEnabled();

		UI.WindowEnd();

		// Raycast finger visualization
		for (int i = 0; i < 2; i++)
		{
			Hand hand = Input.Hand(i);
			if (!hand.IsTracked) continue;

			Ray fingerRay = hand[FingerId.Index, JointId.Tip].Pose.Ray;
			if (World.Raycast(fingerRay, out Ray at))
				Mesh.Sphere.Draw(Material.Default, Matrix.TS(at.position, 0.03f), new Color(1, 0, 0));
		}

		/// :CodeSample: World.HasBounds World.BoundsSize World.BoundsPose
		// Here's some quick and dirty lines for the play boundary rectangle!
		if (World.HasBounds)
		{
			Vec2   s    = World.BoundsSize/2;
			Matrix pose = World.BoundsPose.ToMatrix();
			Vec3   tl   = pose.Transform( new Vec3( s.x, 0,  s.y) );
			Vec3   br   = pose.Transform( new Vec3(-s.x, 0, -s.y) );
			Vec3   tr   = pose.Transform( new Vec3(-s.x, 0,  s.y) );
			Vec3   bl   = pose.Transform( new Vec3( s.x, 0, -s.y) );

			Lines.Add(tl, tr, Color.White, 1.5f*U.cm);
			Lines.Add(bl, br, Color.White, 1.5f*U.cm);
			Lines.Add(tl, bl, Color.White, 1.5f*U.cm);
			Lines.Add(tr, br, Color.White, 1.5f*U.cm);
		}
		/// :End:

		Demo.ShowSummary(title, description,
			new Bounds(V.XY0(0, -0.03f), new Vec3(.38f, .34f, 0)));
	}
}
