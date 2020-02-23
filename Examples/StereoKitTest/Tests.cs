using StereoKit;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;

public static class Tests
{
    static List<Type> allTests  = new List<Type>();
    static List<Type> demoTests = new List<Type>();
    static ITest activeScene;
    static ITest nextScene;
    static int   testIndex  = 0;
    static int   runFrames  = int.MaxValue;
    static float runSeconds = float.MaxValue;
    static float sceneTime  = 0;
    static HashSet<string> screens = new HashSet<string>();

    private static Type ActiveTest { set { nextScene = (ITest)Activator.CreateInstance(value); } }
    public  static int  DemoCount => demoTests.Count;
    public  static bool IsTesting { get; set; }

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
            Input.HandVisible(Handed.Max, false);
        }
        if (activeScene == null)
            activeScene = nextScene;
        if (activeScene == null)
            activeScene = (ITest)Activator.CreateInstance(allTests[testIndex]);
        activeScene.Initialize();
        sceneTime = Time.Totalf;
    }
    public static void Update()
    {
        if (nextScene != null)
        {
            activeScene.Shutdown();
            nextScene  .Initialize();
            activeScene = nextScene;
            nextScene   = null;
        }
        activeScene.Update();

        if (IsTesting && FinishedWithTest())
        {
            testIndex += 1;
            if (testIndex >= allTests.Count)
                StereoKitApp.Quit();
            else
                SetTestActive(allTests[testIndex].Name);
        }
    }
    public static void Shutdown()
    {
        activeScene.Shutdown();
        activeScene = null;
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

        runFrames  = int.MaxValue;
        runSeconds = float.MaxValue;
        ActiveTest = result;
    }

    private static bool FinishedWithTest()
    {
        if (runSeconds != float.MaxValue) { 
            runSeconds -= Time.Elapsedf;
            return runSeconds <= 0;
        }
        if (runFrames != int.MaxValue) { 
            runFrames -= 1;
            return runFrames <= 0;
        }
        return true;
    }
    public static void RunForFrames(int frames)
        => runFrames = frames;
    public static void RunForSeconds(float seconds)
        => runSeconds = seconds;

    public static void Screenshot(int width, int height, string name, Vec3 from, Vec3 at) 
        => Screenshot(0, width, height, name, from, at);
    public static void Screenshot(float time, int width, int height, string name, Vec3 from, Vec3 at)
    {
        if (!IsTesting || time > (Time.Totalf-sceneTime) || screens.Contains(name))
            return;
        screens.Add(name);
        Renderer.Screenshot(from, at, width, height, $"../../../docs/img/screenshots/{name}");
    }
    public static void Hand(in HandJoint[] joints)
    {
        if (!IsTesting)
            return;
        Input.HandVisible (Handed.Right, true);
        Input.HandOverride(Handed.Right, joints);
    }
}