// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2025 Nick Klingensmith
// Copyright (c) 2023-2025 Qualcomm Technologies, Inc.

using System;
using StereoKit;
using StereoKit.Framework;

class Program
{
	// This is the starting scene, and can be overridden by passing
	// -start <testname> via the CLI.
	static string startTest = "welcome";

	// The base settings we use for this test app. Some of these, like mode,
	// are overridden, particularly when running tests.
	static SKSettings     settings  = new SKSettings {
		appName         = "StereoKit C#",
		blendPreference = DisplayBlend.AnyTransparent,
		mode            = AppMode.XR,
	};

	static Mesh           floorMesh;
	static Material       floorMat;
	static Pose           windowDemoPose = new Pose(-0.7f, 0, -0.3f, Quat.LookDir(1, 0, 1));
	static Sprite         powerButton;
	static Tests.Category testCategory = Tests.Category.Demo;
	static float          demoWinWidth = 50 * U.cm;

	public static LogWindow WindowLog;
	public static bool      WindowDemoShow = false;

	static void Main(string[] args)
	{
		bool headless            =  ParamPresent(args, "-headless");
		Tests.IsTesting          =  ParamPresent(args, "-test");
		Tests.MakeScreenshots    = !ParamPresent(args, "-noscreens");
		Tests.ScreenshotRoot     =  ParamVal    (args, "-screenfolder",     null);
		Tests.GltfFolders        =  ParamVal    (args, "-gltf",             null);
		Tests.GltfScreenshotRoot =  ParamVal    (args, "-gltfscreenfolder", null);
		Tests.TestSingle         =  ParamPresent(args, "-start");
		startTest                =  ParamVal    (args, "-start",            startTest);

		if (Tests.IsTesting)
		{
			settings.mode        = headless ? AppMode.Offscreen : AppMode.Simulator;
			settings.standbyMode = StandbyMode.None;
		}

		SK.AddStepper<PassthroughFBExt>();
		SK.AddStepper<Win32PerformanceCounterExt>();
		WindowLog = SK.AddStepper<LogWindow>();
		WindowLog.Enabled = false;

		// Initialize StereoKit
		if (!SK.Initialize(settings))
			Environment.Exit(1);

		Time.Scale = Tests.IsTesting ? 0 : 1;

		Init();

		SK.Run(Step, Tests.Shutdown);
	}

	static void Init()
	{
		floorMat = new Material("Shaders/floor_shader.hlsl");
		floorMat.Transparency = Transparency.Blend;
		floorMat.QueueOffset  = -11;
		floorMat["radius"]    = new Vec4(5, 10, 0, 0);

		floorMesh   = Mesh.GeneratePlane(new Vec2(40,40), Vec3.Up, Vec3.Forward);
		powerButton = Sprite.FromTex(Tex.FromFile("power.png"));

		WindowDemoShow = true;

		Tests.FindTests();
		Tests.SetTestActive(startTest);
		Tests.Initialize();

		if (Tests.IsTesting)
		{
			UI.EnableFarInteract = false;
		}
		else
		{
			SK.AddStepper<DebugToolWindow>();
		}
	}

	//////////////////////

	static void Step()
	{
		CheckFocus();

		Tests.Update();

		if (Input.Key(Key.Esc).IsJustActive())
			SK.Quit();

		/// :CodeSample: Projection Renderer.Projection
		/// ### Toggling the projection mode
		/// Only in flatscreen apps, there is the option to change the main
		/// camera's projection mode between perspective and orthographic.
		if (SK.ActiveDisplayMode == DisplayMode.Flatscreen &&
			Input.Key(Key.P).IsJustActive())
		{
			Renderer.Projection = Renderer.Projection == Projection.Perspective
				? Projection.Ortho
				: Projection.Perspective;
		}
		/// :End:

		// If we can't see the world, we'll draw a floor!
		if (Device.DisplayBlend == DisplayBlend.Opaque)
			floorMesh.Draw(floorMat, World.HasBounds ? World.BoundsPose.ToMatrix() : Matrix.T(0, -1.5f, 0), Color.White);

		// Skip selection window if we're in test mode
		if (Tests.IsTesting)
			return;

		WindowDemoStep();
	}

