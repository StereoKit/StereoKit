
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;

public static class Demos
{
    static List<Type> demos = new List<Type>();
    static IDemo activeScene;
    static IDemo nextScene;
    static Type ActiveScene { set { nextScene = (IDemo)Activator.CreateInstance(value); } }
    public static int Count => demos.Count;

    public static void FindDemos()
    {
        demos = Assembly.GetExecutingAssembly()
            .GetTypes()
            .Where ( a => a != typeof(IDemo) && typeof(IDemo).IsAssignableFrom(a) )
            .ToList();
    }

    public static void Initialize()
    {
        if (activeScene == null)
            activeScene = nextScene;
        if (activeScene == null)
            activeScene = (IDemo)Activator.CreateInstance(demos[0]);
        activeScene.Initialize();
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
    }
    public static void Shutdown()
    {
        activeScene.Shutdown();
        activeScene = null;
    }

    public static string GetName(int index)
    {
        return demos[index].GetType().Name;
    }
    public static void SetActive(int index)
    {
        ActiveScene = demos[index];
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
        Console.WriteLine("Starting Scene: " + result.Name);

        ActiveScene = result;
    }
}