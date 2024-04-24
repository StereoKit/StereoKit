// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using StereoKit;
using StereoKit.Framework;

class Program
{
	static string startTest = "welcome";
	static SKSettings settings = new SKSettings {
		appName         = "StereoKit C#",
		assetsFolder    = "Assets",
		blendPreference = DisplayBlend.AnyTransparent,
		mode            = AppMode.Simulator,
		logFilter       = LogLevel.Diagnostic,
		renderMultisample = 1,
		depthMode       = DepthMode.Stencil,
		
		//origin          = OriginMode.Floor,
	};

	static Model  floorMesh;
	static Matrix floorTr;
	static Pose   windowDemoPose = new Pose();
	static bool   windowDemoShow = false;
	static Sprite powerButton;

	static List<string> demoNames    = new List<string>();
	static float        demoWinWidth = 50 * U.cm;

	public static LogWindow logWindow;

	public static bool WindowDemoShow
	{
		get => windowDemoShow;
		set {
			if (windowDemoShow == value) return;
			windowDemoShow = value;
			if (windowDemoShow)
			{
				windowDemoPose.position    = new Vec3(-0.7f, 0, -0.3f);
				windowDemoPose.orientation = Quat.LookDir(1, 0, 1);
			}
		}
	}

	public static bool WindowConsoleShow
	{
		get => logWindow.Enabled;
		set {
			if (logWindow.Enabled == value) return;
			logWindow.Enabled = value;
			if (logWindow.Enabled)
				logWindow.pose = UI.PopupPose();
		}
	}

	static async Task Main(string[] args)
	{
		bool headless = Array.IndexOf(args, "-headless") != -1;
		Tests.IsTesting = Array.IndexOf(args, "-test") != -1;
		Tests.MakeScreenshots = Array.IndexOf(args, "-noscreens") == -1;
		if (Array.IndexOf(args, "-screenfolder") != -1)
			Tests.ScreenshotRoot = args[Array.IndexOf(args, "-screenfolder") + 1];
		if (Array.IndexOf(args, "-gltf") != -1)
			Tests.GltfFolders = args[Array.IndexOf(args, "-gltf") + 1];
		if (Array.IndexOf(args, "-gltfscreenfolder") != -1)
			Tests.GltfScreenshotRoot = args[Array.IndexOf(args, "-gltfscreenfolder") + 1];
		if (Array.IndexOf(args, "-start") != -1)
		{
			startTest = args[Array.IndexOf(args, "-start") + 1];
			Tests.TestSingle = true;
		}

		if (Tests.IsTesting)
		{
			settings.mode = headless ? AppMode.Offscreen : AppMode.Simulator;
			settings.disableUnfocusedSleep = true;
		}

		// Preload the StereoKit library for access to Time.Scale before
		// initialization occurs.
		SK.PreLoadLibrary();

		//SK.AddStepper<PassthroughFBExt>();
		//SK.AddStepper<Win32PerformanceCounterExt>();
		//logWindow = SK.AddStepper<LogWindow>();
		//logWindow.Enabled = false;
		Log.Info($"Using Platform:{Backend.Platform}");
		Log.Info($"Using Graphics:{Backend.Graphics}");

		// Initialize StereoKit
		if (!SK.Initialize(settings))
			Environment.Exit(1);

		Time.Scale = Tests.IsTesting ? 0 : 1;
		Renderer.ClearColor = new Color(0, 0f, 1f, 0.5f);
		Init();

		// RunAsync() is needed so that single browswer thread dosn't get blocked by the 
		// SK.Run() loop. This is only needed for the browser.
		await SK.RunAsync(() => Step(), () => Tests.Shutdown());

		if (SK.QuitReason != QuitReason.None)
		{
			Log.Info("QuitReason is " + SK.QuitReason);
		}
	}

