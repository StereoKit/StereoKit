// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2024 Nick Klingensmith
// Copyright (c) 2024 Qualcomm Technologies, Inc.

using StereoKit;

internal class TestCustomButton : ITest
{
	public void Initialize() {}
	public void Shutdown  () {}

	Pose windowPose = new Pose(0, 0, -0.5f, Quat.FromAngles(0, 180, 0));
	public void Step()
	{
		UI.WindowBegin("Custom UI Elements", ref windowPose, V.XY(0.2f,0));
		CustomButtonMesh   ("Custom Button Mesh");
		CustomButtonElement("Custom Button Element");
		UI.Button          ("Standard Button");

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
}