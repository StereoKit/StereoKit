
using StereoKit;

class DemoSelect : IDemo
{
    public void Initialize() { }
    public void Shutdown  () { }

    public void Update()
    {
        Pose pose = new Pose();
        UI.WindowBegin("Demos", ref pose, new Vec2(50 * Units.cm2m,0));
        for (int i = 0; i < Demos.Count; i++) {
            string name = Demos.GetName(i);
            if (name.StartsWith("Demo"))
                name = name.Substring("Demo".Length);

            if (UI.Button( name ))
                Demos.SetActive(i);
            UI.SameLine();
        }
        UI.WindowEnd();
    }
}