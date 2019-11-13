/// :CodeDoc: Guides User Interface
/// # Building UI in StereoKit
/// 
/// ## Immediate Mode UI
/// 
/// StereoKit uses an immediate mode UI system. Basically, you define the UI 
/// every single frame you want to see it! Sounds a little odd at first, but
/// it does have some pretty tremendous advantages. Since very little state is
/// actually stored, you can add, remove, and change your UI elements with trivial
/// and standard code structures! You'll find that you often have much less UI
/// code, with far fewer places for things to go wrong.
///
/// The goal for this UI API is to get you up and running as fast as possible
/// with a working UI! This does mean trading some design flexibility for API 
/// simplicity, but we also strive to retain configurability for those that need
/// a little extra.
/// 
/// :End:


using StereoKit;

class DemoUI : IDemo
{
    /// :CodeDoc: Guides User Interface
    /// ## Making a Window
    /// 
    /// ![Simple UI]({{site.url}}/img/screenshots/GuideUserInterface.jpg)
    /// 
    /// Since StereoKit doesn't store state, you'll have to keep track of your
    /// data yourself! But that's actually a pretty good thing, since you'll 
    /// probably do that one way or another anyhow. Here we've got a Pose for
    /// the window, off to the left and facing to the right, as well as a boolean
    /// for a toggle, and a float that we'll use as a slider!
    Pose  windowPose = new Pose(new Vec3(-.25f, 0, 0), Quat.LookDir(Vec3.Right));
    
    bool  showHeader = true;
    float slider     = 0.5f;
    /// :End:

    public void Update()
    {
        if (Demos.TestMode)
            Renderer.Screenshot(new Vec3(-0.125f,-0.05f,0), new Vec3(-1,-0.3f,0), "../../../docs/img/screenshots/GuideUserInterface.jpg");

        /// :CodeDoc: Guides User Interface
        /// Now in our application step, we'll use this code!
        
        // Start a window titled "Window" that's 20cm wide, and auto-resizes on the y-axis.
        // We'll use a toggle so we can turn the window's header on and off!
        UI.WindowBegin("Window", ref windowPose, new Vec2(20, 0) * Units.cm2m, showHeader);

        // Here's that toggle button! ref showHeader gets updated when the user interacts.
        UI.Toggle("Show Header", ref showHeader);

        // Here's an example slider! We start off with a label element, and tell the UI to 
        // keep the next item on the same line. The slider clamped to the range [0,1], and 
        // will step at intervals of 0.2.
        UI.Label("Slide");
        UI.SameLine();
        UI.HSlider("slider", ref slider, 0, 1, 0.2f, 72 * Units.mm2m);

        // Here's how you use a simple button! Just check it with an 'if'. Any UI method
        // will return true on the frame when their value has changed.
        if (UI.ButtonRound("Exit"))
            StereoKitApp.Quit();

        // For every begin, there must also be an end!
        UI.WindowEnd();

        /// And there you go! That's how UI works in StereoKit, pretty simple, huh?
        /// For further reference, and more UI methods, check out the 
        /// [UI class documentation]({{site.url}}/Pages/Reference/UI.html).
        /// 
        /// If you'd like to see the complete code for this sample, 
        /// [check it out on Github](https://github.com/maluoi/StereoKit/blob/master/Examples/StereoKitTest/DemoUI.cs)!
        /// :End:
    }

    public void Initialize() { }
    public void Shutdown() { }
}