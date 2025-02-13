using StereoKit;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;

public enum SceneType
{
	Demos,
	Tests,
	Docs,
	MAX
}

public static class Tests
{
	struct SceneItem
	{
		public Type   type;
		public string name;
	}

	static Type[]          allScenes;
	static SceneItem[][]   sceneTypes;
	static ITest           activeScene;
	static ITest           nextScene;
	static int             sceneIndex  = 0;
	static int             runFrames   = 2;
	static float           runSeconds  = 0;
	static int             sceneFrame  = 0;
	static float           sceneTime   = 0;
	static int             failures    = 0;
	static HashSet<string> screenshots = new HashSet<string>();

	private static Type   ActiveScene        { set { nextScene = (ITest)Activator.CreateInstance(value); } }
	public  static bool   IsTesting          { get; set; }
	public  static bool   TestSingle         { get; set; }

	public  static string ScreenshotRoot     { get; set; } = null;
	public  static bool   MakeScreenshots    { get; set; } = true;
	public  static string GltfFolders        { get; set; } = null;
	public  static string GltfScreenshotRoot { get; set; } = null;

	public static void FindTests()
	{
		allScenes = Assembly.GetExecutingAssembly()
			.GetTypes()
			.Where   (a => a != typeof(ITest) && typeof(ITest).IsAssignableFrom(a) )
			.OrderBy (a => a.Name )
			.ToArray ();

		sceneTypes = new SceneItem[(int)SceneType.MAX][];
		sceneTypes[(int)SceneType.Demos] = allScenes
			.Where  (t => t.Name.StartsWith("Demo"))
			.Select (t => new SceneItem { type = t, name = t.Name.Substring("Demo".Length) })
			.ToArray();
		sceneTypes[(int)SceneType.Tests] = allScenes
			.Where  (t => t.Name.StartsWith("Test"))
			.Select (t => new SceneItem { type = t, name = t.Name.Substring("Test".Length) })
			.ToArray();
		sceneTypes[(int)SceneType.Docs] = allScenes
			.Where  (t => t.Name.StartsWith("Doc"))
			.Select (t => new SceneItem { type = t, name = t.Name.Substring("Doc".Length) })
			.ToArray();
	}

	public static int  Count   (SceneType category)        => sceneTypes[(int)category].Length;
	public static bool IsActive(SceneType category, int i) => sceneTypes[(int)category][i].type == activeScene.GetType();

	public static void Initialize()
	{
		if (IsTesting && !TestSingle) {
			nextScene = null;
		}
		if (nextScene == null)
			nextScene = (ITest)Activator.CreateInstance(allScenes[sceneIndex]);
	}

	public static void Update()
	{
		if (IsTesting && runSeconds != 0)
			Time.SetTime(Time.Total+(1/90.0), 1/90.0);

		if (nextScene != null)
		{
			activeScene?.Shutdown();
			GC.Collect(int.MaxValue, GCCollectionMode.Forced);
			if (IsTesting)
			{
				Time .SetTime          (0);
				Input.HandVisible      (Handed.Max, false);
				Input.HandClearOverride(Handed.Left);
				Input.HandClearOverride(Handed.Right);
			}

			nextScene.Initialize();
			if (IsTesting)
				Assets.BlockForPriority(int.MaxValue);
			sceneTime   = Time.Totalf;
			activeScene = nextScene;
			nextScene   = null;
		}

		// If we're testing, catch and log exceptions instead of crashing
		if (IsTesting)
		{
			try { activeScene.Step(); }
			catch ( Exception e )
			{
				Log.Err(e.ToString());
				failures++;
				runFrames = sceneFrame + 1; // Ditch out of this test
			}
		} else {
			activeScene.Step();
		}
		sceneFrame++;

		if (IsTesting && FinishedWithTest())
		{
			sceneIndex += 1;
			if (sceneIndex >= allScenes.Length || TestSingle)
				SK.Quit();
			else
				SetTestActive(allScenes[sceneIndex].Name);
		}
	}
	public static void Shutdown()
	{
		activeScene?.Shutdown();
		activeScene = null;
		GC.Collect(int.MaxValue, GCCollectionMode.Forced);

		if (IsTesting) {
			if (failures != 0)
			{
				Log.Warn($"Testing <~RED>FAILED<~clr>: {failures} failures encountered!");
				Environment.Exit(-1);
			} else {
				Log.Info($"Testing <~GRN>passed!<~clr>");
			}
		}
		Log.Info($"Quit reason: <~WHT>{SK.QuitReason}<~clr>");
	}

