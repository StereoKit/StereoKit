using StereoKit;
using System.IO;

namespace StereoKitTest
{
    class DemoPicker : IDemo
    {
        static Model model      = null;
        static float modelScale = 1;
        static float menuScale  = 1;
        FilePicker picker;
        Pose  modelPose  = new Pose(-.3f,0,0, Quat.LookDir(-Vec3.Forward));
        Pose  menuPose   = new Pose(0.3f,0,0, Quat.LookDir(-Vec3.Forward));
        
        public void Initialize() {
            picker = new FilePicker(
                new (string, string)[]{
                    ("GLTF", "*.gltf"),
                    ("GLB",  "*.glb") }, 
                Path.GetFullPath(StereoKitApp.settings.assetsFolder));
        }

        public void Shutdown() { }

        public void Update() {
            UI.WindowBegin("Settings", ref menuPose, new Vec2(20,0) * Units.cm2m);
            if (model != null && UI.Button("Close"))
                model = null;
            UI.Label("Scale");
            UI.HSlider("ScaleSlider", ref menuScale, 0, 1, 0);
            UI.WindowEnd();

            if (model == null) {
                if (picker.Show())
                    LoadModel(picker.SelectedFile);
            } else {
                UI.AffordanceBegin("Model", ref modelPose, model.Bounds*modelScale*menuScale);
                model.Draw(Matrix.TS(Vec3.Zero, modelScale*menuScale));
                UI.AffordanceEnd();
            }
        }

        private void LoadModel(string filename)
        {
            model      = Model.FromFile(picker.SelectedFile);
            modelScale = 1 / model.Bounds.dimensions.Magnitude;
        }
    }
}
