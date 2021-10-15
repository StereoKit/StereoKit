using StereoKit;
using System.Threading.Tasks;

namespace StereoKitTest
{
	class DemoPicker : ITest
	{
		Matrix descPose    = Matrix.TR (-0.5f, 0, -0.5f, Quat.LookDir(1,0,1));
		string description = "Applications need to save and load files at runtime! StereoKit has a cross-platform, MR compatible file picker built in, Platform.FilePicker.\n\nOn systems/conditions where a native file picker is available, that's what you'll get! Otherwise, StereoKit will fall back to a custom picker built with StereoKit's UI.";
		Matrix titlePose   = Matrix.TRS(V.XYZ(-0.5f, 0.05f, -0.5f), Quat.LookDir(1, 0, 1), 2);
		string title       = "File Picker";

		Model    model      = null;
		float    modelScale = 1;
		float    animScrub  = 0;
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

			/// :CodeSample: Platform.FilePicker Platform.FilePickerVisible
			/// ### Opening a Model
			/// This is a simple button that will open a 3D model selection
			/// file picker, and make a call to OnLoadModel after a file has
			/// been successfully picked!
			if (UI.Button("Open Model") && !Platform.FilePickerVisible) {
				Platform.FilePicker(PickerMode.Open, OnLoadModel, null,
					".gltf", ".glb", ".obj", ".stl", ".fbx", ".ply");
			}
			/// :End:

			UI.Label("Scale");
			UI.HSlider("ScaleSlider", ref menuScale, 0, 1, 0);

			if (model != null)
			{
				UI.HSeparator();
				foreach (Anim anim in model.Anims)
				{
					if (UI.Button(anim.Name))
						model.PlayAnim(anim, AnimMode.Loop);
				}
				UI.HSeparator();
				if (UI.HSlider("Scrub", ref animScrub, 0, 1, 0))
				{
					if (model.AnimMode != AnimMode.Manual)
						model.PlayAnim(model.ActiveAnim, AnimMode.Manual);
					model.AnimCompletion = animScrub;
				}
			}

			UI.WindowEnd();

			if (model != null) {
				Bounds scaled = model.Bounds * modelScale * menuScale;
				UI.HandleBegin("Model", ref modelPose, scaled);
				Default.MeshCube.Draw(volumeMat, Matrix.TS(scaled.center, scaled.dimensions));
				model.Draw(Matrix.TS(Vec3.Zero, modelScale*menuScale));
				UI.HandleEnd();
			}

			Text.Add(title, titlePose);
			Text.Add(description, descPose, V.XY(0.4f, 0), TextFit.Wrap, TextAlign.TopCenter, TextAlign.TopLeft);
		}

		/// :CodeSample: Platform.FilePicker Platform.FilePickerVisible
		/// Once you have the filename, it's simply a matter of loading it
		/// from file. This is an example of async loading a model, and 
		/// calculating a scale value that ensures the model is a reasonable
		/// size.
		private void OnLoadModel(string filename)
		{
			Task.Run(() =>
			{
				model      = Model.FromFile(filename);
				modelScale = 1 / model.Bounds.dimensions.Magnitude;
				if (model.Anims.Count > 0)
					model.PlayAnim(model.Anims[0], AnimMode.Loop);
			});
		}
		/// :End:
	}
}