	static void WindowDemoStep()
	{
		if (!WindowDemoShow) return;

		// Make a window for demo selection
		UI.WindowBegin("Demos", ref windowDemoPose, new Vec2(demoWinWidth, 0));

		// Display the different categories of tests we have available
		for (int i = 0; i < (int)Tests.Category.MAX; i++)
		{
			string categoryName = "";
			switch ((Tests.Category)i)
			{
				case Tests.Category.Demo:          categoryName = "Demos"; break;
				case Tests.Category.Test:          categoryName = "Tests"; break;
				case Tests.Category.Documentation: categoryName = "Docs"; break;
				default: categoryName = ((Tests.Category)i).ToString(); break;
			}
			if (UI.Radio(categoryName, (Tests.Category)i == testCategory))
				testCategory = (Tests.Category)i;
			UI.SameLine();
		}
		// Display a quit button on the far right side
		Vec2 exitSize = new Vec2(0.06f, 0);
		UI.HSpace(UI.LayoutRemaining.x - exitSize.x);
		if (UI.ButtonImg("Exit", powerButton, UIBtnLayout.Left, exitSize))
			SK.Quit();

		UI.HSeparator();

		// Now display a nice, lined-up collection of buttons for each
		// demo/test in the current category.
		int        start          = 0;
		float      currWidthTotal = 0;
		UISettings uiSettings     = UI.Settings;
		TextStyle  style          = UI.TextStyle;
		for (int i = 0; i < Tests.Count(testCategory); i++)
		{
			float width = Text.SizeLayout(Tests.GetTestName(testCategory,i), style).x + uiSettings.padding * 2;
			if (currWidthTotal + (width+uiSettings.gutter) > demoWinWidth)
			{
				float inflate = (demoWinWidth - (currWidthTotal-uiSettings.gutter+0.0001f)) / (i - start);
				for (int t = start; t < i; t++)
				{
					string name      = Tests.GetTestName(testCategory,t);
					float  currWidth = Text.SizeLayout(name, style).x + uiSettings.padding * 2 + inflate;
					if (UI.Radio(name, Tests.IsActive(testCategory,t),  null, null, UIBtnLayout.None, new Vec2(currWidth, 0) ))
						Tests.SetTestActive(testCategory, t);
					UI.SameLine();
				}
				start = i;
			}
			if (start == i)
				currWidthTotal = uiSettings.margin * 2;
			currWidthTotal += width + uiSettings.gutter;
		}
		for (int t = start; t < Tests.Count(testCategory); t++)
		{
			string name      = Tests.GetTestName(testCategory, t);
			float  currWidth = Text.SizeLayout(name, style).x + uiSettings.padding * 2;
			if (UI.Radio(name, Tests.IsActive(testCategory, t), null, null, UIBtnLayout.None, new Vec2(currWidth, 0)))
				Tests.SetTestActive(testCategory, t);
			UI.SameLine();
		}

		UI.WindowEnd();
	}

	/// :CodeSample: AppFocus SK.AppFocus
	/// ### Checking for changes in application focus
	static AppFocus lastFocus = AppFocus.Hidden;
	static void CheckFocus()
	{
		if (lastFocus != SK.AppFocus)
		{
			lastFocus = SK.AppFocus;
			Log.Info($"App focus changed to: {lastFocus}");
		}
	}
	/// :End:

	static bool ParamPresent(string[] args, string param) 
		=> Array.IndexOf(args, param) != -1;
	static string ParamVal(string[] args, string param, string defaultVal)
	{
		int index = Array.IndexOf(args, param);
		return (index == -1 || index + 1 >= args.Length)
			? defaultVal 
			: args[index + 1];
	}
}
