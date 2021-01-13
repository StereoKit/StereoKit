using System;
using System.Collections.Generic;
using System.Numerics;
using StereoKit;

class Program 
{
#if WINDOWS_UWP
	public const string Root = "Assets";
#else
	public const string Root = "../../../Examples/Assets";
#endif

	static Model     floorMesh;
	static Matrix4x4 floorTr;
	static Pose      demoSelectPose = new Pose();

	//////////////////////
	// Debug log window //
	//////////////////////

	/// :CodeSample: Log.Subscribe Log
	/// ### An in-application log window
	/// Here's an example of using the Log.Subscribe method to build a 
	/// simple logging window. This can be pretty handy to have around 
	/// somewhere in your application!
	/// 
	/// Here's the code for the window, and log tracking.
	static Pose         logPose = new Pose(0, -0.1f, 0.5f, Quat.LookDir(Vec3.Forward));
	static List<string> logList = new List<string>();
	static void OnLog(LogLevel level, string text)
	{
		if (logList.Count > 10)
			logList.RemoveAt(logList.Count - 1);
		logList.Insert(0, text.Length < 100 ? text : text.Substring(0,100)+"...");
	}
	static void LogWindow()
	{
		UI.WindowBegin("Log", ref logPose, new Vector2(40, 0) * U.cm);
		for (int i = 0; i < logList.Count; i++)
			UI.Label(logList[i], false);
		UI.WindowEnd();
	}
	/// :End:

	//////////////////////

	static void Main(string[] args) 
	{
		Tests.IsTesting = args.Length > 0 && args[0].ToLower() == "-test";
		Time .Scale     = Tests.IsTesting ? 0 : 1;

		/// :CodeSample: Log.Subscribe Log
		/// Then you add the OnLog method into the log events like this in your initialization
		/// code!
		Log.Subscribe(OnLog);
		/// :End:
		Log.Filter = LogLevel.Diagnostic;

		if (!SK.Initialize(new SKSettings { 
				appName           = "StereoKit C#",
				assetsFolder      = Program.Root,
				displayFallback   = true,
				displayPreference = Tests.IsTesting ? DisplayMode.Flatscreen : DisplayMode.MixedReality}))
			Environment.Exit(1);

		CommonInit();

		Tests.FindTests();
		Tests.SetTestActive(args.Length > 0 ? args[0] : "pbr");
		Tests.Initialize();

		while (SK.Step(() =>
		{
			Tests.Update();
			CommonUpdate();
		}));

		Tests.Shutdown();
		CommonShutdown();

		SK.Shutdown();
	}

	static void CommonInit()
	{
		/*Material floorMat = new Material(Shader.FromFile("floor_shader.hlsl"));
		floorMat.Transparency = Transparency.Blend;
		floorMat.SetVector("radius", new Vec4(5,10,0,0));
		floorMat.QueueOffset = -11;

		floorMesh = Model.FromMesh(Mesh.GenerateCube(Vec3.One), floorMat);
		floorTr   = Matrix.TRS(new Vec3(0, -1.5f, 0), Quat.Identity, new Vec3(40, .01f, 40));*/

		demoSelectPose.position    = new Vector3(0, 0, -0.4f);
		demoSelectPose.orientation = Quat.LookDir(-Vec3.Forward);
	}
	static void CommonUpdate()
	{
		if (Input.Key(Key.Esc).IsJustActive())
			SK.Quit();

		// If we can't see the world, we'll draw a floor!
		//if (StereoKitApp.System.displayType == Display.Opaque)
		//    Renderer.Add(floorMesh, floorTr, Color.White);

		// Skip selection window if we're in test mode
		if (Tests.IsTesting)
			return;

		// Lets add floor bounds if we have 'em
		if (World.HasBounds)
		{
			Vector2   s    = World.BoundsSize/2;
			Matrix4x4 pose = World.BoundsPose.ToMatrix();
			Vector3 tl = pose.Transform( new Vector3( s.X, 0,  s.Y) );
			Vector3 br = pose.Transform( new Vector3(-s.X, 0, -s.Y) );
			Vector3 tr = pose.Transform( new Vector3(-s.X, 0,  s.Y) );
			Vector3 bl = pose.Transform( new Vector3( s.X, 0, -s.Y) );

			Lines.Add(tl, tr, Color.White, 1.5f*U.cm);
			Lines.Add(bl, br, Color.White, 1.5f*U.cm);
			Lines.Add(tl, bl, Color.White, 1.5f*U.cm);
			Lines.Add(tr,br, Color.White, 1.5f*U.cm);
		}

		// Make a window for demo selection
		UI.WindowBegin("Demos", ref demoSelectPose, new Vector2(50 * U.cm, 0));
		for (int i = 0; i < Tests.DemoCount; i++)
		{
			string name = Tests.GetDemoName(i).Substring("Demo".Length);

			if (UI.Button(name))
				Tests.SetDemoActive(i);
			UI.SameLine();
		}
		UI.WindowEnd();

		RulerWindow();
		DebugToolWindow.Step();
		/// :CodeSample: Log.Subscribe Log
		/// And in your Update loop, you can draw the window.
		LogWindow();
		/// And that's it!
		/// :End:
	}
	static void CommonShutdown()
	{
	}

	//////////////////
	// Ruler object //
	//////////////////

	static Pose demoRuler = new Pose(0, 0, .5f, Quat.Identity);
	static void RulerWindow()
	{
		UI.HandleBegin("Ruler", ref demoRuler, new Bounds(new Vector3(30,4,1)*U.cm), true);
		Color32 color = Color.HSV(.6f, 0.5f, 1);
		Text.Add("Centimeters", Matrix.TRS(new Vector3(14.5f, -1.5f, -.6f)*U.cm, Quat.Identity, .3f), TextAlign.XLeft | TextAlign.YBottom);
		for (int d = 0; d <= 60; d+=1)
		{
			float x = d/2.0f;
			float size = d%2==0?1f:0.15f;
			Lines.Add(new Vector3(15-x,2,-.6f)*U.cm, new Vector3(15-x,2-size, -.6f)*U.cm, color, U.mm*0.5f);
			if (d%2==0 && d/2 != 30)
				Text.Add((d/2).ToString(), Matrix.TRS(new Vector3(15-x-0.1f,2-size, -.6f)*U.cm, Quat.Identity, .2f), TextAlign.XLeft|TextAlign.YBottom);
		}
		UI.HandleEnd();
	}
}
