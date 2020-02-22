using System;
using System.Collections.Generic;
using StereoKit;

class Program 
{
#if WINDOWS_UWP
    const string Root = "Assets";
#else
    const string Root = "../../../Examples/Assets";
#endif

    static Model  floorMesh;
    static Matrix floorTr;
    static Pose   demoSelectPose = new Pose();

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
    static Pose         demoLog     = new Pose(0, -0.1f, 0.5f, Quat.LookDir(Vec3.Forward));
    static List<string> demoLogList = new List<string>();
    static void OnLog(LogLevel level, string text)
    {
        if (demoLogList.Count > 10)
            demoLogList.RemoveAt(demoLogList.Count - 1);
        demoLogList.Insert(0, text.Length < 100 ? text : text.Substring(0,100)+"...");
    }
    static void LogWindow()
    {
        UI.WindowBegin("Log", ref demoLog, new Vec2(40, 0) * Units.cm2m);
        for (int i = 0; i < demoLogList.Count; i++)
            UI.Label(demoLogList[i], false);
        UI.WindowEnd();
    }
    /// :End:

    //////////////////////

    static void Main(string[] args) 
    {
        Tests.IsTesting = args.Length > 0 && args[0].ToLower() == "-test";
        Time .Scale     = Tests.IsTesting ? 0 : 1;

        Log.Filter = LogLevel.Diagnostic;
        StereoKitApp.settings.assetsFolder = Program.Root;
        if (!StereoKitApp.Initialize("StereoKit C#", Tests.IsTesting ? Runtime.Flatscreen : Runtime.MixedReality,  true))
            Environment.Exit(1);

        CommonInit();

        Tests.FindTests();
        Tests.SetTestActive(args.Length > 0 ? args[0] : "Lines");
        Tests.Initialize();
        
        while (StereoKitApp.Step(() =>
        {
            Tests.Update();
            CommonUpdate();
        }));

        Tests.Shutdown();
        CommonShutdown();

        StereoKitApp.Shutdown();
    }

    static void CommonInit()
    {
        /// :CodeSample: Log.Subscribe Log
        /// Then you add the OnLog method into the log events like this in your initialization
        /// code!
        Log.Subscribe(OnLog);
        /// :End:

        Material floorMat = Default.Material.Copy();
        floorMat[MatParamName.DiffuseTex] = Tex.FromFile("Floor.png");
        floorMat[MatParamName.TexScale  ] = 16;

        floorMesh = Model.FromMesh(Mesh.GenerateCube(Vec3.One), floorMat);
        floorTr   = Matrix.TRS(new Vec3(0, -1.5f, 0), Quat.Identity, new Vec3(20, 1, 20));

        demoSelectPose.position    = new Vec3(0, 0, -0.25f);
        demoSelectPose.orientation = Quat.LookDir(-Vec3.Forward);
    }
    static void CommonUpdate()
    {
        // If we can't see the world, we'll draw a floor!
        if (StereoKitApp.System.displayType == Display.Opaque)
            Renderer.Add(floorMesh, floorTr, Color.White);

        // Skip selection window if we're in test mode
        if (Tests.IsTesting)
            return;

        // Make a window for demo selection
        UI.WindowBegin("Demos", ref demoSelectPose, new Vec2(50 * Units.cm2m, 0));
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
        UI.AffordanceBegin("Ruler", ref demoRuler, new Bounds(new Vec3(30,4,1)*Units.cm2m), true);
        Color32 color = Color.HSV(.6f, 0.5f, 1);
        Text.Add("Centimeters", Matrix.TRS(new Vec3(14.5f, -1.5f, -.6f) * Units.cm2m, Quat.Identity, .3f), TextAlign.XLeft | TextAlign.YBottom);
        for (int d = 0; d <= 60; d+=1)
        {
            float x = d/2.0f;
            float size = d%2==0?1f:0.15f;
            Lines.Add(new Vec3(15-x,2,-.6f)* Units.cm2m, new Vec3(15-x,2-size, -.6f) *Units.cm2m, color, Units.mm2m*0.5f);
            if (d%2==0 && d/2 != 30)
                Text.Add((d/2).ToString(), Matrix.TRS(new Vec3(15-x-0.1f,2-size, -.6f) * Units.cm2m, Quat.Identity, .2f), TextAlign.XLeft|TextAlign.YBottom);
        }
        UI.AffordanceEnd();
    }
}