using StereoKit;
using System.Collections.Generic;

class App
{
	string     startTest = "geo";
	SKSettings settings  = new SKSettings {
		appName           = "StereoKit C#",
		assetsFolder      = "Assets",
		blendPreference   = Display.AnyTransparent,
		displayPreference = DisplayMode.MixedReality };

	public SKSettings Settings => settings;

	Model  floorMesh;
	Matrix floorTr;
	Pose   demoSelectPose = new Pose();

	//////////////////////

	public App(string[] args)
	{
		Tests.IsTesting = args.Length > 0 && args[0].ToLower() == "-test";
		if (!Tests.IsTesting && args.Length > 0)
			startTest = args[0];
		if (Tests.IsTesting)
			settings.displayPreference = DisplayMode.Flatscreen;

		Time.Scale = Tests.IsTesting ? 0 : 1;

		/// :CodeSample: Log.Subscribe Log
		/// Then you add the OnLog method into the log events like this in
		/// your initialization code!
		Log.Subscribe(OnLog);
		/// :End:
		Log.Filter = LogLevel.Diagnostic;
	}

	//////////////////////

	public void Init()
	{
		Material floorMat = new Material(Shader.FromFile("floor_shader.hlsl"));
		floorMat.Transparency = Transparency.Blend;
		floorMat.SetVector("radius", new Vec4(5,10,0,0));
		floorMat.QueueOffset = -11;

		floorMesh = Model.FromMesh(Mesh.GenerateCube(Vec3.One), floorMat);
		floorTr   = Matrix.TRS(new Vec3(0, -1.5f, 0), Quat.Identity, new Vec3(40, .01f, 40));

		demoSelectPose.position    = new Vec3(0, 0, -0.4f);
		demoSelectPose.orientation = Quat.LookDir(-Vec3.Forward);

		Tests.FindTests();
		Tests.SetTestActive(startTest);
		Tests.Initialize();
	}

	//////////////////////

	public void Step()
	{
		Tests.Update();

		if (Input.Key(Key.Esc).IsJustActive())
			SK.Quit();

		// If we can't see the world, we'll draw a floor!
		if (SK.System.displayType == Display.Opaque)
			Renderer.Add(floorMesh, floorTr, Color.White);

		// Skip selection window if we're in test mode
		if (Tests.IsTesting)
			return;

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

		// Make a window for demo selection
		UI.WindowBegin("Demos", ref demoSelectPose, new Vec2(50 * U.cm, 0));
		for (int i = 0; i < Tests.DemoCount; i++)
		{
			string name = Tests.GetDemoName(i).Substring("Demo".Length);

			if (UI.Button(name))
				Tests.SetDemoActive(i);
			UI.SameLine();
		}
		UI.NextLine();
		UI.HSeparator();
		if (UI.Button("Exit"))
			SK.Quit();
		UI.WindowEnd();

		RulerWindow();
		DebugToolWindow.Step();
		/// :CodeSample: Log.Subscribe Log
		/// And in your Update loop, you can draw the window.
		LogWindow();
		/// And that's it!
		/// :End:
	}

	//////////////////
	// Ruler object //
	//////////////////

	static Pose demoRuler = new Pose(0, 0, .5f, Quat.Identity);
	static void RulerWindow()
	{
		UI.HandleBegin("Ruler", ref demoRuler, new Bounds(new Vec3(31,4,1)*U.cm), true);
		Color32 color = Color.HSV(.6f, 0.5f, 1);
		Text.Add("Centimeters", Matrix.TRS(new Vec3(14.5f, -1.5f, -.6f)*U.cm, Quat.Identity, .3f), TextAlign.XLeft | TextAlign.YBottom);
		for (int d = 0; d <= 60; d+=1)
		{
			float x = d/2.0f;
			float size = d%2==0?1f:0.15f;
			Lines.Add(new Vec3(15-x,1.8f,-.6f)*U.cm, new Vec3(15-x,1.8f-size, -.6f)*U.cm, color, U.mm*0.5f);
			if (d%2==0 && d/2 != 30)
				Text.Add((d/2).ToString(), Matrix.TRS(new Vec3(15-x-0.1f,2-size, -.6f)*U.cm, Quat.Identity, .2f), TextAlign.XLeft|TextAlign.YBottom);
		}
		UI.HandleEnd();
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
		UI.WindowBegin("Log", ref logPose, new Vec2(40, 0) * U.cm);
		for (int i = 0; i < logList.Count; i++)
			UI.Label(logList[i], false);
		UI.WindowEnd();
	}
	/// :End:
}
