using StereoKit;
using System.IO;
using System.Threading.Tasks;

namespace StereoKitTest
{
	class DemoPicker : ITest
	{
		Model    model      = null;
		float    modelScale = 1;
		float    menuScale  = 1;
		Pose     modelPose  = new Pose(0.6f,0,0, Quat.LookDir(-Vec3.Forward));
		Pose     menuPose   = new Pose(0.5f,0,-0.5f, Quat.LookDir(-1,0,1));
		Material volumeMat;

		public void Initialize()
		{
			volumeMat = Default.MaterialUIBox.Copy();
			volumeMat["border_size"] = 0;
			volumeMat["border_affect_radius"] = 0.3f;
		}
		public void Shutdown() => Platform.FilePickerClose();

		public void Update() {
			UI.WindowBegin("Settings", ref menuPose, new Vec2(20,0) * U.cm);
			if (UI.Button("Open Model") && !Platform.FilePickerVisible) {
				Platform.FilePicker(PickerMode.Open, LoadModel, null,
					".gltf", ".glb", ".obj", ".stl", ".fbx", ".ply");
			}
			UI.Label("Scale");
			UI.HSlider("ScaleSlider", ref menuScale, 0, 1, 0);
			UI.WindowEnd();

			if (model != null) {
				Bounds scaled = model.Bounds * modelScale * menuScale;
				UI.HandleBegin("Model", ref modelPose, scaled);
				Default.MeshCube.Draw(volumeMat, Matrix.TS(scaled.center, scaled.dimensions));
				model.Draw(Matrix.TS(Vec3.Zero, modelScale*menuScale));
				UI.HandleEnd();
			}
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
