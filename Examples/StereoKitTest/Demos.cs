using StereoKit;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;

public static class Demos
{
    static List<Type> demos = new List<Type>();
    static IDemo activeScene;
    static IDemo nextScene;
    static int   testIndex = 0;
    static float sceneTime = 0;
    static HashSet<string> screens = new HashSet<string>();
    static Type ActiveScene { set { nextScene = (IDemo)Activator.CreateInstance(value); } }
    public static int Count => demos.Count;
    public static bool TestMode { get; set; }

    public static void FindDemos()
    {
        demos = Assembly.GetExecutingAssembly()
            .GetTypes()
            .Where ( a => a != typeof(IDemo) && typeof(IDemo).IsAssignableFrom(a) )
            .ToList();
    }

    public static void Initialize()
    {
        if (TestMode) { 
            nextScene = null;
            Input.HandVisible(Handed.Max, false);
        }
        if (activeScene == null)
            activeScene = nextScene;
        if (activeScene == null)
            activeScene = (IDemo)Activator.CreateInstance(demos[testIndex]);
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

        if (TestMode)
        {
            testIndex += 1;
            if (testIndex >= Count)
                StereoKitApp.Quit();
            else
                SetActive(testIndex);
        }
    }
    public static void Shutdown()
    {
        activeScene.Shutdown();
        activeScene = null;
    }

    public static string GetName(int index)
    {
        return demos[index].Name;
    }
    public static void SetActive(int index)
    {
        ActiveScene = demos[index];
        Log.Write(LogLevel.Info, "Starting Scene: " + demos[index].Name);
    }
    public static void SetActive(string name)
    {
        name = name.ToLower();
        Type result = demos.OrderBy( a => {
            string str = a.Name.ToLower();
            if (str == name)             return 0;
            else if (str.Contains(name)) return str.Length - name.Length;
            else                         return 1000 + string.Compare(str, name);
        }).First();
        Log.Write(LogLevel.Info, "Starting Scene: " + result.Name);

        ActiveScene = result;
    }

    public static void Screenshot(int width, int height, string name, Vec3 from, Vec3 at) 
        => Screenshot(0, width, height, name, from, at);
    public static void Screenshot(float time, int width, int height, string name, Vec3 from, Vec3 at)
    {
        if (!TestMode || time > (Time.Totalf-sceneTime) || screens.Contains(name))
            return;
        screens.Add(name);
        Renderer.Screenshot(from, at, width, height, $"../../../docs/img/screenshots/{name}");
    }
    public static void Hand(in HandJoint[] joints)
    {
        if (!TestMode)
            return;
        Input.HandVisible (Handed.Right, true);
        Input.HandOverride(Handed.Right, joints);
    }
}