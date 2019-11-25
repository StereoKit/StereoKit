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
    /// for a toggle, and a float that we'll use as a slider! We'll add this code
    /// to our initialization section.
    Pose  windowPose = new Pose(-.4f, 0, 0, Quat.LookDir(1,0,1));
    
    bool  showHeader = true;
    float slider     = 0.5f;
    /// :End:

    Model  clipboard     = new Model("Clipboard.glb");
    Sprite sprite        = new Sprite("Floor.png", SpriteType.Single);
    Pose   clipboardPose = new Pose(.4f,0,0, Quat.LookDir(-1,0,1));
    bool  subtitles;
    bool  clipButts;
    float clipSlider;

    public void Update()
    {
        if (Demos.TestMode)
            Renderer.Screenshot(new Vec3(-0.325f,-0.025f,.075f), new Vec3(-.4f,-0.075f,0), 600, 400, "../../../docs/img/screenshots/GuideUserInterface.jpg");

        /// :CodeDoc: Guides User Interface
        /// Then we'll move over to the application step where we'll do the rest of the UI code!
        ///
        /// We'll start with a window titled "Window" that's 20cm wide, and auto-resizes on the 
        /// y-axis. The Units class is pretty helpful here, as it allows us to reason more visually
        /// about the units we're using! StereoKit uses meters as its base unit, which look a little
        /// awkward, especially in the millimeter range.
        /// 
        /// We'll also use a toggle to turn the window's header on and off! The value from that toggle
        /// is passed in here via the showHeader field.
        UI.WindowBegin("Window", ref windowPose, new Vec2(20, 0) * Units.cm2m, showHeader);

        /// Here's that toggle button! You'll also notice our use of 'ref' values in a lot of the UI
        /// code. UI functions typically follow the pattern of returning true/false to indicate they've
        /// been interacted with during the frame, so you can nicely wrap them in 'if' statements to 
        /// react to change!
        /// 
        /// Then with the 'ref' parameter, we let you pass in the current state of the UI element. The UI
        /// element will update that value for you based on user interaction, but you can also change it
        /// yourself whenever you want to!
        UI.Toggle("Show Header", ref showHeader);

        /// Here's an example slider! We start off with a label element, and tell the UI to 
        /// keep the next item on the same line. The slider clamps to the range [0,1], and 
        /// will step at intervals of 0.2. If you want it to slide continuously, you can just set
        /// the `step` value to 0!
        UI.Label("Slide");
        UI.SameLine();
        UI.HSlider("slider", ref slider, 0, 1, 0.2f, 72 * Units.mm2m);

        /// Here's how you use a simple button! Just check it with an 'if'. Any UI method
        /// will return true on the frame when their value or state has changed.
        if (UI.ButtonRound("Exit"))
            StereoKitApp.Quit();

        /// And for every begin, there must also be an end! StereoKit will log errors when this
        /// occurs, so keep your eyes peeled for that!
        UI.WindowEnd();

        /// And there you go! That's how UI works in StereoKit, pretty simple, huh?
        /// For further reference, and more UI methods, check out the 
        /// [UI class documentation]({{site.url}}/Pages/Reference/UI.html).
        /// 
        /// If you'd like to see the complete code for this sample, 
        /// [check it out on Github](https://github.com/maluoi/StereoKit/blob/master/Examples/StereoKitTest/DemoUI.cs)!
        /// :End:

        UI.AffordanceBegin("Clip", ref clipboardPose, new Vec3(15, 20, 0) * Units.cm2m, new Vec3(30, 40, 5) * Units.cm2m, false);
        UI.LayoutArea(new Vec3(12, 15, 0) * Units.cm2m, new Vec2(24, 30) * Units.cm2m);
        UI.Label("Application 'Settings'");
        UI.Toggle("Subtitles", ref subtitles); UI.SameLine();
        UI.Toggle("Butts", ref clipButts);
        UI.HSlider("Slide", ref clipSlider, 0, 1, 0, 22 * Units.cm2m);
        UI.Image(sprite, Vec2.One * UI.LineHeight); UI.SameLine();
        UI.Label("Test!");
        UI.ButtonRound("Press", UI.LineHeight); UI.SameLine();
        UI.Button("Squeeze");
        UI.AffordanceEnd();
        clipboard.Draw(clipboardPose.ToMatrix());
    }

    public void Initialize() { }
    public void Shutdown() { }
}