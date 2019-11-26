using StereoKit;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StereoKitTest
{
    class DemoPicker : IDemo
    {
        FilePicker picker = new FilePicker(new (string,string)[]{ 
            ("GLTF", "*.gltf"),
            ("GLTF", "*.glb")});

        Model model      = null;
        float modelScale = 1;
        Pose  modelPose  = new Pose(-.3f,0,0, Quat.LookDir(-Vec3.Forward));
        Pose  menuPose   = new Pose(0.3f,0,0, Quat.LookDir(-Vec3.Forward));
        float menuScale  = 1;

        public void Initialize() { }

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
                UI.AffordanceBegin("Model", ref modelPose, model.Bounds.center*modelScale*menuScale, model.Bounds.dimensions*modelScale*menuScale);
                model.Draw(Matrix.TS(Vec3.Zero, modelScale*menuScale));
                UI.AffordanceEnd();
            }
        }

        private void LoadModel(string filename)
        {
            model      = new Model(picker.SelectedFile);
            modelScale = 1 / model.Bounds.dimensions.Magnitude;
        }
    }
}
