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
	static SKSettings settings = new SKSettings {
		appName         = "StereoKit C#",
		blendPreference = DisplayBlend.AnyTransparent,
		mode            = AppMode.XR,
		renderScaling   = 1.5f,
	};

	static Mesh      floorMesh;
	static Material  floorMat;
	static Pose      windowDemoPose = new Pose(-0.7f, 0, -0.3f, Quat.LookDir(1, 0, 1));
	static Sprite    powerButton;
	static SceneType sceneCategory = SceneType.Demos;

	public static LogWindow WindowLog;
	public static bool      WindowDemoShow = false;

	static void Main(string[] args)
	{
		// CLI arguments
		bool headless            =  ParamPresent(args, "-headless");
		bool xr                  =  ParamPresent(args, "-xr"); // Force XR in testing mode
		Tests.IsTesting          =  ParamPresent(args, "-test");
		Tests.MakeScreenshots    = !ParamPresent(args, "-noscreens");
		Tests.ScreenshotRoot     =  ParamVal    (args, "-screenfolder",     "../../../tools/screenshots");
		Tests.GltfFolders        =  ParamVal    (args, "-gltf",             null); // "C:\\Tools\\glTF-Sample-Models-master\\2.0";
		Tests.GltfScreenshotRoot =  ParamVal    (args, "-gltfscreenfolder", null);
		Tests.TestSingle         =  ParamPresent(args, "-start");
		startTest                =  ParamVal    (args, "-start",            startTest);

		if (Tests.IsTesting)
		{
			settings.mode        = headless ? AppMode.Offscreen : xr ? AppMode.XR : AppMode.Simulator;
			settings.standbyMode = StandbyMode.None;
		}

		// OpenXR extensions need added before SK.Initialize, so does
		// LogWindow for early log registration!
		SK.AddStepper<Win32PerformanceCounterExt>();
		SK.AddStepper<XrCompLayers>();
		SK.AddStepper(new XrRefreshRate(120));
		WindowLog = SK.AddStepper<LogWindow>();
		WindowLog.Enabled = false;

		// Initialize StereoKit
		if (!SK.Initialize(settings))
			Environment.Exit(1);

		Init();

		SK.Run(Step, Tests.Shutdown);
	}

	static void Init()
	{
		floorMat = new Material("Shaders/floor_shader.hlsl");
		floorMat.Transparency = Transparency.Blend;
		floorMat.QueueOffset  = -11;
		floorMat["radius"]    = new Vec4(5, 10, 0, 0);

		floorMesh   = Mesh.GeneratePlane(V.XY(40,40), Vec3.Up, Vec3.Forward);
		powerButton = Sprite.FromTex(Tex.FromFile("power.png"));

		Tests.FindTests();
		Tests.SetTestActive(startTest);
		Tests.Initialize();

		if (Tests.IsTesting)
		{
			UI  .EnableFarInteract = false;
			Time.Scale             = 0;
			WindowDemoShow         = false;
		}
		else
		{
			WindowDemoShow = true;
			SK.AddStepper<DebugToolWindow>();
		}
	}

	//////////////////////

	static void Step()
	{
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

		CheckFocus    ();
		Tests.Update  ();
		WindowDemoStep();
	}

	static void WindowDemoStep()
	{
		// Skip the window if we're in test mode
		if (!WindowDemoShow) return;

		// Make a window for demo selection
		UI.WindowBegin("Demos", ref windowDemoPose, new Vec2(50 * U.cm, 0));

		// Display the different categories of tests we have available
		for (int i = 0; i < (int)SceneType.MAX; i++)
		{
			SceneType category = (SceneType)i;
			if (UI.Radio(category.ToString(), category == sceneCategory))
				sceneCategory = category;
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
		float      windowWidth    = UI.LayoutRemaining.x;
		for (int i = 0; i < Tests.Count(sceneCategory); i++)
		{
			float width = Text.SizeLayout(Tests.GetTestName(sceneCategory,i), style).x + uiSettings.padding * 2;
			if (currWidthTotal + (width+uiSettings.gutter) > windowWidth)
			{
				float inflate = (windowWidth - (currWidthTotal-uiSettings.gutter+0.0001f)) / (i - start);
				for (int t = start; t < i; t++)
				{
					string name      = Tests.GetTestName(sceneCategory,t);
					float  currWidth = Text.SizeLayout(name, style).x + uiSettings.padding * 2 + inflate;
					if (UI.Radio(name, Tests.IsActive(sceneCategory, t),  null, null, UIBtnLayout.None, new Vec2(currWidth, 0) ))
						Tests.SetTestActive(sceneCategory, t);
					UI.SameLine();
				}
				start = i;
			}
			if (start == i)
				currWidthTotal = 0;
			currWidthTotal += width + uiSettings.gutter;
		}
		for (int t = start; t < Tests.Count(sceneCategory); t++)
		{
			string name      = Tests.GetTestName(sceneCategory, t);
			float  currWidth = Text.SizeLayout(name, style).x + uiSettings.padding * 2;
			if (UI.Radio(name, Tests.IsActive(sceneCategory, t), null, null, UIBtnLayout.None, new Vec2(currWidth, 0)))
				Tests.SetTestActive(sceneCategory, t);
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