	public static string GetTestName(SceneType category, int index)
	{
		return sceneTypes[(int)category][index].name;
	}

	public static void SetTestActive(SceneType category, int index)
	{
		Log.Info($"Starting Scene: {sceneTypes[(int)category][index].name}");
		ActiveScene = sceneTypes[(int)category][index].type;
	}

	public static void SetTestActive(string name)
	{
		name = name.ToLower();
		Type result = allScenes.OrderBy( a => {
			string str = a.Name.ToLower();
			if (str == name)             return 0;
			else if (str.Contains(name)) return str.Length - name.Length;
			else                         return 1000 + string.Compare(str, name);
		}).First();
		Log.Info($"Starting Scene: {result.Name}");

		sceneFrame  = 0;
		runFrames   = 2;
		runSeconds  = 0;
		ActiveScene = result;
	}

	public static void Test(Func<bool> testFunction)
	{
		try {
			if (!testFunction())
			{
				Log.Err($"Test failed for {testFunction.Method.Name}!");
				failures += 1;
			}
		} catch (Exception e) {
			Log.Err($"Test CRASHED for {testFunction.Method.Name}!\n{e.ToString()}");
			failures += 1;
		}
	}

	private static bool FinishedWithTest()
	{
		if (runSeconds != 0) return Time.Totalf-sceneTime > runSeconds;
		if (runFrames != -1) return sceneFrame == runFrames;
		return true;
	}

	public static void RunForFrames (int   frames)  => runFrames  = frames;
	public static void RunForSeconds(float seconds) => runSeconds = seconds;
	public static void RunContinue  ()              => runFrames +=  1;
	public static void RunStop      ()              => runFrames  = -1;

	public static void Screenshot(string name, int width, int height, float fov, Vec3 from, Vec3 at)
		=> Screenshot(name, 0, width, height, fov, from, at);
	public static void Screenshot(string name, int width, int height, Vec3 from, Vec3 at) 
		=> Screenshot(name, 0, width, height, 90, from, at);
	public static void Screenshot(string name, int frame, int width, int height, float fov, Vec3 from, Vec3 at)
	{
		if (!IsTesting || frame != sceneFrame || screenshots.Contains(name) || !MakeScreenshots)
			return;
		screenshots.Add(name);

		string file = Path.Combine(ScreenshotRoot, name);
		string dir  = Path.GetDirectoryName(file);
		if (Directory.Exists(dir) == false)
			Directory.CreateDirectory(dir);
		Renderer.Screenshot(file, from, at, width, height, fov);
	}

	public static void ScreenshotGltf(string name, int width, int height, Vec3 from, Vec3 at)
	{
		if (!IsTesting || screenshots.Contains(name) || !MakeScreenshots || GltfScreenshotRoot == null)
			return;
		screenshots.Add(name);

		string file = Path.Combine(GltfScreenshotRoot, name);
		string dir  = Path.GetDirectoryName(file);
		if (Directory.Exists(dir) == false)
			Directory.CreateDirectory(dir);
		Renderer.Screenshot(file, from, at, width, height, 45);
	}

	public static void Hand(in HandJoint[] joints) => Hand(Handed.Right, joints);
	public static void Hand(Handed hand, in HandJoint[] joints)
	{
		if (!IsTesting)
			return;
		Input.HandVisible (hand, true);
		Input.HandOverride(hand, joints);
	}
}