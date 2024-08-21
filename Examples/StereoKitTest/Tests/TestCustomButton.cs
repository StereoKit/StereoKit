// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2024 Nick Klingensmith
// Copyright (c) 2024 Qualcomm Technologies, Inc.

using StereoKit;
using System;

internal class TestCustomButton : ITest
{
	public void Initialize() {}
	public void Shutdown  () {}

	Pose windowPose = new Pose(0, 0, -0.5f, Quat.FromAngles(0, 180, 0));
	public void Step()
	{
		UI.WindowBegin("Custom UI Elements", ref windowPose, V.XY(0.2f,0));

		CustomButtonMesh("Custom Button Mesh");
		Tooltip("Small tip");

		CustomButtonElement("Custom Button Element");
		Tooltip("Larger tooltip for a custom button!");

		UI.Button("Standard Button");
		Tooltip("Very standard");

		UI.PushEnabled(false);
		CustomButtonMesh("Custom Button Disabled");
		UI.PopEnabled();

		UI.PushTint(Color.HSV(0,0.2f,0.7f));
		CustomButtonElement("Custom Button Tinted");
		UI.PopTint();
		UI.WindowEnd();

		Tests.Screenshot("Tests/CustomButtons.jpg", 400, 400, windowPose.position + new Vec3(0, -0.1f, .15f), windowPose.position + new Vec3(0, -0.1f, 0));
	}

	static bool CustomButtonMesh(string text)
	{
		ulong id = UI.StackHash(text);

		Vec2   size   = Text.Size(text, UI.TextStyle) + UI.Settings.padding * 2;
		Bounds layout = UI.LayoutReserve(size);
		UI.ButtonBehavior(layout.TLC, size, text, out float offset, out BtnState state, out BtnState focus, out int hand);

		layout.center    .z -= offset / 2;
		layout.dimensions.z  = offset;
		Mesh.Cube.Draw(Material.UI, Matrix.TS(layout.center, layout.dimensions), UI.GetElementColor(UIVisual.Button, UI.GetAnimFocus(id, focus, state)));
		Text.Add(text, Matrix.T(layout.center.x, layout.center.y, -(offset + 0.002f)), UI.TextStyle, TextAlign.Center);

		return state.IsJustInactive();
	}

	static bool CustomButtonElement(string text)
	{
		ulong id = UI.StackHash(text);

		Vec2   size   = Text.Size(text, UI.TextStyle) + UI.Settings.padding * 2;
		Bounds layout = UI.LayoutReserve(size);
		UI.ButtonBehavior(layout.TLC, size, text, out float offset, out BtnState state, out BtnState focus, out int hand);

		layout.center.z    -= offset / 2;
		layout.dimensions.z = offset;
		UI.DrawElement(UIVisual.Button, layout.TLB, layout.dimensions, UI.GetAnimFocus(id, focus, state));
		Text.Add(text, Matrix.T(layout.center.x, layout.center.y, -(offset + 0.002f)), UI.TextStyle, TextAlign.Center);

		return state.IsJustInactive();
	}

	static void Tooltip(string text)
	{
		if (!UI.LastElementFocused.IsActive()) return;

		Bounds     parent = UI.LayoutLast;
		UISettings s      = UI.Settings;
		float      size   = Math.Min(parent.dimensions.x, Text.Size(text, UI.TextStyle).x + s.padding*2);

		UI.LayoutPush(new Vec3(parent.center.x + size/2, parent.center.y-parent.dimensions.y/2 - s.gutter, -s.depth*1.5f ), new Vec2(size, 0));
		UI.PanelBegin(UIPad.Inside);
		UI.Text(text);
		UI.PanelEnd();
		UI.LayoutPop();
	}
}