using System;
using StereoKit;

class Program 
{
    static ISKApp activeApp;
    static ISKApp nextApp;
    public static ISKApp ActiveApp { get{ return activeApp;} set { nextApp = value; } }
    static void Main(string[] args) 
    {
        if (!StereoKitApp.Initialize("StereoKit C#", Runtime.Flatscreen, true))
            Environment.Exit(1);

        activeApp = new DemoBasics();
        activeApp.Initialize();

        while (StereoKitApp.Step(() =>
        {
            if (nextApp != null)
            {
                activeApp.Shutdown();
                nextApp.Initialize();
                activeApp = nextApp;
                nextApp = null;
            }
            activeApp.Update();
        }));

        activeApp.Shutdown();

        StereoKitApp.Shutdown();
    }
}