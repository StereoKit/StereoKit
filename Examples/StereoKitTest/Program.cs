using System;
using StereoKit;

class Program 
{
    static IScene activeScene;
    static IScene nextScene;
    public static IScene ActiveScene { get{ return activeScene;} set { nextScene = value; } }
    static void Main(string[] args) 
    {
        if (!StereoKitApp.Initialize("StereoKit C#", Runtime.Flatscreen, true))
            Environment.Exit(1);

        activeScene = new DemoBasics();
        activeScene.Initialize();

        while (StereoKitApp.Step(() =>
        {
            if (nextScene != null)
            {
                activeScene.Shutdown();
                nextScene.Initialize();
                activeScene = nextScene;
                nextScene = null;
            }
            activeScene.Update();
        }));

        activeScene.Shutdown();

        StereoKitApp.Shutdown();
    }
}