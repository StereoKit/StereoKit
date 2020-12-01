using StereoKit;
using StereoKit.Framework;
using System.IO;
using System.Threading.Tasks;

namespace StereoKitTest
{
    class DemoPicker : ITest
    {
        Model model      = null;
        float modelScale = 1;
        float menuScale  = 1;
        Pose  modelPose  = new Pose(-.3f,0,0, Quat.LookDir(-Vec3.Forward));
        Pose  menuPose   = new Pose(0.3f,0,0, Quat.LookDir(-Vec3.Forward));
        
        public void Initialize() => ShowPicker();
        public void Shutdown() => FilePicker.Hide();

        public void Update() {
            UI.WindowBegin("Settings", ref menuPose, new Vec2(20,0) * U.cm);
            if (model != null && UI.Button("Close")) { 
                model = null;
                ShowPicker();
            }
            UI.Label("Scale");
            UI.HSlider("ScaleSlider", ref menuScale, 0, 1, 0);
            UI.WindowEnd();

            if (model != null) {
                UI.HandleBegin("Model", ref modelPose, model.Bounds*modelScale*menuScale);
                model.Draw(Matrix.TS(Vec3.Zero, modelScale*menuScale));
                UI.HandleEnd();
            }
        }

        void ShowPicker()
        {
            FilePicker.Show(
                FilePickerMode.Open,
                Path.GetFullPath(SK.settings.assetsFolder),
                LoadModel,
                new FileFilter("GLTF", "*.gltf"),
                new FileFilter("GLB", "*.glb"),
                new FileFilter("OBJ", "*.obj"),
                new FileFilter("STL", "*.stl"),
				new FileFilter("FBX", "*.fbx"));
        }

        private void LoadModel(string filename)
        {
            Task.Run(() =>
            {
                model      = Model.FromFile(filename);
                modelScale = 1 / model.Bounds.dimensions.Magnitude;
            });
        }
    }
}