    static void Init()
	{
		Material floorMat = Material.Default;// new Material(Shader.FromFile("Shaders/floor_shader.hlsl"));
		floorMat.Transparency = Transparency.Blend;
		floorMat.SetVector("radius", new Vec4(5, 10, 0, 0));
		floorMat.QueueOffset = -11;

		floorMesh = Model.FromMesh(Mesh.GeneratePlane(new Vec2(40, 40), Vec3.Up, Vec3.Forward), floorMat);
		floorTr = Matrix.Identity;// Matrix.TR(new Vec3(0, -1.5f, 0), Quat.Identity);


		//powerButton = Sprite.FromTex(Tex.FromFile("power.png"));

		//WindowDemoShow = true;

		//Tests.FindTests();
		//Tests.SetTestActive(startTest);
		//Tests.Initialize();

		//for (int i = 0; i < Tests.DemoCount; i++)
		//	demoNames.Add(Tests.GetDemoName(i).Substring("Demo".Length));

		//if (Tests.IsTesting)
		//{
		//	UI.EnableFarInteract = false;
		//}
		//else
		//{
		//	SK.AddStepper<DebugToolWindow>();
		//}
	}

	//////////////////////

	static void Step()
	{
        Renderer.Add(floorMesh, floorTr, Color.White);
        Mesh.Cube.Draw(Material.Default, Matrix.T(new Vec3(0, 0.5f, -1.5f))); //Matrix.TRS(new Vec3(0, 0, -1.5f), Quat.Identity, 0.1f));
                                                                              //CheckFocus();
        Mesh.Cube.Draw(Material.Default, Matrix.T(new Vec3(0, 0f, 1.5f)));
        //Thread.Sleep(200); // give the runtime some time to think!


        //Tests.Update();

        //if (Input.Key(Key.Esc).IsJustActive())
        //	SK.Quit();

        ///// :CodeSample: Projection Renderer.Projection
        ///// ### Toggling the projection mode
        ///// Only in flatscreen apps, there is the option to change the main
        ///// camera's projection mode between perspective and orthographic.
        //if (SK.ActiveDisplayMode == DisplayMode.Flatscreen &&
        //	Input.Key(Key.P).IsJustActive())
        //{
        //	Renderer.Projection = Renderer.Projection == Projection.Perspective
        //		? Projection.Ortho
        //		: Projection.Perspective;
        //}
        ///// :End:

        //// If we can't see the world, we'll draw a floor!
        //if (Device.DisplayBlend == DisplayBlend.Opaque)
        //	Renderer.Add(floorMesh, World.HasBounds ? World.BoundsPose.ToMatrix() : floorTr, Color.White);

        //// Skip selection window if we're in test mode
        //if (Tests.IsTesting)
        //	return;

        //WindowDemoStep();
    }

	static void WindowDemoStep()
	{
		if (!windowDemoShow) return;

		// Make a window for demo selection
		UI.WindowBegin("Demos", ref windowDemoPose, new Vec2(demoWinWidth, 0));
		int        start          = 0;
		float      currWidthTotal = 0;
		UISettings uiSettings     = UI.Settings;
		TextStyle  style          = UI.TextStyle;
		for (int i = 0; i < demoNames.Count; i++)
		{
			float width = Text.Size(demoNames[i], style).x + uiSettings.padding * 2;
			if (currWidthTotal + (width+uiSettings.gutter) > demoWinWidth)
			{
				float inflate = (demoWinWidth - (currWidthTotal-uiSettings.gutter+0.0001f)) / (i - start);
				for (int t = start; t < i; t++)
				{
					float currWidth = Text.Size(demoNames[t], style).x + uiSettings.padding * 2 + inflate;
					if (UI.Button(demoNames[t], new Vec2(currWidth, 0)))
						Tests.SetDemoActive(t);
					UI.SameLine();
				}
				start = i;
			}
			if (start == i)
				currWidthTotal = uiSettings.margin * 2;
			currWidthTotal += width + uiSettings.gutter;
		}
		for (int t = start; t < demoNames.Count; t++)
		{
			float currWidth = Text.Size(demoNames[t], style).x + uiSettings.padding * 2;
			if (UI.Button(demoNames[t], new Vec2(currWidth, 0)))
				Tests.SetDemoActive(t);
			UI.SameLine();
		}
		UI.NextLine();
		UI.HSeparator();
		if (UI.ButtonImg("Exit", powerButton))
			SK.Quit();
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
}
