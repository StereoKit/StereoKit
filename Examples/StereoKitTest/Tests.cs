using StereoKit;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;

public static class Tests
{
	static List<Type> allTests  = new List<Type>();
	static List<Type> demoTests = new List<Type>();
	static ITest activeScene;
	static ITest nextScene;
	static int   testIndex  = 0;
	static int   runFrames  = -1;
	static float runSeconds = 0;
	static int   sceneFrame = 0;
	static float sceneTime  = 0;
	static HashSet<string> screens = new HashSet<string>();

	private static Type ActiveTest { set { nextScene = (ITest)Activator.CreateInstance(value); } }
	public  static int  DemoCount => demoTests.Count;
	public  static bool IsTesting { get; set; }

	public static string ScreenshotRoot  { get; set; } = "../../../docs/img/screenshots";
	public static bool   MakeScreenshots { get; set; } = true;

	public static void FindTests()
	{
		allTests = Assembly.GetExecutingAssembly()
			.GetTypes()
			.Where ( a => a != typeof(ITest) && typeof(ITest).IsAssignableFrom(a) )
			.ToList();
		demoTests = allTests
			.Where(a=>a.Name.StartsWith("Demo"))
			.ToList();
	}

	public static void Initialize()
	{
		if (IsTesting) { 
			nextScene = null;
		}
		if (nextScene == null)
			nextScene = (ITest)Activator.CreateInstance(allTests[testIndex]);
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
				Time.SetTime(0);
				Input.HandVisible(Handed.Max, false);
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
		activeScene.Update();
		sceneFrame++;

		if (IsTesting && FinishedWithTest())
		{
			testIndex += 1;
			if (testIndex >= allTests.Count)
				SK.Quit();
			else
				SetTestActive(allTests[testIndex].Name);
		}
	}
	public static void Shutdown()
	{
		activeScene.Shutdown();
		activeScene = null;
		GC.Collect(int.MaxValue, GCCollectionMode.Forced);
	}

	public static string GetDemoName  (int index)
	{
		return demoTests[index].Name;
	}
	public static void   SetDemoActive(int index)
	{
		Log.Write(LogLevel.Info, "Starting Scene: " + demoTests[index].Name);
		ActiveTest = demoTests[index];
	}
	public static void   SetTestActive(string name)
	{
		name = name.ToLower();
		Type result = allTests.OrderBy( a => {
			string str = a.Name.ToLower();
			if (str == name)             return 0;
			else if (str.Contains(name)) return str.Length - name.Length;
			else                         return 1000 + string.Compare(str, name);
		}).First();
		Log.Write(LogLevel.Info, "Starting Scene: " + result.Name);

		sceneFrame = 0;
		runFrames  = -1;
		runSeconds = 0;
		ActiveTest = result;
	}
	public static void Test(Func<bool> testFunction)
	{
		if (!testFunction())
		{
			Log.Err("Test failed for {0}!", testFunction.Method.Name);
			Environment.Exit(-1);
		}
	}

	private static bool FinishedWithTest()
	{
		if (runSeconds != 0) {
			return Time.Totalf-sceneTime > runSeconds;
		}
		if (runFrames != -1) { 
			return sceneFrame == runFrames;
		}
		return true;
	}
	public static void RunForFrames(int frames)
		=> runFrames = frames;
	public static void RunForSeconds(float seconds)
		=> runSeconds = seconds;

	public static void Screenshot(string name, int width, int height, float fov, Vec3 from, Vec3 at)
		=> Screenshot(name, 0, width, height, fov, from, at);
	public static void Screenshot(string name, int width, int height, Vec3 from, Vec3 at) 
		=> Screenshot(name, 0, width, height, 90, from, at);
	public static void Screenshot(string name, int frame, int width, int height, float fov, Vec3 from, Vec3 at)
	{
		if (!IsTesting || frame != sceneFrame || screens.Contains(name) || !MakeScreenshots)
			return;
		screens.Add(name);

		string file = Path.Combine(ScreenshotRoot, name);
		string dir = Path.GetDirectoryName(file);
		if (!Directory.Exists(dir))
			Directory.CreateDirectory(dir);
		Renderer.Screenshot(file, from, at, width, height, fov);
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