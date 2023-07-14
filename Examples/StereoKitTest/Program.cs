// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using System;
using System.Collections.Generic;
using StereoKit;
using StereoKit.Framework;

class Program
{
	static string startTest = "welcome";
	static SKSettings settings = new SKSettings {
		appName           = "StereoKit C#",
		assetsFolder      = "Assets",
		blendPreference   = DisplayBlend.AnyTransparent,
		displayPreference = DisplayMode.MixedReality,
		logFilter         = LogLevel.Diagnostic,
		//origin            = OriginMode.Floor,
	};

	static Model  floorMesh;
	static Matrix floorTr;
	static Pose   windowDemoPose = new Pose();
	static bool   windowDemoShow = false;
	static bool   windowConsoleShow = false;
	static Sprite powerButton;

	static List<string> demoNames    = new List<string>();
	static float        demoWinWidth = 50 * U.cm;

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
		get => windowConsoleShow;
		set {
			if (windowConsoleShow == value) return;
			windowConsoleShow = value;
			if (windowConsoleShow)
				logPose = UI.PopupPose();
		}
	}

	static void Main(string[] args) 
	{
		Tests.IsTesting       = Array.IndexOf(args, "-test") != -1;
		Tests.MakeScreenshots = Array.IndexOf(args, "-noscreens") == -1;
		if (Array.IndexOf(args, "-screenfolder") != -1)
			Tests.ScreenshotRoot = args[Array.IndexOf(args, "-screenfolder")+1];
		if (Array.IndexOf(args, "-start") != -1)
		{
			startTest = args[Array.IndexOf(args, "-start") + 1];
			Tests.TestSingle = true;
		}

		if (Tests.IsTesting)
		{
			settings.displayPreference     = DisplayMode.Flatscreen;
			settings.disableUnfocusedSleep = true;
		}

		// Preload the StereoKit library for access to Time.Scale before
		// initialization occurs.
		SK.PreLoadLibrary();
		Time.Scale = Tests.IsTesting ? 0 : 1;

		/// :CodeSample: Log.Subscribe Log
		/// Then you add the OnLog method into the log events like this in
		/// your initialization code!
		Log.Subscribe(OnLog);
		/// :End:
		
		SK.AddStepper<PassthroughFBExt>();
		//SK.AddStepper<Win32PerformanceCounterExt>();

		// Initialize StereoKit
		if (!SK.Initialize(settings))
			Environment.Exit(1);

		Init();

		SK.Run(Step);

		Tests.Shutdown();
	}

	static void Init()
	{
		Material floorMat = new Material(Shader.FromFile("Shaders/floor_shader.hlsl"));
		floorMat.Transparency = Transparency.Blend;
		floorMat.SetVector("radius", new Vec4(5,10,0,0));
		floorMat.QueueOffset = -11;

		floorMesh = Model.FromMesh(Mesh.GeneratePlane(new Vec2(40,40), Vec3.Up, Vec3.Forward), floorMat);
		floorTr   = Matrix.TR(new Vec3(0, -1.5f, 0), Quat.Identity);

		powerButton = Sprite.FromTex(Tex.FromFile("power.png"));

		WindowDemoShow = true;

		Tests.FindTests();
		Tests.SetTestActive(startTest);
		Tests.Initialize();

		for (int i = 0; i < Tests.DemoCount; i++)
			demoNames.Add(Tests.GetDemoName(i).Substring("Demo".Length));

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
		if (SK.System.displayType == Display.Opaque)
			Renderer.Add(floorMesh, World.HasBounds ? World.BoundsPose.ToMatrix() : floorTr, Color.White);

		// Skip selection window if we're in test mode
		if (Tests.IsTesting)
			return;

		WindowDemoStep();

		if (windowConsoleShow)
		{
			/// :CodeSample: Log.Subscribe Log
			/// And in your Update loop, you can draw the window.
			LogWindow();
			/// And that's it!
			/// :End:
		}
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
			if (currWidthTotal + (width+uiSettings.gutter) > demoWinWidth || i == demoNames.Count-1)
			{
				float inflate = i == demoNames.Count - 1
					? 0 
					: (demoWinWidth - (currWidthTotal-uiSettings.gutter+0.0001f)) / (i - start);
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
		UI.NextLine();
		UI.HSeparator();
		if (UI.ButtonImg("Exit", powerButton))
			SK.Quit();
		UI.WindowEnd();
	}

	//////////////////////
	// Debug log window //
	//////////////////////

	/// :CodeSample: Log.Subscribe Log
	/// ### An in-application log window
	/// Here's an example of using the Log.Subscribe method to build a simple
	/// logging window. This can be pretty handy to have around somewhere in
	/// your application!
	/// 
	/// Here's the code for the window, and log tracking.
	static Pose         logPose   = new Pose(0, -0.1f, 0.5f, Quat.LookDir(Vec3.Forward));
	static List<string> logList   = new List<string>();
	static float        logIndex  = 0;
	static string       logString = "";
	static void OnLog(LogLevel level, string text)
	{
		logList.Insert(0, text.Length < 100 ? text : text.Substring(0, 100) + "...\n");
		UpdateLogStr((int)logIndex);
	}

	static void UpdateLogStr(int index)
	{
		logIndex  = Math.Max(Math.Min(index, logList.Count-1), 0);
		logString = "";
		for (int i = index; i < index + 15 && i < logList.Count; i++)
			logString += logList[i];
	}

	static void LogWindow()
	{
		UI.WindowBegin("Log", ref logPose, new Vec2(40, 0) * U.cm);

		UI.LayoutPushCut(UICut.Right, UI.LineHeight);
		if (UI.VSlider("scroll", ref logIndex, 0, Math.Max(logList.Count - 3, 0), 1))
			UpdateLogStr((int)logIndex);
		UI.LayoutPop();

		UI.Text(logString);
		UI.WindowEnd();
	}
	/// :End:

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
