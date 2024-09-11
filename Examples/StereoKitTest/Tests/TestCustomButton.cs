// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2024 Nick Klingensmith
// Copyright (c) 2024 Qualcomm Technologies, Inc.

using StereoKit;
using System;

internal class TestCustomButton : ITest
{
	static class CustomUIVisual
	{
		public const UIVisual Button      = UIVisual.Max + 1;
		public const UIVisual Nonexistent = UIVisual.Max + 2; // We intentionally don't set this UIVisual up for testing purposes
	}
	static class CustomUIColor
	{
		public const UIColor Extra       = UIColor.Max + 1;
		public const UIColor Nonexistent = UIColor.Max + 2; // We intentionally don't set this UIColor up for testing purposes
	}

	public void Initialize() {
		UI.SetElementVisual(CustomUIVisual.Button, Mesh.Cube, Material.Unlit);
		UI.SetElementColor (CustomUIVisual.Button, CustomUIColor.Extra);
		UI.SetThemeColor   (CustomUIColor.Extra, Color.HSV(0.33f, 0.3f, 0.5f));
	}

	public void Shutdown  () {}

	Pose windowPose = new Pose(0, 0, -0.5f, Quat.FromAngles(0, 180, 0));
	public void Step()
	{
		UI.WindowBegin("Custom UI Elements", ref windowPose, V.XY(0.2f,0));

		CustomButtonMesh("Custom Button Mesh");
		Tooltip("This button manually draws its own mesh, but uses SK's theme coloring/animation system.");

		CustomButtonElement("Custom Button Element");
		Tooltip("This button uses SK's themed UI drawing API that allows the visual to be completely driven by the theme.");

		CustomButtonTheme("Extended Theme Element");
		Tooltip("This uses extended theming slots, which are available for adding custom elements to SK's theming system.");

		CustomButtonNonexistentTheme("Nonexistent Theme");
		Tooltip("This uses a theme slot that hasn't been initialized. In this case, it should fall back to a default instead of crashing.");

		UI.Button("Standard Button");
		Tooltip("Small tip");

		UI.PushEnabled(false);
		CustomButtonMesh("Custom Button Disabled");
		UI.PopEnabled();

		UI.PushTint(Color.HSV(0,0.2f,0.7f));
		CustomButtonElement("Custom Button Tinted");
		UI.PopTint();
		UI.WindowEnd();

		float y = UI.LayoutLast.center.y;
		Tests.Screenshot("Tests/CustomButtons.jpg", 400, 400, windowPose.position + new Vec3(0, y, .18f), windowPose.position + new Vec3(0, y, 0));
	}

	static bool CustomButtonMesh(string text)
	{
		ulong id = UI.StackHash(text);

		Vec2   size   = Text.SizeLayout(text, UI.TextStyle) + UI.Settings.padding * 2;
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

		Vec2   size   = Text.SizeLayout(text, UI.TextStyle) + UI.Settings.padding * 2;
		Bounds layout = UI.LayoutReserve(size);
		UI.ButtonBehavior(layout.TLC, size, text, out float offset, out BtnState state, out BtnState focus, out int hand);

		layout.center.z    -= offset / 2;
		layout.dimensions.z = offset;
		UI.DrawElement(UIVisual.Button, layout.TLB, layout.dimensions, UI.GetAnimFocus(id, focus, state));
		Text.Add(text, Matrix.T(layout.center.x, layout.center.y, -(offset + 0.002f)), UI.TextStyle, TextAlign.Center);

		return state.IsJustInactive();
	}

	static bool CustomButtonTheme(string text)
	{
		ulong id = UI.StackHash(text);

		Vec2   size   = Text.SizeLayout(text, UI.TextStyle) + UI.Settings.padding * 2;
		Bounds layout = UI.LayoutReserve(size);
		UI.ButtonBehavior(layout.TLC, size, text, out float offset, out BtnState state, out BtnState focus, out int hand);

		layout.center.z    -= offset / 2;
		layout.dimensions.z = offset;
		UI.DrawElement(CustomUIVisual.Button, layout.TLB, layout.dimensions, UI.GetAnimFocus(id, focus, state));
		Text.Add(text, Matrix.T(layout.center.x, layout.center.y, -(offset + 0.002f)), UI.TextStyle, TextAlign.Center);

		return state.IsJustInactive();
	}

	static bool CustomButtonNonexistentTheme(string text)
	{
		ulong id = UI.StackHash(text);

		Vec2   size   = Text.SizeLayout(text, UI.TextStyle) + UI.Settings.padding * 2;
		Bounds layout = UI.LayoutReserve(size);
		UI.ButtonBehavior(layout.TLC, size, text, out float offset, out BtnState state, out BtnState focus, out int hand);

		layout.center.z    -= offset / 2;
		layout.dimensions.z = offset;
		UI.DrawElement(CustomUIVisual.Nonexistent, layout.TLB, layout.dimensions, UI.GetAnimFocus(id, focus, state));
		Text.Add(text, Matrix.T(layout.center.x, layout.center.y, -(offset + 0.002f)), UI.TextStyle, TextAlign.Center);

		return state.IsJustInactive();
	}

	static void Tooltip(string text)
	{
		if (!UI.LastElementFocused.IsActive()) return;

		Bounds     parent = UI.LayoutLast;
		UISettings s      = UI.Settings;
		float      size   = Math.Min(parent.dimensions.x, Text.SizeLayout(text, UI.TextStyle).x + s.padding*2);

		UI.LayoutPush(new Vec3(parent.center.x + size/2, parent.center.y-parent.dimensions.y/2 - s.gutter, -s.depth*1.5f ), new Vec2(size, 0));
		UI.PanelBegin(UIPad.Inside);
		UI.Text(text);
		UI.PanelEnd();
		UI.LayoutPop();
	}
}