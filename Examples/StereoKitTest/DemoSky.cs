using StereoKit;

namespace StereoKitTest
{
    class DemoSky : IDemo
    {
        static Gradient gradient = new Gradient(new GradientKey[] {
                new GradientKey {color = Color.HSV(0.1f, 0.4f, 0.0f), position = 0},
                new GradientKey {color = Color.HSV(0.7f, 0.6f, 0.5f), position = 0.5f},
                new GradientKey {color = Color.HSV(0.3f, 0.4f, 0.8f), position = 1.0f} });

        static Pose dirPose = new Pose(new Vec3(0,0.1f,0), Quat.Identity);
        static Pose colorWinPose = new Pose(new Vec3(0,0,-0.35f), Quat.LookDir(-Vec3.Forward));
        float index = 0;
        bool  prevState = false;

        public void Initialize() {
            Renderer.SkyTex = Tex.GenCubemap(gradient, 16, dirPose.position);
        }

        public void Shutdown() { }

        public void Update()
        {
            bool moving = UI.AffordanceBegin("Direction", ref dirPose, Vec3.Zero, Vec3.One * 6 * Units.cm2m, true);
            UI.AffordanceEnd();
            Lines.Add(Vec3.Zero, dirPose.position, Color.White, 0.01f);
            bool dirChanged = moving;// !moving && prevState;
            prevState = moving;

            UI.WindowBegin("Gradient", ref colorWinPose, new Vec2(20, 0)*Units.cm2m);
            UI.Label("Color Index"); UI.SameLine();
            UI.HSlider("Index", ref index, 0, 2, 1, 10 * Units.cm2m);
            Vec3 c = gradient.Get(index/2.0f).ToHSV();
            bool colorDirty =     UI.HSlider("H", ref c.x, 0, 1, 0, 20 * Units.cm2m);
            colorDirty = UI.HSlider("S", ref c.y, 0, 1, 0, 20 * Units.cm2m) || colorDirty;
            colorDirty = UI.HSlider("V", ref c.z, 0, 1, 0, 20 * Units.cm2m) || colorDirty;
            if (colorDirty) {
                gradient.Set((int)index, Color.HSV(c.x, c.y, c.z), index/2.0f);
            }
            if (colorDirty || dirChanged)
            {
                Renderer.SkyTex = Tex.GenCubemap(gradient, 16, dirPose.position);
            }
            UI.WindowEnd();
        }
    }
}
