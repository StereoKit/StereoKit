using StereoKit;

class DocLastElement : ITest
{
    float sliderVal  = 0.5f;
    Pose  windowPose = new Pose(0,0,-0.5f, Quat.LookDir(0,0,1));

    public void Update()
    {
        /// :CodeSample: UI.LastElementHandUsed UI.LastElementActive UI.LastElementFocused
        /// ### Checking UI element status
        /// It can sometimes be nice to know how the user is interacting with a
        /// particular UI element! The UI.LastElementX functions can be used to
        /// query a bit of this information, but only for _the most recent_ UI
        /// element that **uses an id**!
        /// 
        /// ![A window containing the status of a UI element]({{site.screen_url}}/UI/LastElementAPI.jpg)
        /// 
        /// So in this example, we're querying the information for the "Slider"
        /// UI element. Note that UI.Text does NOT use an id, which is why this
        /// works.
        UI.WindowBegin("Last Element API", ref windowPose);

        UI.HSlider("Slider", ref sliderVal, 0, 1, 0.1f, 0, UIConfirm.Pinch);
        UI.Text("Element Info:", TextAlign.TopCenter);
        if (UI.LastElementHandUsed(Handed.Left ).IsActive()) UI.Label("Left");
        if (UI.LastElementHandUsed(Handed.Right).IsActive()) UI.Label("Right");
        if (UI.LastElementFocused               .IsActive()) UI.Label("Focused");
        if (UI.LastElementActive                .IsActive()) UI.Label("Active");

        UI.WindowEnd();
        /// :End:

        Tests.Screenshot("UI/LastElementAPI.jpg", 1, 500, 500, 90, V.XYZ(0, -0.08f, -0.35f), V.XYZ(0, -0.08f, -0.5f));
    }

    public void Initialize()
    {
        Tests.RunForFrames(2);
        Tests.Hand(new HandJoint[] { new HandJoint(V.XYZ(0.049f, -0.004f, -0.415f), new Quat(-0.359f, -0.281f, -0.375f, -0.807f), 0.006f), new HandJoint(V.XYZ(0.049f, -0.004f, -0.415f), new Quat(-0.103f, -0.370f, -0.491f, -0.782f), 0.015f), new HandJoint(V.XYZ(0.019f, -0.013f, -0.447f), new Quat(-0.039f, -0.323f, -0.521f, -0.789f), 0.013f), new HandJoint(V.XYZ(0.001f, -0.021f, -0.471f), new Quat(0.081f, -0.231f, -0.565f, -0.788f), 0.011f), new HandJoint(V.XYZ(-0.003f, -0.028f, -0.486f), new Quat(0.081f, -0.231f, -0.565f, -0.788f), 0.008f), new HandJoint(V.XYZ(0.062f, 0.016f, -0.419f), new Quat(-0.071f, -0.307f, 0.104f, -0.943f), 0.005f), new HandJoint(V.XYZ(0.021f, 0.030f, -0.477f), new Quat(0.305f, -0.346f, -0.028f, -0.887f), 0.013f), new HandJoint(V.XYZ(-0.002f, 0.008f, -0.499f), new Quat(0.696f, -0.263f, -0.201f, -0.637f), 0.011f), new HandJoint(V.XYZ(-0.003f, -0.014f, -0.497f), new Quat(0.827f, -0.197f, -0.269f, -0.453f), 0.010f), new HandJoint(V.XYZ(-0.000f, -0.024f, -0.491f), new Quat(0.827f, -0.197f, -0.269f, -0.453f), 0.007f), new HandJoint(V.XYZ(0.071f, 0.016f, -0.428f), new Quat(-0.043f, -0.247f, 0.191f, -0.949f), 0.005f), new HandJoint(V.XYZ(0.040f, 0.028f, -0.487f), new Quat(0.083f, -0.318f, 0.151f, -0.932f), 0.013f), new HandJoint(V.XYZ(0.013f, 0.026f, -0.521f), new Quat(0.317f, -0.337f, 0.066f, -0.884f), 0.011f), new HandJoint(V.XYZ(-0.003f, 0.013f, -0.536f), new Quat(0.460f, -0.338f, 0.007f, -0.821f), 0.010f), new HandJoint(V.XYZ(-0.010f, 0.003f, -0.541f), new Quat(0.460f, -0.338f, 0.007f, -0.821f), 0.007f), new HandJoint(V.XYZ(0.079f, 0.012f, -0.435f), new Quat(-0.032f, -0.174f, 0.231f, -0.957f), 0.004f), new HandJoint(V.XYZ(0.060f, 0.020f, -0.492f), new Quat(0.005f, -0.236f, 0.220f, -0.946f), 0.011f), new HandJoint(V.XYZ(0.042f, 0.024f, -0.529f), new Quat(0.165f, -0.275f, 0.177f, -0.930f), 0.010f), new HandJoint(V.XYZ(0.027f, 0.019f, -0.549f), new Quat(0.283f, -0.299f, 0.140f, -0.900f), 0.008f), new HandJoint(V.XYZ(0.019f, 0.013f, -0.558f), new Quat(0.283f, -0.299f, 0.140f, -0.900f), 0.006f), new HandJoint(V.XYZ(0.088f, 0.001f, -0.439f), new Quat(-0.041f, -0.108f, 0.316f, -0.942f), 0.004f), new HandJoint(V.XYZ(0.078f, 0.010f, -0.495f), new Quat(-0.006f, -0.145f, 0.310f, -0.940f), 0.010f), new HandJoint(V.XYZ(0.069f, 0.013f, -0.526f), new Quat(0.129f, -0.206f, 0.284f, -0.927f), 0.008f), new HandJoint(V.XYZ(0.061f, 0.011f, -0.542f), new Quat(0.230f, -0.249f, 0.260f, -0.904f), 0.007f), new HandJoint(V.XYZ(0.054f, 0.007f, -0.551f), new Quat(0.230f, -0.249f, 0.260f, -0.904f), 0.006f), new HandJoint(V.XYZ(0.056f, 0.022f, -0.457f), new Quat(-0.585f, 0.252f, 0.057f, 0.769f), 0.000f), new HandJoint(V.XYZ(0.075f, 0.009f, -0.429f), new Quat(-0.585f, 0.252f, 0.057f, 0.769f), 0.000f) });
    }
    public void Shutdown() { }
}